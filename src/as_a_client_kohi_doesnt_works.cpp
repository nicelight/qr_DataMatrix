#include <Arduino.h>
#include <ETH.h>
#include <ArduinoJson.h>
#include "led.h"
#include "timer.h"
#include "sets.h"
// #include "nastroyki.h"

// источник кода ethernet клиента
// https://github.com/khoih-prog/AsyncHTTPRequest_Generic/blob/master/examples/WT32_ETH01/AsyncHTTPRequest_WT32_ETH01/AsyncHTTPRequest_WT32_ETH01.ino

// Level from 0-4
#define ASYNC_HTTP_DEBUG_PORT Serial
#define _ASYNC_HTTP_LOGLEVEL_ 1
#define _ETHERNET_WEBSERVER_LOGLEVEL_ 1

#include <WebServer_WT32_ETH01.h> // https://github.com/khoih-prog/WebServer_WT32_ETH01

#define ASYNC_HTTP_REQUEST_GENERIC_VERSION_MIN_TARGET "AsyncHTTPRequest_Generic v1.10.2"
#define ASYNC_HTTP_REQUEST_GENERIC_VERSION_MIN 1010002

// Uncomment for certain HTTP site to optimize
// #define NOT_SEND_HEADER_AFTER_CONNECTED        true

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include <AsyncHTTPRequest_Generic.h> // https://github.com/khoih-prog/AsyncHTTPRequest_Generic
AsyncHTTPRequest request;

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

void requestCB(void *optParm, AsyncHTTPRequest *request, int readyState)
{
    (void)optParm;

    if (readyState == readyStateDone)
    {
        AHTTP_LOGDEBUG(F("\n**************************************"));
        AHTTP_LOGDEBUG1(F("Response Code = "), request->responseHTTPString());

        if (request->responseHTTPcode() == 200)
        {
            Serial.println(F("\n**************************************"));
            Serial.println(request->responseText());
            Serial.println(F("**************************************"));
        }
    }
}

int passDMCode(long dm_number)
{
    static bool requestOpenResult;

    // Формируем JSON
    JsonDocument doc;
    JsonArray arr = doc["passIds"].to<JsonArray>();
    arr.add(String(dm_number));
    doc["code"] = "TURNIKET";
    String jsonStr;
    serializeJson(doc, jsonStr);

    // Формируем HTTP POST запрос
    String postData =
        // String("POST ") + serverPath + " HTTP/1.1\r\n" +
        // "Host: " + serverIP.toString() + "\r\n" +
        String("Content-Type: application/json\r\n") +
        "Content-Length: " + jsonStr.length() + "\r\n" +
        "Connection: close\r\n\r\n" +
        jsonStr;
    Serial.print("\n\nSending POST:\n\n");
    Serial.print(jsonStr);
    // client.print(postData);

    if (request.readyState() == readyStateUnsent || request.readyState() == readyStateDone)
    {
        // requestOpenResult = request.open("GET", "http://worldtimeapi.org/api/timezone/Europe/London.txt");
        // requestOpenResult = request.open("GET", "http://worldtimeapi.org/api/timezone/America/Toronto.txt");
        // requestOpenResult = request.open("POST", (POST_ServerAddress + apiPath + postData).c_str());
        requestOpenResult = request.open("POST", (POST_ServerAddress + apiPath + jsonStr).c_str());
        if (requestOpenResult)
            request.send(); // Only send() if open() returns true, or crash
        else
        {
            Serial.println("Can't reache server");
            return 409; // conflict
        }
    }
    else
    {
        Serial.println("Requests dispatcher doesn't ready. Cancel sending..");
        return 408; // timeout
    }

    for(int i=0; i<20; i++){
        Serial.print(i);
        Serial.print(" reqv = ");
        Serial.println(request.responseHTTPcode());
        delay(200);
    }
    return request.responseHTTPcode();
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
    Serial.print(BOARD_NAME);
    Serial.print(F(" with "));
    Serial.println(SHIELD_TYPE);
    Serial.println(WEBSERVER_WT32_ETH01_VERSION);
    Serial.println(ASYNC_HTTP_REQUEST_GENERIC_VERSION);

    Serial.setDebugOutput(true);

    WT32_ETH01_onEvent(); // To be called before ETH.begin()
    ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER);
    ETH.config(myIP, myGW, mySN, myDNS);
    WT32_ETH01_waitForConnect();
    Serial.print(F("\nHTTP WebClient is @ IP : "));
    Serial.println(ETH.localIP());

    request.setDebug(false);
    request.onReadyStateChange(requestCB); // получение ответа от сервера
}

void loop()
{
    parse_serial();
    if (scanDMcode())
    {
        int result = passDMCode(dm_number);
        Serial.print("Srv resp: ");
        Serial.println(result);
        if (result == 1)
        {
            // Открываем дверь на 2 секунды
            digitalWrite(ENTER_PIN, ON);
            delay(2000);
            digitalWrite(ENTER_PIN, OFF);
        }
        else if (result == -1)
        {
            // Таймаут — мигаем 2 раза по 700 мс
            blinkLed(2, 700);
        }
        else
        {
            // Любая другая ошибка — мигаем 3 раза по 200 мс
            blinkLed(3, 200);
        }
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
            int res = passDMCode(1234567890);
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