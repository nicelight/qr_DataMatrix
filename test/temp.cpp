#if !( defined(ESP32) )
  #error This code is intended to run on the ESP32 platform! Please check your Tools->Board setting.
#endif



#include <WebServer_WT32_ETH01.h>               // https://github.com/khoih-prog/WebServer_WT32_ETH01

#define ASYNC_HTTP_REQUEST_ESP32_ETHERNET_VERSION_MIN_TARGET      "AsyncHTTPRequest_ESP32_Ethernet v1.14.0"
#define ASYNC_HTTP_REQUEST_ESP32_ETHERNET_VERSION_MIN             1014000

// Uncomment for certain HTTP site to optimize
//#define NOT_SEND_HEADER_AFTER_CONNECTED        true

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include <AsyncHTTPRequest_ESP32_Ethernet.h>             // https://github.com/khoih-prog/AsyncHTTPRequest_ESP32_Ethernet

#include <Ticker.h>

AsyncHTTPRequest request;
Ticker ticker;
Ticker ticker1;

/////////////////////////////////////////////

// Select the IP address according to your local network
IPAddress myIP(192, 168, 2, 232);
IPAddress myGW(192, 168, 2, 1);
IPAddress mySN(255, 255, 255, 0);

// Google DNS Server IP
IPAddress myDNS(8, 8, 8, 8);

/////////////////////////////////////////////

void heartBeatPrint(void)
{
  static int num = 1;

  if (WT32_ETH01_isConnected())
    Serial.print(F("H"));        // H means connected
  else
    Serial.print(F("F"));        // F means not connected

  if (num == 80)
  {
    Serial.println();
    num = 1;
  }
  else if (num++ % 10 == 0)
  {
    Serial.print(F(" "));
  }
}

void sendRequest()
{
  static bool requestOpenResult;

  if (request.readyState() == readyStateUnsent || request.readyState() == readyStateDone)
  {
    //requestOpenResult = request.open("GET", "http://worldtimeapi.org/api/timezone/Europe/London.txt");
    requestOpenResult = request.open("GET", "http://worldtimeapi.org/api/timezone/America/Toronto.txt");

    if (requestOpenResult)
    {
      // Only send() if open() returns true, or crash
      request.send();
    }
    else
    {
      Serial.println("Can't send bad request");
    }
  }
  else
  {
    Serial.println("Can't send request");
  }
}

void requestCB(void *optParm, AsyncHTTPRequest *request, int readyState)
{
  (void) optParm;

  if (readyState == readyStateDone)
  {
    AHTTP_LOGDEBUG(F("\n**************************************"));
    AHTTP_LOGDEBUG1(F("Response Code = "), request->responseHTTPString());

    if (request->responseHTTPcode() == 200)
    {
      Serial.println(F("\n**************************************"));
      Serial.println(request->responseText());
      Serial.println(F("**************************************"));
    }
  }
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  while (!Serial && millis() < 5000);

  delay(200);

  Serial.print("\nStart AsyncHTTPRequest_WT32_ETH01 on ");
  Serial.print(ARDUINO_BOARD);
  Serial.print(" with ");
  Serial.println(SHIELD_TYPE);
  Serial.println(WEBSERVER_WT32_ETH01_VERSION);
  Serial.println(ASYNC_HTTP_REQUEST_ESP32_ETHERNET_VERSION);

  Serial.setDebugOutput(true);

#if defined(ASYNC_HTTP_REQUEST_GENERIC_VERSION_MIN)

  if (ASYNC_HTTP_REQUEST_GENERIC_VERSION_INT < ASYNC_HTTP_REQUEST_GENERIC_VERSION_MIN)
  {
    Serial.print("Warning. Must use this example on Version equal or later than : ");
    Serial.println(ASYNC_HTTP_REQUEST_GENERIC_VERSION_MIN_TARGET);
  }

#endif

  // To be called before ETH.begin()
  WT32_ETH01_onEvent();

  //bool begin(uint8_t phy_addr=ETH_PHY_ADDR, int power=ETH_PHY_POWER, int mdc=ETH_PHY_MDC, int mdio=ETH_PHY_MDIO,
  //           eth_phy_type_t type=ETH_PHY_TYPE, eth_clock_mode_t clk_mode=ETH_CLK_MODE);
  //ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_TYPE, ETH_CLK_MODE);
  ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER);

  // Static IP, leave without this line to get IP via DHCP
  //bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1 = 0, IPAddress dns2 = 0);
  ETH.config(myIP, myGW, mySN, myDNS);

  WT32_ETH01_waitForConnect();

  Serial.print(F("\nHTTP WebClient is @ IP : "));
  Serial.println(ETH.localIP());

  request.setDebug(false);

  request.onReadyStateChange(requestCB);
  ticker.attach(HTTP_REQUEST_INTERVAL, sendRequest);

  ticker1.attach(HEARTBEAT_INTERVAL, heartBeatPrint);

  // Send first request now
  sendRequest();
}

void loop()
{
}