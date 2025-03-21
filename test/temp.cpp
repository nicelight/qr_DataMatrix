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


#include <AsyncTCP.h>
#include <AsyncWebServer_WT32_ETH01.h>

#define DATABASE_SIZE 1000
long database[DATABASE_SIZE];
int currentIndex = 0;

AsyncWebServer* ETHserver;

const char* PARAM_MESSAGE = "message";

void handleEthRoot(AsyncWebServerRequest* request) {
    request->send(200, F("text/html"), "root page from separet func");
}
void handleEthnotFound(AsyncWebServerRequest* request) {
    request->send(404, "text/plain", "\n\n \t\t @smartfarm_diy 404");
}

void handleeEthPost(AsyncWebServerRequest* request) {
    String postMsg;
    if (request->hasParam(PARAM_MESSAGE, true)) {
        postMsg = request->getParam(PARAM_MESSAGE, true)->value();
    } else {
        postMsg = "No message sent";
    }
    Serial.println(postMsg);
    request->send(200, "text/plain", "\tHello, POST: \n\n" + postMsg);
}  // handleeEthPost


void setup() {
    Serial.begin(115200);
    Serial.println();

    Serial.print("\n\t\t\t INIT \n\t wifi ssid: ");
    Serial.println(db[kk::wifi_ssid]);  // из settings.h доступны db и ключи
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

    ETHserver = new AsyncWebServer(80);
    ETHserver->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        handleEthRoot(request);
    });
    ETHserver->on("/post", HTTP_POST, [](AsyncWebServerRequest* request) {
        handleeEthPost(request);
    });

    ETHserver->onNotFound([](AsyncWebServerRequest* request) {
        handleEthnotFound(request);
    });
    ETHserver->begin();

}  // setup()

void loop() {
}