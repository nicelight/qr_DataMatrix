#include <WebServer_WT32_ETH01.h>
#include <HTTPClient.h>

// Библиотека является обёрткой вокруг ESP32 WebServer и HTTPClient
// GitHub
// .

// 2. Конфигурация Ethernet-соединения

//     Определите параметры сети (можно оставить без config() для DHCP):

IPAddress localIP(192, 168, 2, 232);
IPAddress gateway(192, 168, 2, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);

// В setup() инициализируйте Ethernet и дождитесь подключения:
void setup()
{
    Serial.begin(115200);
    WT32_ETH01_onEvent(); // Подписка на события Ethernet
    ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER);
    ETH.config(localIP, gateway, subnet, dns);
    WT32_ETH01_waitForConnect(); // Ожидание получения IP
}
// Эта процедура аналогична примеру BasicHttpClient
// GitHub
// 3. Реализация POST-запроса

//     В loop() проверьте соединение и отправьте POST-запрос:
loop()
{
    if (WT32_ETH01_isConnected())
    {
        HTTPClient http;
        const char *serverUrl = "http://your.server.com/api/data";
        http.begin(serverUrl);                              // Инициализация клиента :contentReference[oaicite:3]{index=3}
        http.addHeader("Content-Type", "application/json"); // Установка заголовков :contentReference[oaicite:4]{index=4}

        // Тело запроса в формате JSON
        String payload = "{\"temperature\":23.5,\"humidity\":60}";
        int httpCode = http.POST(payload); // Отправка POST :contentReference[oaicite:5]{index=5}

        if (httpCode > 0)
        {
            Serial.printf("POST code: %d\n", httpCode);
            String response = http.getString();
            Serial.println(response);
        }
        else
        {
            Serial.printf("POST failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end(); // Освобождение ресурсов
    }
    delay(5000);
}