// C:\Users\Acer\Documents\PlatformIO\Projects\qr_DataMatrix\.pio\build\wt32-eth01

#include "sets.h"

#include <LittleFS.h>
#include <WiFiConnector.h>

// LED indikator(INDIKATOR, 300, 3, 50, 20);  // каждые 1000 милисек мигаем 3

GyverDBFile db(&LittleFS, "/data.db");
// SettingsGyver sett(PROJECT_NAME, &db);
SettingsGyverWS sett(PROJECT_NAME, &db);
static bool notice_f;  // флаг на отправку уведомления о подключении к wifi
bool gotWifi = false;  // если произошел момент подключения к сети
uint32_t checkWifiMs = 0, curSecMs = 0;

Datime curDataTime(NTP);  // NTP это объект

// для чтения qr кодов
String dm_string = "";
bool newScan = false;
// для отправки на сервер qr кодов
String srvResponse = ""; // ответ от сервера на отправку данных по ethernet сканера 
int httpCode = 0;  // код ответа от сервера на отправку данных сканера  
bool newPass = false;  // флаг о том что можно вывести в логгер данные нового скана

static const char *const WEEKdays[] = {
    "вчера",
    "Понедельник",
    "Вторник",
    "Среда",
    "Четверг",
    "Пятница",
    "Суббота",
    "Воскресенье"};
// это апдейтер. Функция вызывается, когда вебморда запрашивает обновления

Data data;

sets::Logger logger(150);

// ========== build ==========
// WEB интерфейс ВЕБ морда формируется здесь
static void build(sets::Builder &b) {
    // можно узнать, было ли действие по виджету
    if (b.build.isAction()) {
        Serial.print("Set: 0x");
        Serial.print(b.build.id, HEX);
        Serial.print(" = ");
        Serial.println(b.build.value);

        logger.print("Set: 0x");
        logger.println(b.build.id, HEX);
    }
    // логирование скана qr кода
    if (newScan) {
        logger.print("dm_string: ");
        logger.println(dm_string);
        newScan = false;
    }
    if (newPass) {
        logger.print("srvResponse: ");
        logger.println(srvResponse);
        logger.print("httpCode ");
        logger.println(httpCode);
        newPass = false;
    }
    // морда
    {
        sets::Group g(b, "Nicelight");
        if (NTP.online()) {
            {
                sets::Row g(b);
                // b.DateTime(kk::datime, "Сегодня ");
                b.Label(kk::dayofweek, "Сегодня");  // текущая дата
                b.Label(kk::datime, " ");           // текущая дата
            }
        }  // NTP.online()

        // {
        //     sets::Row g(b);
        //     // sets::Row g(b, "Row");
        //     b.Label(kk::uptimeDays, "Аптайм");
        //     b.Time(kk::secondsUptime, " ");
        // }
        {
            sets::GuestAccess g(b);
            b.Time(kk::secondsNow, "Времечко");
        }
        // логгер, в него печатаем выше
        b.Log(logger);
    }  // Nicelight

    // {  // удалить если работает wifi. это было в исходном примере ard_pio
    //     sets::Group g(b, "Настройки WiFi");
    //     b.Input(kk::wifi_ssid, "WiFI сеть");
    //     b.Pass(kk::wifi_pass, "пароль", "");
    //     if (b.Switch(kk::close_ap, "закрывать точку доступа")) {
    //         WiFiConnector.closeAP(db[kk::close_ap]);
    //     }
    //     if (b.Button("Подключить")) {
    //         db.update();      //  в примере WiFiconnector было, а в примере ard_pio не было
    //         notice_f = true;  // пользователю попап уведомление
    //         WiFiConnector.connect(db[kk::wifi_ssid], db[kk::wifi_pass]);
    //     }
    // }

    /* Настройки , внизу страницы*/
    {
        sets::Group g(b, " ");
        {
            sets::Menu m1(b, "Опции");
            {
                sets::Menu m2(b, "Интерфейс");
                b.Label("если надо - добавим тут");
            }  // настройки - интерфейс
            {
                sets::Menu m3(b, "Wifi");
                // провалились в расширенные пристройки {
                sets::Group m4(b, "Настройки WiFi");
                b.Input(kk::wifi_ssid, "Wifi сеть");
                b.Pass(kk::wifi_pass, "Пароль", "");
                if (b.Switch(kk::close_ap, "Закрывать точку доступа")) {
                    WiFiConnector.closeAP(db[kk::close_ap]);
                }
                if (b.Button("Подключить")) {
                    db.update();      //  в примере WiFiconnector было, а в примере ard_pio не было
                    notice_f = true;  // пользователю попап уведомление
                    WiFiConnector.connect(db[kk::wifi_ssid], db[kk::wifi_pass]);
                }
            }  // настройки wifi
            b.Input(kk::ntp_gmt, "Часовой пояс");
            b.Label(" ", "\n\n ");

            if (b.Button(kk::btn2, "стереть настройки(TODO!)", sets::Colors::Red)) {
                Serial.println("could clear db");
                // db.clear();
                // db.update();
            }
        }  // настройки - расширенные
    }  // Подстройки
    b.Label(H(testLabel), "тест:");
}  // build

/// TODO переделать на веб сокеты
// ========== update =========
// static void update(sets::Updater& u) {
//     if (notice_f)  // уведомление при вводе wifi данных
//     {
//         notice_f = false;
//         u.notice("Если не ошибся с вводом, устройство подключится к  wifi сети, светодиод медленно замигает");
//         //    upd.alert("Ошибка");
//     }
//     u.update(kk::secondsNow, data.secondsNow);
//     u.update(kk::secondsUptime, data.secondsUptime);
//     u.update(kk::datime, NTP.dateToString());
//     u.update(kk::dayofweek, (String)(WEEKdays[curDataTime.weekDay]));
//     if (!data.uptime_Days)  // апдейтим аптайм
//         u.update(kk::uptimeDays, (String)("0 дней"));
//     else if (data.uptime_Days == 1)
//         u.update(kk::uptimeDays, (String)("1 день"));
//     else if (data.uptime_Days < 5)
//         u.update(kk::uptimeDays, (String)(data.uptime_Days + String(" дня")));
//     else if (data.uptime_Days >= 5)
//         u.update(kk::uptimeDays, (String)(data.uptime_Days + String(" дней")));

// }  // update

// ========== begin ==========
void sett_begin() {
    // fs
#ifdef ESP32
    LittleFS.begin(true);
#else
    LittleFS.begin();
#endif

    // database
    db.begin();
    db.init(kk::wifi_ssid, "");
    db.init(kk::wifi_pass, "");
    db.init(kk::close_ap, true);
    db.init(kk::ntp_gmt, 5);

    // wifi
    WiFiConnector.setName(AP_SSID); // имя точки доступа
    WiFiConnector.setPass(AP_PASS); // 30 сек таймаут на подключение к wifi
    WiFiConnector.setTimeout(180); // 30 сек таймаут на подключение к wifi
    WiFiConnector.onConnect([]() {
        Serial.print("Connected: ");
        Serial.println(WiFi.localIP());
        // indikator.setPeriod(3000, 1, 200, 150);  // раз в 3000 сек, 1 раз
    });
    WiFiConnector.onError([]() {
        Serial.print("Error. Start AP: ");
        Serial.println(WiFi.softAPIP());
        // indikator.setPeriod(600, 2, 100, 50);  // раз в  секунду два раза взмигнем - по 200 милисек, гореть будем 50 милисек
        // if (each5min.ready()) ESP.restart();  // через 5 минут ребутаемся
        NTP.begin();
        NTP.setHost("1.asia.pool.ntp.oronUpdateg");     // установить другой хост
        // NTP.setHost("uz.pool.ntp.org");     // установить другой хост
        // NTP.setHost("2.asia.pool.ntp.org");     // установить другой хост
        NTP.setPeriod(600);  // обновлять раз в 600 сек        
        NTP.setGMT(db[kk::ntp_gmt]);
        NTP.updateNow();  // синхронизировать
        NTP.tick(); });

    WiFiConnector.setName(PROJECT_NAME);
    WiFiConnector.closeAP(db[kk::close_ap]);
    WiFiConnector.connect(db[kk::wifi_ssid], db[kk::wifi_pass]);

    // settings
    sett.setPass(F("123456789000"));  // без пароля ничего не видно в интерфейсе
    sett.begin();
    sett.onBuild(build);
    // sett.onUpdate(update); // для webSocket не нужно
}

// ========== loop ==========
void sett_loop() {
    WiFiConnector.tick();
    // проверка связи с вайфаем
    if (WiFiConnector.connected()) {
        if (!gotWifi) {
            // indikator.setPeriod(3000, 1, 200, 150); // спокойное мигание после реконнекта к wifi
        }
        gotWifi = true;
        checkWifiMs = millis();
    } else {
        if (gotWifi) {
            gotWifi = false;  // для запуска частой мигалки
            // общее время, кол-во, период одного, один зажжен на.
            // indikator.setPeriod(1000, 10, 100, 70); // часто мигаем
        }
        if ((millis() - checkWifiMs) > 300000ul) {  // через 5 минут оффлайна
            for (int i = 0; i < 20; i++) {
                // digitalWrite(INDIKATOR, 1);
                // delay(70);
                // digitalWrite(INDIKATOR, 0);
                // delay(70);
            }
            ESP.restart();
        }
    }  // WiFi.connected()

    sett.tick();

    // обновление в веб морде
    static uint32_t tmr;
    if (millis() - tmr >= 300) {
        tmr = millis();
        // отправить апдейт прямо сейчас
        sett.updater().update(H(testLabel), random(100));
    }  // if ms

    // indikator.tick();
    NTP.tick();

    // тикаем временем каждую секунду,
    //  TODO переделать чтобы NTP нормально секунды отдавало... не знаю как
    if (millis() - curSecMs >= 1000) {  // раз в 1 сек
        curSecMs = millis();
        if (NTP.online()) {
            data.secondsNow = NTP.daySeconds();
            curDataTime = NTP.getUnix();
        } else
            data.secondsNow++;  // инкермент реалтайм

        data.secondsUptime++;               // инкермент аптайм
        if (data.secondsUptime == 86399) {  // инкремент дней аптайма
            data.secondsUptime = 0;
            data.uptime_Days++;
        }
    }  // each ms
}