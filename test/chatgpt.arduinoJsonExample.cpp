// https://chatgpt.com/c/67d6ec70-a080-8012-967b-05fa3143eab2

/* промпт:
Необходимо разработать архитектуру программы для микроконтроллера ESP32, который будет иметь веб сервер и получать по сети POST запрос со следующим JSON:

json{  "action": "add",  "data": ["01A1B2C3D4E5F61701025214123456789", "01A1B2C3D4E5F61701025213999999999", "01A1B2C3D4E5F6170102521123456789", "01A1B2C3D4E5F6171212521000000005", "01A1B2C3D4E5F6171212621400000000"]}

 JSON имеет в поле "data" массив от 1 до 50 однотипных элементов ( datamatrix коды). Поле "action" имеет один из трех ключей "add" или"delete" или"removeall". В оперативной памяти хранится база из 1000 элементов.  если action  с ключем add, то нужно распарсить массив из поля data и для каждого элемента массива получить ряд символов, которые стоят на на порядковых номерах 23..31. Допустим, для элемента "01A1B2C3D4E5F61701025214123456789" должно получиться "123456789". Далее преобразуем получившийся String в int  и записываем в базу данных , запоминая индекс.Каждый новый элемент записываем по новому индексу. Если индекс достиг 1000, не записываем, выводим в серийный порт ошибку. В случае, если "action" содержит ключ "delete" мы аналогичным образом получаем int из String ищем, если такое значение естьв базе, то заменяем его на 0.В случае, если "action" содержит ключ "removeall" мы записываем все значения всех элементов в базе равными 0
*/ 

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

WebServer server(80);
#define DATABASE_SIZE 1000
long database[DATABASE_SIZE];
int currentIndex = 0;

void handleJSON() {
  String body = server.arg("plain");
  Serial.println("Received POST: " + body);

  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, body);
  if(error) {
    Serial.print("JSON parse error: ");
    Serial.println(error.c_str());
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }

  const char* action = doc["action"];
  
  if(strcmp(action, "add") == 0) {
    JsonArray data = doc["data"].as<JsonArray>();
    for(JsonVariant v : data) {
      String code = v.as<String>();
      // Проверка длины кода
      if(code.length() < 33) {
        Serial.println("Неверная длина кода: " + code);
        continue;
      }
      // Извлечение подстроки: символы с позиций 25..33 (1-индексация) => [24,33) в 0-индексации
      String subStr = code.substring(24, 33);
      long value = subStr.toInt();
      if(currentIndex < DATABASE_SIZE) {
        database[currentIndex++] = value;
        Serial.println("Добавлено: " + String(value));
      } else {
        Serial.println("База заполнена. Не могу добавить: " + String(value));
      }
    }
  }
  else if(strcmp(action, "delete") == 0) {
    JsonArray data = doc["data"].as<JsonArray>();
    for(JsonVariant v : data) {
      String code = v.as<String>();
      if(code.length() < 33) {
        Serial.println("Неверная длина кода: " + code);
        continue;
      }
      String subStr = code.substring(24, 33);
      long value = subStr.toInt();
      bool found = false;
      for (int i = 0; i < currentIndex; i++) {
        if(database[i] == value) {
          database[i] = 0;
          Serial.println("Удалено: " + String(value));
          found = true;
          break;
        }
      }
      if(!found)
        Serial.println("Не найден: " + String(value));
    }
  }
  else if(strcmp(action, "removeall") == 0) {
    for (int i = 0; i < currentIndex; i++) {
      database[i] = 0;
    }
    Serial.println("Все элементы обнулены.");
  }
  else {
    Serial.println("Неизвестное действие: " + String(action));
    server.send(400, "text/plain", "Unknown action");
    return;
  }
  
  server.send(200, "text/plain", "OK");
}//handleJSON

void setup() {
  Serial.begin(115200);
  // Инициализация базы
  for (int i = 0; i < DATABASE_SIZE; i++) {
    database[i] = 0;
  }
  // Подключение к WiFi
  WiFi.begin(ssid, password);
  Serial.print("Подключение к WiFi");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  
  // Регистрация обработчика POST-запросов
  server.on("/", HTTP_POST, handleJSON);
  server.begin();
  Serial.println("Сервер запущен.");
}

void loop() {
  server.handleClient();
}