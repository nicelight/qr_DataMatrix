// статический айпишник ETH в сети
#define IP_OCT1 192
#define IP_OCT2 168
// #define IP_OCT3 10
#define IP_OCT3 50
#define IP_OCT4 231
#define IP_GATE 1

// IP адрес целевого сервера (пример)
// тестовый дома
// #define SERVERURL "http://192.168.10.250/api/callback"
#define SERVERURL "http://192.168.50.210/api/callback"
// #define SERVER_OCT1 192
// #define SERVER_OCT2 168
// // #define SERVER_OCT3 50
// #define SERVER_OCT3 10
// #define SERVER_OCT4 50

#define FIRST_ENTER_DELAY 4000
#define FIRST_EXIT_DELAY 4000
#define MULTIPLE_OPEN_DELAY 1000
#define ENTER_PIN 4
#define EXIT_PIN 2



#define ON 0  // включение ворот логическим нулем или единицей
#define OFF 1 //

#define RX_SCANER 14 // RX1
#define TX_SCANER 12 // TX1


// дальномер на I2C шине 
// SDA белый 17 (TXD на плате) 
// SCL синий 16 (RXD на плате)



// имя пароль вашей домашней сети
// можно ввести, подключившись к ESP AP c паролем 1234567890
#define WIFI ""
#define WIFIPASS ""
// #define DEBUG

// #define INDIKATOR 2 // на каком пине индикаторный светодиод
#define BTN 0       // встроенная кнопка
