#include <Arduino.h>
#include <ETH.h>
#include <ArduinoJson.h>
#include "nastroyki.h"

IPAddress myIP(IP_OCT1, IP_OCT2, IP_OCT3, IP_OCT4);
IPAddress myGW(IP_OCT1, IP_OCT2, IP_OCT3, IP_GATE);
IPAddress mySN(255, 255, 255, 0);
IPAddress myDNS(8, 8, 8, 8);

IPAddress serverIP(SERVER_OCT1, SERVER_OCT2, SERVER_OCT3, SERVER_OCT4);
const uint16_t serverPort = 80;
const char* serverPath = "/api/callback";

#define HTTP_TIMEOUT 3000  // мс

String dm_string = "";
long dm_number = 1000000000;

bool scanDMcode() {
    if (Serial1.available() > 0) {
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

void blinkLed(int times, int duration) {
    for (int i = 0; i < times; i++) {
        digitalWrite(INDIKATOR, HIGH);
        delay(duration);
        digitalWrite(INDIKATOR, LOW);
        delay(duration);
    }
}

bool sendDMCode(long dm_number) {
    WiFiClient client;
    if (!client.connect(serverIP, serverPort)) {
        Serial.println("Ошибка подключения к серверу");
        return false;
    }

    // Формируем JSON
    JsonDocument doc;
    JsonArray arr = doc["passIds"].to<JsonArray>();
    arr.add(String(dm_number));
    doc["code"] = "TURNIKET";
    String jsonStr;
    serializeJson(doc, jsonStr);

    // Формируем HTTP POST запрос
    String request =
        String("POST ") + serverPath + " HTTP/1.1\r\n" +
        "Host: " + serverIP.toString() + "\r\n" +
        "Content-Type: application/json\r\n" +
        "Content-Length: " + jsonStr.length() + "\r\n" +
        "Connection: close\r\n\r\n" +
        jsonStr;

    client.print(request);

    uint32_t t_start = millis();
    while (!client.available() && millis() - t_start < HTTP_TIMEOUT) {
        delay(10);
    }
    if (!client.available()) {
        client.stop();
        return -1; // таймаут
    }

    // Читаем ответ
    String response;
    int httpCode = 0;
    while (client.available()) {
        String line = client.readStringUntil('\n');
        if (line.startsWith("HTTP/1.1")) {
            httpCode = line.substring(9, 12).toInt();
        }
        response += line + "\n";
    }
    client.stop();
    return httpCode == 200;
}

void setup() {
    Serial.begin(115200);
    Serial1.begin(9600, SERIAL_8N1, RX_SCANER, TX_SCANER);

    pinMode(ENTER_PIN, OUTPUT);
    digitalWrite(ENTER_PIN, OFF);
    pinMode(INDIKATOR, OUTPUT);
    digitalWrite(INDIKATOR, LOW);

    ETH.begin();
    ETH.config(myIP, myGW, mySN, myDNS);

    Serial.print("Ethernet IP: ");
    Serial.println(ETH.localIP());
}

void loop() {
    if (scanDMcode()) {
        int result = sendDMCode(dm_number);
        if (result == 1) {
            // Открываем дверь на 2 секунды
            digitalWrite(ENTER_PIN, ON);
            delay(2000);
            digitalWrite(ENTER_PIN, OFF);
        } else if (result == -1) {
            // Таймаут — мигаем 2 раза по 700 мс
            blinkLed(2, 700);
        } else {
            // Любая другая ошибка — мигаем 3 раза по 200 мс
            blinkLed(3, 200);
        }
        // Очищаем буфер сканера
        while (Serial1.available()) Serial1.read();
    }
}