// статический айпишник ETH в сети
#define IP_OCT1 192
#define IP_OCT2 168
#define IP_OCT3 10
// #define IP_OCT3 50
#define IP_OCT4 232
#define IP_GATE 1

// IP адрес целевого сервера (пример)
#define SERVERURL "http://192.168.10.253/api/callback" // тестовый дом
// #define SERVERURL "http://192.168.50.201/api/callback" // прод 

// дальномер на I2C шине 
#define LASER_DALNOMER 0    // 1 - включен, 0 - выключен
// SDA белый 17 (TXD на плате) 
// SCL синий 5 (RXD на плате)


#define ENTER_PIN 4
#define EXIT_PIN 2

#define OPEN_DELAY 4000ul

#define ON 0  // включение ворот логическим нулем или единицей
#define OFF 1 //



#define RX_SCANER 14 // RX1
#define TX_SCANER 12 // TX1 не нужно



// имя пароль вашей домашней сети
// можно ввести, подключившись к ESP AP c паролем 1234567890
#define AP_SSID "ESP32 232" // имя точки доступа
#define AP_PASS "123456789000" // имя точки доступа
#define WIFI ""
#define WIFIPASS ""

// #define DEBUG

// #define INDIKATOR 2 // на каком пине индикаторный светодиод
#define BTN 0       // встроенная кнопка

#define SERVICE_QR "01A1B2C3D4E5F6170102521123456780" // сервисный QR код