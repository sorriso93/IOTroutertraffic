// v.00 first version
#include <ESP8266WiFi.h>
//needed for WIFIMANAGER
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

//needed for OTA
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>


const char* pwd_OTA = password_AP;

// --------------------------------------------------------
// SETUP ROUTINE FOR OTA MANAGEMENT
// --------------------------------------------------------

void setup_OTA_MAX()
{
  // Port defaults to 8266 -- ArduinoOTA.setPort(8266);
  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname(name_sensor_default);
  // No authentication by default
  ArduinoOTA.setPassword((const char *) pwd_OTA);

   ArduinoOTA.onStart([]() 
   {
     //DEBUG_PRINTLN("Start");
   });
   ArduinoOTA.onEnd([]() 
   {
     //DEBUG_PRINTLN("\nEnd");
   });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) 
  {
    DEBUG_PRINTLN("Progress: %u%%\r" + String(progress / (total / 100)));
   });
  ArduinoOTA.onError([](ota_error_t error) 
  {
     DEBUG_PRINTLN("Error[%u]: " + error);
     if (error == OTA_AUTH_ERROR) DEBUG_PRINTLN("Auth Failed");
     else if (error == OTA_BEGIN_ERROR) DEBUG_PRINTLN("Begin Failed");
     else if (error == OTA_CONNECT_ERROR) DEBUG_PRINTLN("Connect Failed");
     else if (error == OTA_RECEIVE_ERROR) DEBUG_PRINTLN("Receive Failed");
     else if (error == OTA_END_ERROR) DEBUG_PRINTLN("End Failed");
  });

  ArduinoOTA.begin();

  DEBUG_PRINTLN("Ready");
  DEBUG_PRINTLN("IP address: ");
  DEBUG_PRINTLN(WiFi.localIP());
  DEBUG_PRINTLN("Hostname: ");
  DEBUG_PRINTLN(WiFi.hostname());
  
  // ENDOTA
}
