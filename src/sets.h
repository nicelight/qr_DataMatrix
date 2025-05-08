#pragma once
#include "nastroyki.h"
#include "led.h"
#include "timer.h"

// #define PROJECT_NAME "Turniket-v0.1"
// #define PROJECT_NAME "Турникеты v0.1"
#define PROJECT_NAME "ESP32 Settings"

#include <GyverDBFile.h>
#include <LittleFS.h>
// #include <SettingsGyver.h>
#include <SettingsGyverWS.h>
#include <GyverNTP.h>

extern GyverDBFile db; // без веб сокетов
extern SettingsGyverWS sett;
// extern SettingsGyver sett;

extern Datime curDataTime;
extern LED indikator; // если так можно
extern String dm_string; // что просканировал сканер, выведем в лог
extern bool newScan; // флаг о том что можно вывести в лог данные нового скана 
extern bool newPass; // флаг о том что можно вывести в лог данные новой отправки 
extern String srvResponse; // ответ от сервера на отправку данных сканера
extern int httpCode;
void sett_begin();
void sett_loop();

DB_KEYS(
    kk,
    datime,
    dayofweek,
    uptimeDays,
    secondsNow,
    secondsUptime,
    ntp_gmt,
    btn2,


    wifi_ssid,
    wifi_pass,
    close_ap
);


struct Data  // обьявляем класс структуры
{
    uint32_t secondsNow = 44000ul;
    uint32_t secondsUptime = 1;
    byte uptime_Days = 0;

};

extern Data data;