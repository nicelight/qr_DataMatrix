#include <Arduino.h>
#include <ETH.h>
#include <ArduinoJson.h>
#include "led.h"
#include "timer.h"
#include "sets.h"
// #include "nastroyki.h"
// const char *serverUrl = "http://192.168.10.250/api/callback";

// источник кода ethernet клиента
// https://github.com/khoih-prog/AsyncHTTPRequest_Generic/blob/master/examples/WT32_ETH01/AsyncHTTPRequest_WT32_ETH01/AsyncHTTPRequest_WT32_ETH01.ino

// Level from 0-4
#define ASYNC_HTTP_DEBUG_PORT Serial
#define _ASYNC_HTTP_LOGLEVEL_ 1
#define _ETHERNET_WEBSERVER_LOGLEVEL_ 1

#include <WebServer_WT32_ETH01.h> // https://github.com/khoih-prog/WebServer_WT32_ETH01
#include <HTTPClient.h>

// #define ASYNC_HTTP_REQUEST_GENERIC_VERSION_MIN_TARGET "AsyncHTTPRequest_Generic v1.10.2"
// #define ASYNC_HTTP_REQUEST_GENERIC_VERSION_MIN 1010002

// Uncomment for certain HTTP site to optimize
// #define NOT_SEND_HEADER_AFTER_CONNECTED        true

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
// #include <AsyncHTTPRequest_Generic.h> // https://github.com/khoih-prog/AsyncHTTPRequest_Generic
// AsyncHTTPRequest request;

IPAddress myIP(IP_OCT1, IP_OCT2, IP_OCT3, IP_OCT4);
IPAddress myGW(IP_OCT1, IP_OCT2, IP_OCT3, IP_GATE);
IPAddress mySN(255, 255, 255, 0);
IPAddress myDNS(8, 8, 8, 8);

// 192.168.10.250/api/callback мой комп

// const char POST_ServerAddress[] = "192.168.10.50"; // в файле nastroyki.h
String apiPath = "/api/callback";

String dm_string = "";
long dm_number = 1000000000;

bool scanDMcode()
{
    if (Serial1.available() > 0)
    {
        dm_string = "";
        uint32_t startTime = millis();
        while (millis() - startTime < 100ul)
        {
            while (Serial1.available() > 0)
            {
                char c = Serial1.read();
                dm_string += c;
                startTime = millis();
            }
        }
        if (dm_string.length() >= 32)
        {
            String sub_str = dm_string.substring(23, 32);
            for (int i = 0; i < sub_str.length(); i++)
            {
                if (!isDigit(sub_str.charAt(i)))
                {
                    return 0;
                }
            }
            dm_number = sub_str.toInt();
            return 1;
        }
    }
    dm_number = 1000000000;
    return 0;
}

void blinkLed(int times, int duration)
{
    for (int i = 0; i < times; i++)
    {
        digitalWrite(INDIKATOR, HIGH);
        delay(duration);
        digitalWrite(INDIKATOR, LOW);
        delay(duration);
    }
}

int passDMCode(long dm_number)
{
    // если не подключен Ethernet выйдем
    if (!WT32_ETH01_isConnected())
        return -1;

    static bool requestOpenResult;
    // Формируем JSON
    JsonDocument doc;
    JsonArray arr = doc["passIds"].to<JsonArray>();
    arr.add(String(dm_number));
    doc["code"] = "TURNIKET";
    String jsonStr;
    serializeJson(doc, jsonStr);

    // СТАРЫЙ ПРИМЕР, УДАЛИТЬ !
    // // Формируем HTTP POST запрос
    // String postData =
    //     // String("POST ") + serverPath + " HTTP/1.1\r\n" +
    //     // "Host: " + serverIP.toString() + "\r\n" +
    //     String("Content-Type: application/json\r\n") +
    //     "Content-Length: " + jsonStr.length() + "\r\n" +
    //     "Connection: close\r\n\r\n" +
    //     jsonStr;
    const char *serverUrl = SERVERURL ;
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json"); // Установка заголовков :contentReference[oaicite:4]{index=4}

    Serial.print("Connecting to srv ");
    Serial.print(serverUrl);
    Serial.print("\n\tand sending POST:\n");
    Serial.print(jsonStr);
    // client.print(postData);
    int httpCode = http.POST(jsonStr); // отправка POST
    if (httpCode > 0)
    {
        // Serial.printf("\nSRV code: %d\n", httpCode);
        String response = http.getString();
        Serial.print(" \n\t||\n\t||\n resp:\t");
        Serial.println(response);
        http.end(); // Освобождение ресурсов
        return httpCode;
    }
    else
    {
        Serial.printf("POST failed, error: %s\n", http.errorToString(httpCode).c_str());
        http.end(); // Освобождение ресурсов
        return -2;
    }

} // passDMCode

void parse_serial()
{
    if (Serial.available() > 0)
    {
        char receivedChar = Serial.read();
        if (receivedChar == 'd')
        {
            Serial.print("аптайм: ");
            Serial.println(millis() >> 10);
            // for (int i = 0; i < currentIndex; i++) {
            //     Serial.print(i);
            //     Serial.print(":\t\t");
            //     Serial.println(database[i]);
            // }
        }
        else if (receivedChar == 'i')
        {
            Serial.print("аптайм: ");
            Serial.println(millis() >> 10);
            // Serial.print("текущий индекс:");
            // Serial.println(currentIndex);
        }
    }
} // parse_serial()

void setup()
{
    Serial.begin(115200);
    Serial1.begin(9600, SERIAL_8N1, RX_SCANER, TX_SCANER);
    pinMode(39, INPUT);
    pinMode(ENTER_PIN, OUTPUT);
    digitalWrite(ENTER_PIN, OFF);
    pinMode(INDIKATOR, OUTPUT);
    digitalWrite(INDIKATOR, LOW);

    while (!Serial && millis() < 2000)
        ;
    Serial.setDebugOutput(true);
    WT32_ETH01_onEvent(); // To be called before ETH.begin()
    ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER);
    ETH.config(myIP, myGW, mySN, myDNS);
    WT32_ETH01_waitForConnect();
    Serial.print(F("\nHTTP WebClient is @ IP : "));
    Serial.println(ETH.localIP());
}

void loop()
{
    parse_serial();
    if (scanDMcode())
    {
        int result = passDMCode(dm_number);
        Serial.print("Srv resp: ");
        Serial.println(result);
        if (result == 200)
        {
            // Открываем дверь на 2 секунды
            digitalWrite(ENTER_PIN, ON);
            delay(2000);
            digitalWrite(ENTER_PIN, OFF);
        }
        else 
        {
            Serial.print("\n\n \t\t !! не успешно:\n код ответа");
            Serial.println(result);
            // Таймаут — мигаем 2 раза по 700 мс
            blinkLed(2, 700);
        }
        // else
        // {
        //     // Любая другая ошибка — мигаем 3 раза по 200 мс
        //     blinkLed(3, 200);
        // }
        // Очищаем буфер сканера
        while (Serial1.available())
            Serial1.read();
    } // scan

    if (!digitalRead(39))
    {
        if (!digitalRead(39))
        {
            delay(20);
            while (!digitalRead(39))
                delay(50);

            Serial.println("test 1234567890 passed");
            // int res = passDMCode(01A1B2C3D4E5F6170102521123456789);
            int res = passDMCode(123456789);
            Serial.print("Srv resp: ");
            Serial.println(res);
            delay(500);
        }
    } // if digRead
}

// // просто пример от автора либы

// #define DEBUG_ETHERNET_WEBSERVER_PORT       Serial

// // Debug Level from 0 to 4
// #define _ETHERNET_WEBSERVER_LOGLEVEL_       3

// #include <WebServer_WT32_ETH01.h>
// #include <HTTPClient.h>

// // Select the IP address according to your local network
// IPAddress myIP(192, 168, 2, 232);
// IPAddress myGW(192, 168, 2, 1);
// IPAddress mySN(255, 255, 255, 0);

// // Google DNS Server IP
// IPAddress myDNS(8, 8, 8, 8);

// void setup()
// {
//   Serial.begin(115200);

//   while (!Serial);

//   // Using this if Serial debugging is not necessary or not using Serial port
//   //while (!Serial && (millis() < 3000));

//   Serial.print("\nStarting BasicHttpClient on " + String(ARDUINO_BOARD));
//   Serial.println(" with " + String(SHIELD_TYPE));
//   Serial.println(WEBSERVER_WT32_ETH01_VERSION);

//   // To be called before ETH.begin()
//   WT32_ETH01_onEvent();

//   //bool begin(uint8_t phy_addr=ETH_PHY_ADDR, int power=ETH_PHY_POWER, int mdc=ETH_PHY_MDC, int mdio=ETH_PHY_MDIO,
//   //           eth_phy_type_t type=ETH_PHY_TYPE, eth_clock_mode_t clk_mode=ETH_CLK_MODE);
//   //ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_TYPE, ETH_CLK_MODE);
//   ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER);

//   // Static IP, leave without this line to get IP via DHCP
//   //bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1 = 0, IPAddress dns2 = 0);
//   ETH.config(myIP, myGW, mySN, myDNS);

//   WT32_ETH01_waitForConnect();
// }

// void loop()
// {
//   if (WT32_ETH01_isConnected())
//   {
//     HTTPClient http;

//     Serial.print("[HTTP] begin...\n");
//     // configure traged server and url
//     //http.begin("https://www.howsmyssl.com/a/check", ca); //HTTPS
//     http.begin("http://example.com/index.html"); //HTTP

//     Serial.print("[HTTP] GET...\n");
//     // start connection and send HTTP header
//     int httpCode = http.GET();

//     // httpCode will be negative on error
//     if (httpCode > 0)
//     {
//       // HTTP header has been send and Server response header has been handled
//       Serial.printf("[HTTP] GET... code: %d\n", httpCode);

//       // file found at server
//       if (httpCode == HTTP_CODE_OK)
//       {
//         String payload = http.getString();
//         Serial.println(payload);
//       }
//     }
//     else
//     {
//       Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
//     }

//     http.end();
//   }

//   delay(5000);
// }