// статический айпишник ETH в сети
#define IP_OCT1 192
#define IP_OCT2 168
#define IP_OCT3 11
// #define IP_OCT3 50
#define IP_OCT4 232
#define IP_GATE 1

// IP адрес целевого сервера (пример)
#define SERVERURL "http://192.168.11.200/api/callback" // тестовый дом
// #define SERVERURL "http://192.168.50.201/api/callback" // прод 

// дальномер на I2C шине 
#define LASER_DALNOMER 1    // 1 - включен, 0 - выключен
// выбрать один из двух DALNOMER_TYPE :
// #define   VL53L0X // этот заказан первым, стоит на первом тестовом турникете
#define VL53LXX_V2 // эти были заказаны позже 
// SDA белый 17 (TXD на плате) 
// SCL синий 5 (RXD на плате)r

// Kompas 231(ENTER) 
#define ENTER_PIN 2
#define EXIT_PIN 4

// // Kompas 232(EXIT), 233(EXIT)
// #define ENTER_PIN 4
// #define EXIT_PIN 2

#define OPENINIG_DELAY 4000ul // задержка если датчика нет


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