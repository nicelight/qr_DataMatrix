// отладка бэктрейсов
// ~/.platformio/packages/toolchain-xtensa-esp32/bin/xtensa-esp32-elf-addr2line -pfiaC -e "C:\Users\Acer\Documents\PlatformIO\Projects\qr_DataMatrix\.pio\build\v02-client\firmware.elf" 0x400dcbf1:0x3ffb21f0 0x400dcc3a:0x3ffb2210 0x400d3156:0x3ffb2230 0x400e2f7e:0x3ffb2290

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ETH.h>

#include "led.h"
#include "sets.h"
#include "timer.h"
// #include "nastroyki.h"
// const char *serverUrl = "http://192.168.10.250/api/callback";

#include <VL53L1X.h>
#include <Wire.h>
VL53L1X dalnomer;  // для фиксации прохода через турникет
bool dalnomer_enabled = false;

#include "PCF8575.h"
PCF8575 PCF1(0x20);

// источник кода ethernet клиента
// https://github.com/khoih-prog/AsyncHTTPRequest_Generic/blob/master/examples/WT32_ETH01/AsyncHTTPRequest_WT32_ETH01/AsyncHTTPRequest_WT32_ETH01.ino

// Level from 0-4
#define ASYNC_HTTP_DEBUG_PORT Serial
#define _ASYNC_HTTP_LOGLEVEL_ 1
#define _ETHERNET_WEBSERVER_LOGLEVEL_ 1

#include <HTTPClient.h>
#include <WebServer_WT32_ETH01.h>  // https://github.com/khoih-prog/WebServer_WT32_ETH01

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
// String dm_string = "";

long dm_number = 1000000000;

String scanDMcode() {
    if (Serial1.available() > 0) {
        newScan = true;
        dm_string = "";
        uint32_t startTime = millis();
        while (millis() - startTime < 100ul) {
            while (Serial1.available() > 0) {
                char c = Serial1.read();
                dm_string += c;
                startTime = millis();
            }
        }
        if (dm_string.length() >= 32) {
            String sub_str = dm_string.substring(23, 32);
            for (int i = 0; i < sub_str.length(); i++) {
                if (!isDigit(sub_str.charAt(i))) {
                    Serial.println("[scanDMcode] Не все символы цифры!");
                    Serial.print("[scanDMcode] dm_string: ");
                    Serial.println(dm_string);
                    Serial.print("[scanDMcode] sub_str: ");
                    Serial.println(sub_str);
                    dm_number = 1000000000;
                    return "";
                }
            }
            dm_number = sub_str.toInt();
            Serial.print("[scanDMcode] dm_number: ");
            Serial.println(dm_number);
        }
        Serial.print("[scanDMcode] возвращаемая строка: ");
        dm_string.trim();
        Serial.println(dm_string);
        return dm_string;
    }
    dm_number = 1000000000;
    return "";
}

int passDMCode(const String &code_str) {
    if (code_str == SERVICE_QR) {
        return 20;
    }
    // если не подключен Ethernet выйдем
    if (!WT32_ETH01_isConnected())
        return -1;

    static bool requestOpenResult;
    // Формируем JSON
    JsonDocument doc;
    JsonArray arr = doc["passIds"].to<JsonArray>();
    arr.add(code_str);
    doc["code"] = "TURNIKET";
    String jsonStr;
    serializeJson(doc, jsonStr);

    const char *serverUrl = SERVERURL;
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");  // Установка заголовков :contentReference[oaicite:4]{index=4}

    Serial.print("Connecting to srv ");
    Serial.print(serverUrl);
    Serial.print("\n\tRequest POST:\n");
    Serial.print(jsonStr);
    // int httpCode = http.POST(jsonStr);
    httpCode = 0;                   // стираем старые данные из глоб переменной
    httpCode = http.POST(jsonStr);  // отправка POST
    if (httpCode > 0) {
        // Serial.printf("\nSRV code: %d\n", httpCode);
        // String response = http.getString();
        srvResponse = http.getString();
        Serial.print(" \n\t||\n\t||\n\tAnswer:\n");
        Serial.println(srvResponse);
        newPass = true;
        http.end();  // Освобождение ресурсов
        // пропуск по сервисному коду
        return httpCode;
    } else {
        Serial.printf("\n\n\t POST failed, error: %s\n", http.errorToString(httpCode).c_str());
        srvResponse = "None";
        newPass = true;
        http.end();  // Освобождение ресурсов
        return -2;
    }

}  // passDMCode

void parse_serial() {
    if (Serial.available() > 0) {
        char receivedChar = Serial.read();
        if (receivedChar == 'd') {
            Serial.print("аптайм: ");
            Serial.println(millis() >> 10);
            // for (int i = 0; i < currentIndex; i++) {
            //     Serial.print(i);
            //     Serial.print(":\t\t");
            //     Serial.println(database[i]);
            // }
        } else if (receivedChar == 'i') {
            Serial.print("аптайм: ");
            Serial.println(millis() >> 10);
            // Serial.print("текущий индекс:");
            // Serial.println(currentIndex);
        }
    }
}  // parse_serial()

void setup() {
    Serial.begin(115200);
    Serial1.begin(9600, SERIAL_8N1, RX_SCANER, TX_SCANER);
    Wire.begin(5, 17);
    Wire.setClock(400000);  // use 400 kHz I2C
    // // расширитель пинов
    // PCF1.pinMode(SET1, INPUT);
    // PCF1.pinMode(SET2, INPUT);
    // PCF1.pinMode(SET3, INPUT);
    // PCF1.pinMode(P0, OUTPUT);
    // PCF1.digitalWrite(P0, HIGH);

    dalnomer.setTimeout(500);
    // reduce timing budget to 20 ms (default is about 33 ms)
    dalnomer_enabled = dalnomer.init();
    if (!dalnomer_enabled) Serial.println("Init dalnomer failed!");
    else {
        Serial.println("Init dalnomer -- OK!");
        dalnomer.setMeasurementTimingBudget(20000);
        dalnomer.startContinuous(50);
    }

    while (!Serial && millis() < 2000);
    // инициализация ETHERNET
    Serial.setDebugOutput(true);
    WT32_ETH01_onEvent();  // To be called before ETH.begin()
    ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER);
    ETH.config(myIP, myGW, mySN, myDNS);
    WT32_ETH01_waitForConnect();
    Serial.print("\nHTTP WebClient on IP : ");
    Serial.println(ETH.localIP());
    // инициализация веб морды (todo)
    // sett_begin();

    // I2C Scanner
}  // setup

enum steps {
    INIT,
    SCAN_QR,
    WRONG_QR,
    OPEN_DOOR,
    WAIT_FOR_OPENING,
    WAIT_FOR_ENTERING,
    AFTERENTER_DELAY,
    CLOSING_DOOR,
    WAIT_FOR_CLOSING
};
steps proc = INIT;
uint32_t procMs = 0;
String code_str = "";
int dist = 1000;

LED status(0, 2000, 1, 200, 70);  // каждые 1000 милисек мигаем 3 раза

bool statusState = 0;
uint32_t statusOn = 100;
uint32_t statusOff = 900;
uint32_t statusOnMs = 0;
uint32_t statusOffMs = 0;
void loop() {
    // sett_loop();
    status.tick();
    // status.setPeriod(1000, 3, 100, 30);  // каждые 1000 милисек мигаем 3 раза каждых 100 мс, время горения 30 мсек

    // code_str = scanDMcode();
    // Serial.print("code_str: ");
    // Serial.println(code_str);
    // Serial.print("\n\n\n");
    //

    // основной автомат
    switch (proc) {
        case INIT:
            procMs = millis();
            while (Serial1.available())
                Serial1.read();
            proc = SCAN_QR;
            Serial.println("SCAN QR");
            break;
        case SCAN_QR:  // сканируем qr код
            parse_serial();
            code_str = scanDMcode();
            if (code_str.length() > 0) {
                int result = passDMCode(code_str);
                Serial.print("Srv resp: ");
                Serial.println(result);
                switch (result) {
                    case -1:         // нет связи ETHERNET
                        delay(500);  // заглушка TODO
                        break;
                    case -2:         // не удалось подключиться к серверу
                        delay(500);  // заглушка TODO
                        break;
                    case 20:
                        proc = OPEN_DOOR;
                        break;
                    case 200:
                        proc = OPEN_DOOR;
                        break;
                    case 400:
                        proc = WRONG_QR;
                        break;
                }  // switch result
                while (Serial1.available())
                    Serial1.read();
            }  // scan exist
            break;
        case WRONG_QR:
            Serial.println("REJECTED QR");
            proc = INIT;
            break;
        case OPEN_DOOR:
            Serial.println("OPENING DOOR");
            // digitalWrite(ENTER_PIN, ON);
            PCF1.digitalWrite(DOOR_STATUS, 1);   // светодиод открытой двери
            PCF1.digitalWrite(DOOR, 0);          // открываем дверь
            status.setPeriod(500, 1, 500, 450);  // раз в пол сек иногда тухнем
            procMs = millis();
            proc = WAIT_FOR_OPENING;
            break;
            // старая глючновая реализация, тупит на вход
        // case WAIT_FOR_OPENING:
        //     if (millis() - procMs > OPENINIG_DELAY)  // заглушка TODO
        //     {
        //         procMs = millis();
        //         Serial.print("WAITING FOR ENTERING..");
        //         if (LASER_DALNOMER) {
        //             proc = WAIT_FOR_ENTERING;
        //         } else {
        //             proc = AFTERENTER_DELAY;
        //         }
        //     }  // if ms
        //     break;
        case WAIT_FOR_OPENING:

            // if (LASER_DALNOMER) {
            if (1) {
                if (millis() - procMs > 500ul) {
                    procMs = millis();
                    Serial.print("WAITING FOR ENTERING..");
                    proc = WAIT_FOR_ENTERING;
                }  // if ms
            } else {  // дальномера нет, тупо ждем
                if (millis() - procMs > OPENINIG_DELAY) {
                    procMs = millis();
                    proc = AFTERENTER_DELAY;
                }
            }

            break;
        case WAIT_FOR_ENTERING:
            if (millis() - procMs > 10000ul) {
                Serial.println("..TIMEOUT");
                procMs = millis();
                Serial.println("AFTERENTER_DELAY");
                proc = AFTERENTER_DELAY;
            }  // if ms
            // проверяеем дальномером если тело прошло
            dist = dalnomer.read();
            Serial.print("DISTANCE: ");
            Serial.println(dist);
            if ((dist > 20) && (dist < 500)) {
                Serial.print("..DISTANCE ");
                Serial.println(dist);
                procMs = millis();
                Serial.println("AFTERENTER_DELAY");
                proc = AFTERENTER_DELAY;
                status.setPeriod(500, 1, 500, 50);  // раз в пол сек иногда тухнем

            }  // if ms
            break;
        case AFTERENTER_DELAY:
            if (millis() - procMs > 2000ul) {
                proc = CLOSING_DOOR;
            }
            break;
        case CLOSING_DOOR:
            Serial.println("CLOSING DOOR");
            // digitalWrite(ENTER_PIN, OFF);
            PCF1.digitalWrite(DOOR_STATUS, 0);   // светодиод открытой двери
            PCF1.digitalWrite(DOOR, 1);          // открываем дверь
            status.setPeriod(2000, 1, 200, 70);  // раз в пол сек иногда тухнем
            procMs = millis();
            proc = WAIT_FOR_CLOSING;
            break;
        case WAIT_FOR_CLOSING:
            if (millis() - procMs > 1000ul) {
                proc = INIT;
            }
            break;
    }  // switch proc

    // // тесты
    // if (!digitalRead(39))
    // {
    //     if (!digitalRead(39))
    //     {
    //         delay(20);
    //         while (!digitalRead(39))
    //             delay(50);
    //         Serial.println("test 01A1B2C3D4E5F6170102521123456789 passed");
    //         int res = passDMCode("01A1B2C3D4E5F6170102521123456789");
    //         Serial.print("Srv resp: ");
    //         Serial.println(res);
    //         delay(500);
    //         for (int i = 0; i < 100; i++)
    //         {
    //             Serial.print("дальномер: ");
    //             Serial.println(dalnomer.readRangeSingleMillimeters());
    //             if (dalnomer.timeoutOccurred())
    //             {
    //                 Serial.print(" TIMEOUT");
    //             }
    //         }
    //         Serial.println('\n\n\n');
    //     }
    // } // test

    // тестируем работу дальномера
    // Serial.println(dalnomer.readRangeSingleMillimeters());
    // if (dalnomer.timeoutOccurred())
    // {
    //     Serial.print(" TIMEOUT");
    // }

    // // I2C Scanner
    // byte error, address;
    // int nDevices;
    // Serial.println("Scanning...");
    // nDevices = 0;
    // for (address = 8; address < 127; address++) {
    //     Wire.beginTransmission(address);
    //     error = Wire.endTransmission();
    //     if (error == 0) {
    //         Serial.print("I2C device found at address 0x");
    //         if (address < 16)
    //             Serial.print("0");
    //         Serial.print(address, HEX);
    //         Serial.println(" !");
    //         nDevices++;
    //     } else if (error == 4) {
    //         Serial.print("Unknow error at address 0x");
    //         if (address < 16)
    //             Serial.print("0");
    //         Serial.println(address, HEX);
    //     }
    // }  // for
    // if (nDevices == 0) Serial.println("No I2C devices found\n");

}  // loop
