#include <Arduino.h>

#include "led.h"
#include "nastroyki.h"
#include "sets.h"
#include "timer.h"

#define _ASYNC_WEBSERVER_LOGLEVEL_ 2

// Select the IP address according to your local network
IPAddress myIP(192, 168, 10, 231);
IPAddress myGW(192, 168, 10, 1);
IPAddress mySN(255, 255, 255, 0);
// Google DNS Server IP
IPAddress myDNS(8, 8, 8, 8);
// получаем строку последних октетов айпишника
String stringEthIp = String(myIP[2]) + "." + String(myIP[3]);

// https://github.com/khoih-prog/AsyncWebServer_WT32_ETH01/tree/main/examples/AsyncSimpleServer_WT32_ETH01

#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <AsyncWebServer_WT32_ETH01.h>
#define DATABASE_SIZE 1000
long database[DATABASE_SIZE];  // база со всеми DM кодами
byte quantity[DATABASE_SIZE];  // база с количеством посещений

int currentIndex = 1;         // текущий индекс в базе
String dm_string = "";        // Переменная для хранения считанной сканером строки
long dm_number = 1000000000;  // Переменная для считанного полезного DM кода из строки , будет сравниваться с базой. Дефолт за пределами 9 знаков

// для мультисерверности
// https://github.com/khoih-prog/AsyncWebServer_WT32_ETH01/blob/main/examples/AsyncMultiWebServer_WT32_ETH01/AsyncMultiWebServer_WT32_ETH01.ino

// пример POST запроса
// curl -X POST -H 'Content-Type: application/json' -d '{"FirstName":"Joe","LastName":"Soap"}' http://192.168.10.231/post

AsyncWebServer ETHserver(80);
// const char* PARAM_MESSAGE = "message";

bool scanDMcode() {
    if (Serial1.available() > 0) {
        Serial.println("Serial1 avaliable");
        dm_string = "";  // Очищаем строку перед новым чтением
        // Читаем данные, пока поступают символы. Если 100(50) мс без новых данных, считаем, что пакет завершился.
        uint32_t startTime = millis();
        while (millis() - startTime < 100ul) {
            while (Serial1.available() > 0) {
                char c = Serial1.read();
                dm_string += c;
                startTime = millis();  // Сброс таймера при поступлении нового символа
            }
        }
        // Обрезаем строку от лишних пробелов и управляющих символов
        // dm_string.trim();

        Serial.print("Получена строка: ");
        Serial.println(dm_string);
        // Проверка: строка должна быть не менее 33 символов для извлечения индексов 23-32
        if (dm_string.length() >= 32) {
            // Извлечение подстроки с символами с 23-го по 32-й индекс (10 символов)
            String sub_str = dm_string.substring(23, 32);
            Serial.print("Извлечённая подстрока: ");
            Serial.println(sub_str);

            // Проверка, что все символы подстроки являются цифрами
            bool valid = true;
            for (int i = 0; i < sub_str.length(); i++) {
                if (!isDigit(sub_str.charAt(i))) {
                    valid = false;
                    Serial.println("ERR: в DM коде не только цифры");
                    return 0;
                }
            }
            // Преобразование подстроки в число типа long
            dm_number = sub_str.toInt();
            Serial.print("Преобразованное число: ");
            Serial.println(dm_number);
            return 1;
        } else {
            Serial.println("Ошибка: полученная строка слишком короткая.");
        }
    }
    dm_number = 1000000000;  // сброс результата скана
    return 0;
}  // scanDMcode()

byte handleJSON(String& body) {
    // Serial.print("POST body: ");
    // Serial.println(body);
    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, body);
    if (error) {
        Serial.print("JSON parse error: ");  //"Invalid JSON"
        Serial.println(error.c_str());
        return 4;
    }

    const char* action = doc["action"];
    if (action == nullptr) {
        Serial.println("JSON: Ключ 'action' не найден или содержит некорректное значение.");
        return 4;
    }
    if (strcmp(action, "add") == 0) {
        if (!doc.containsKey("data")) {
            Serial.println("JSON: Отсутствует поле 'data'");
            return 4;
        }
        if (!doc["data"].is<JsonArray>()) {
            Serial.println("JSON: Поле 'data' не является массивом");
            return 4;
        }
        JsonArray data = doc["data"].as<JsonArray>();
        // Проверка, что массив 'data' не пуст
        if (data.size() == 0) {
            Serial.println("JSON: Массив 'data' пуст.");  // 400
            return 4;
        }
        for (JsonVariant v : data) {
            String code = v.as<String>();
            // Проверка длины кода
            // Serial.print("code.length() ");
            // Serial.println(code.length());
            if (code.length() != 32) {
                Serial.println("JSON: Неверная длина DM кода: " + code);  // 400
                return 4;
            }
            // Извлечение подстроки: символы с позиций 23..32 (1-индексация) => [24,33) в 0-индексации
            String subStr = code.substring(23, 32);
            long value = subStr.toInt();
            if (currentIndex < DATABASE_SIZE) {
                bool found = false;
                for (int i = 0; i < currentIndex; i++) {
                    if (database[i] == value) {
                        Serial.print("JSON:  DM: " + String(value));
                        Serial.println(" уже в базе. Индекс" + String(i));
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    database[currentIndex++] = value;
                    Serial.println("JSON: В базу добавлен DM: " + String(value));
                    Serial.println(", index: " + String(currentIndex));
                }
            } else {
                Serial.println("JSON: База заполнена. Не могу добавить: " + String(value));  // 507
                return 5;
            }
        }
    } else if (strcmp(action, "delete") == 0) {
        JsonArray data = doc["data"].as<JsonArray>();
        for (JsonVariant v : data) {
            String code = v.as<String>();
            // Проверка длины кода
            // Serial.print("code.length() ");
            // Serial.println(code.length());
            if (code.length() != 32) {
                Serial.println("JSON: Неверная длина DM кода: " + code);  // 400
                return 4;
            }
            String subStr = code.substring(23, 32);
            long value = subStr.toInt();
            bool found = false;
            for (int i = 0; i < currentIndex; i++) {
                if (database[i] == value) {
                    database[i] = 0;
                    Serial.print("Удален DM: " + String(value));
                    Serial.println(", index: " + String(i));
                    found = true;
                    break;
                }
            }
            if (!found) {
                Serial.println("Для удаления не найдена: " + String(value));
                // return 4;
            }
        }
    } else if (strcmp(action, "removeall") == 0) {
        for (int i = 0; i < currentIndex; i++) {
            database[i] = 0;
            quantity[i] = 0;
        }
        currentIndex = 0;
        Serial.println("Все элементы в базе обнулены.");
    } else {
        Serial.println("Неизвестное действие: " + String(action));  // 400
        return 4;
    }
    // server.send(200, "text/plain", "OK");
    return 2;
}  // handleJSON

void handlePostBody(AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
    static String body = "";
    // Если индекс равен нулю, начинаем заново
    if (index == 0) {
        body = "";
    }
    // Добавляем полученные данные
    for (size_t i = 0; i < len; i++) {
        body += (char)data[i];
    }
    // Если получены все данные, парсим их
    if (index + len >= total) {
        byte result;
        result = handleJSON(body);  // обработка JSON
        if (result == 2)
            request->send(200, "text/plain", "\tGOT POST with BODY:\n" + body);
        else if (result == 4)
            request->send(400, "text/plain", "Parsing unsuccesfull. Try again:\n" + body);
        else if (result == 5)
            request->send(507, "text/plain", "Database is full, cant save:\n" + body);
    }
}  // handlePostBody

void handleEthRoot(AsyncWebServerRequest* request) {
    request->send(200, F("text/html"), "root page from separet func");
}

void handleEthnotFound(AsyncWebServerRequest* request) {
    request->send(404, "text/plain", "\n\n \t\t @smartfarm_diy \n\n\n\t\t\t\t\t No such page. 404");
}

void parse_serial() {
    if (Serial.available() > 0) {
        char receivedChar = Serial.read();
        if (receivedChar == 'd') {
            Serial.print("аптайм: ");
            Serial.println(millis() >> 10);
            for (int i = 0; i < currentIndex; i++) {
                Serial.print(i);
                Serial.print(":\t\t");
                Serial.println(database[i]);
            }
        } else if (receivedChar == 'i') {
            Serial.print("аптайм: ");
            Serial.println(millis() >> 10);
            Serial.print("текущий индекс:");
            Serial.println(currentIndex);
        }
    }
}  // parse_serial()

byte valid_customer() {
    // начинаем с единицы, т.к. нулевой элемент в базе зарезервирован под сервисный браслет
    for (int i = 1; i < currentIndex; i++) {
        if (database[i] == dm_number) {
            return ++quantity[i];  // количество посещений
        }
    }
    return 0;
}  // valid_customer()

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial1.begin(9600, SERIAL_8N1, RX_SCANER, TX_SCANER);

    for (int i = 0; i < DATABASE_SIZE; i++) {
        database[i] = 0;
        quantity[i] = 0;
    }

    pinMode(ENTER_PIN, OUTPUT);
    digitalWrite(ENTER_PIN, OFF);
    pinMode(EXIT_PIN, OUTPUT);
    digitalWrite(EXIT_PIN, OFF);
    
    while (!Serial && millis() < 2000);

    Serial.print(F("\nStart AsyncSimpleServer_WT32_ETH01 on "));
    Serial.print(BOARD_NAME);
    Serial.print(F(" with "));
    Serial.println(SHIELD_TYPE);
    Serial.println(ASYNC_WEBSERVER_WT32_ETH01_VERSION);
    WT32_ETH01_onEvent();  // To be called before ETH.begin()
    ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER);
    ETH.config(myIP, myGW, mySN, myDNS);
    WT32_ETH01_waitForConnect();
    IPAddress localEthIP = ETH.localIP();
    Serial.print(F("Ethernet IP: "));
    Serial.println(localEthIP);

    ETHserver.on("/", HTTP_GET, [](AsyncWebServerRequest* request) { handleEthRoot(request); });
    ETHserver.on("/post", HTTP_POST, [](AsyncWebServerRequest* request) {}, NULL, handlePostBody);
    ETHserver.onNotFound([](AsyncWebServerRequest* request) { handleEthnotFound(request); });
    ETHserver.begin();

    // sett_begin(); //wifi морда Settings Alex Gyver
    // Serial.print("\n\t\t\t INIT \n\t wifi ssid: ");
    // Serial.println(db[kk::wifi_ssid]);  // из settings.h доступны db и ключи

}  // setup()

void loop() {
    // sett_loop();  // wifi морда Settings Alex Gyver. не работает с ETH
    parse_serial();  // отладка

    Serial1.flush();  // очистка если там что то насыпали
    if (scanDMcode()) {
        // Serial.println("отсканированный DM: " + String(dm_number));
        byte customer_count = valid_customer();  // если
        if (customer_count) {
            if (customer_count == 1) {  // проходит первый раз
                Serial.println("1-й вхоД ");
                digitalWrite(ENTER_PIN, ON);
                delay(FIRST_ENTER_DELAY);
                digitalWrite(ENTER_PIN, OFF);
            } else if (customer_count == 2){
                Serial.println("\t1-й ВЫход ");  
                digitalWrite(EXIT_PIN, ON);
                delay(FIRST_EXIT_DELAY);
                digitalWrite(EXIT_PIN, OFF);
            } else if (customer_count % 2 == 1){
                Serial.println("\t\tснова вхоДит ");
                digitalWrite(ENTER_PIN, ON);
                delay(MULTIPLE_OPEN_DELAY);
                digitalWrite(ENTER_PIN, OFF);
            } else if (customer_count % 2 == 0){
                Serial.println("\t\t\tснова ВЫходит ");
                digitalWrite(EXIT_PIN, ON);
                delay(MULTIPLE_OPEN_DELAY);
                digitalWrite(EXIT_PIN, OFF);
            }
        } else {
            Serial.print("НЕ валидный посетитель");
        }
    }

}  // loop