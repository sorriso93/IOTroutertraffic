/*
  IOTRouter Traffic by sorriso93 alias MAX v04
  ____________________________________________
  Copyright (c) 2019 by Massimiliano Orso
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, and to permit persons to whom the 
  Software is furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
// v02 ++ web call to switch on/off display for night - useful to be used via homebridge-homekit
// v03 ++ influxdb data stream
// v04 ++ reboot every x hours to clean heap and stack

//
#include <FS.h>                   //WIFIMANAGER this needs to be first, or it all crashes and burns...
extern "C" {                      // needed to set hostname
#include "user_interface.h"
}

#include <ArduinoJson.h>
#include "preferences.h"
#include <ESP8266WiFi.h>
#include <ESP8266Ping.h>
#include <WiFiClient.h> 
#include <ESP8266HTTPClient.h>
#include "wifiOTA.h"

ESP8266WebServer wserver(80); //webserver to switch off/on display via homebridge-homekit
boolean displayOn = true;
String output_http ="";

double stringToLong(String s)
{
   char arr[20];
   s.toCharArray(arr, sizeof(arr));
   return atoll(arr)/1024;
}

double stringToDouble(String & str)
{
  return atof( str.c_str() );
}

double call_upnp (String _action)
{
   HTTPClient http;    //Declare object of class HTTPClient

   //http.begin("http://192.168.0.1:49152/upnp/control/WANCommonIFC1");      //Specify request destination
   http.begin(_ip+":"+_port+_path);      //Specify request destinatio
   http.addHeader("cache-control", "no-cache");
   //http.addHeader("soapaction", " \"urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1#GetTotalbytesReceived\"");
   http.addHeader("soapaction", " "+(_service.substring(0,_service.length()-1))+"#"+_action+"\"");
   http.addHeader("Content-Type","text/xml; charset=\"utf-8\"");  //Specify content-type header
   http.addHeader("connection","close");
   String body_tmp = "<?xml version=\"1.0\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:"+_action+" xmlns:u="+_service+"></u:"+_action+"></s:Body>\n</s:Envelope>";
   int httpCode = http.POST(body_tmp);   //Send the request
   String payload = http.getString();    //Get the response payload 

   if (_action == gettotalbytesreceived)
   {
     //DEBUG_PRINTLN(payload);
     payload = payload.substring(payload.indexOf(bytesreceivedresult)+bytesreceivedresult.length()+1,payload.indexOf("</"+bytesreceivedresult));
     //DEBUG_PRINTLN("tot bytes received"+payload);
     //return stringToLong(payload);
     return stringToDouble(payload);
   }
   if (_action == gettotalbytessent)
   {
     //DEBUG_PRINTLN(+payload);
     payload = payload.substring(payload.indexOf(bytessentresult)+bytessentresult.length()+1,payload.indexOf("</"+bytessentresult));    
     //DEBUG_PRINTLN("tot bytes sent"+payload);
     return stringToDouble(payload);
   }
   //gettotalpacketsreceived
   if (_action == gettotalpacketsreceived)
   {
     payload = payload.substring(payload.indexOf(packetreceivedresult)+packetreceivedresult.length()+1,payload.indexOf("</"+packetreceivedresult));
     //DEBUG_PRINTLN("tot packets received"+payload);
     return stringToDouble(payload);
   }
   //gettotalpacketssent
   if (_action == gettotalpacketssent)
   {
     payload = payload.substring(payload.indexOf(packetsentresult)+packetsentresult.length()+1,payload.indexOf("</"+packetsentresult));
     //DEBUG_PRINTLN("tot packets sent" + payload);
     return stringToDouble(payload);
   }
   
   //getcommonlinkproperties
   /*<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/" s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><s:Body>
      <u:GetCommonLinkPropertiesResponse xmlns:u="urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1">
      <NewWANAccessType>Cable</NewWANAccessType>
      <NewLayer1UpstreamMaxBitRate>512000</NewLayer1UpstreamMaxBitRate>
      <NewLayer1DownstreamMaxBitRate>512000</NewLayer1DownstreamMaxBitRate>
      <NewPhysicalLinkStatus>Up</NewPhysicalLinkStatus>
      </u:GetCommonLinkPropertiesResponse>
      </s:Body> </s:Envelope>
   */
   http.end();  //Close connection
}

void enableDisplay(boolean enable) {
  if (enable) 
  {
    pinMode(D8, OUTPUT);    //PWM on A0 for backlight
    digitalWrite(D8,1);
  } 
  else 
  {
    pinMode(D8, OUTPUT);    //PWM on A0 for backlight
    digitalWrite(D8,0);
  }
}

void handleState()
{
  output_http ="0";
  if (displayOn) 
  {
    output_http ="1";
  } 
  wserver.send(200, "text/plain", output_http);
  DEBUG_PRINTLN("screen status");
}

void handleScreenOn()
{
  enableDisplay(true);
  output_http ="1";
  displayOn = true;
  wserver.send(200, "text/plain", output_http);
  DEBUG_PRINTLN("screen on");
}

void handleScreenOff()
{
  enableDisplay(false);
  output_http ="0";
  displayOn = false;
  wserver.send(200, "text/plain", output_http);
  DEBUG_PRINTLN("screen off");
}

void redirectHome() 
{
  // Send them back to the Root Directory
  wserver.sendHeader("Location", String("/"), true);
  wserver.sendHeader("Cache-Control", "no-cache, no-store");
  wserver.sendHeader("Pragma", "no-cache");
  wserver.sendHeader("Expires", "-1");
  wserver.send(302, "text/plain", "");
  wserver.client().stop();
  delay(1000);
}

//------------------- SETUP
void setup() 
{
  Serial.begin(115200);
  setup_display(); // DISPLAY
  cancella_display(); 
  accendi_backlight();
  scrivi_display_riga_colore ("  CONNECT TO WIFI\r\n"+String(name_sensor_default),32, TFT_BLACK);
  
  WiFiManager wifiManager;
  wifi_station_set_hostname (host_name);
  WiFi.hostname(host_name);
  wifiManager.autoConnect(name_sensor_default);

  #ifdef MQTT_Y //setup for MQTT
   //DEBUG_PRINTLN ("SETUP MQTT");
   client.setServer(mqtt_server, 1883);
   //#ifndef publisher_MQTT
   client.setCallback(callback);
  #endif //per MQTT
  #ifdef influx //influx db connection
   db_influx.setDbAuth("RTStat", INFLUXDB_USER, INFLUXDB_PASS);
  #endif
  
  setup_OTA_MAX();
  cancella_display();
  scrivi_display_riga_colore ("  IOT Traffic\r\n\r\n                by MAX",32,TFT_YELLOW);
  delay(2000);
  cancella_display();

  // Start the NTP UDP client
  pm_ntp = 0;
  timeClient.begin();   
  utc = now();
  if (timeClient.update())
  {
    unsigned long epochTime =  timeClient.getEpochTime();
    // convert received time stamp to time_t object
    utc = epochTime;
    local = Italy.toLocal(utc); // Then convert the UTC UNIX timestamp to local time
  }
  else
  {
    //No time via NTP I reset
    cancella_display();
    DEBUG_PRINTLN("No time, reset");
    scrivi_display_riga_colore (" NO\r\n NETWORK\r\n TIME",16, TFT_GREEN);
    delay(2000);
    ESP.reset(); // resetto se non riceve l'orario da server NTP
    delay(1000);
  }

  randomSeed(analogRead(0));
  tft.setCursor(0, 0);

  //webserver config
  wserver.on("/screen_status", handleState);
  wserver.on("/screen_on", handleScreenOn); 
  wserver.on("/screen_off", handleScreenOff); 
  wserver.onNotFound(redirectHome);
  // Start the server
  wserver.begin();
}

//----------------- LOOP

void loop() 
{
   String temp_str;
   double bytes_r1, bytes_s1, bytes_r2, bytes_s2; 
//   long int kbytes_r_xfreq, kbytes_s_xfreq;
//   int avg_time_ms = 0;
   int readings = 0;
   
   ArduinoOTA.handle(); // for OTA
   wserver.handleClient(); // for webserver
   delay(1);

   // NTP clock update
   cm_ntp = millis(); 
   if ((unsigned long) cm_ntp-pm_ntp >= r_ntp)
   {
      utc = now();
      timeClient.update();
      unsigned long epochTime =  timeClient.getEpochTime();
      // convert received time stamp to time_t object
      utc = epochTime;
      local = Italy.toLocal(utc); // Then convert the UTC UNIX timestamp to local time
      pm_ntp = cm_ntp;
   }

  // restart every 12 hour to clear heap
  if (((hour(local) % 12) == 0)&&(minute(local) == 0)&&((second(local)>0)&&(second(local)<=10)))  
  {
    DEBUG_PRINTLN("RESETTO OGNI DUE ORE");
    ESP.reset();
    delay(1000);
  }
   
   #ifdef publisher
     //collects data from IG router
     if(WiFi.status()== WL_CONNECTED)
     {   //Check WiFi connection status
        bytes_r1 = call_upnp(gettotalbytesreceived);
        delay(2);
        bytes_s1 = call_upnp(gettotalbytessent);
        delay(freq_request-4);
        bytes_r2 = call_upnp(gettotalbytesreceived);
        delay(2);
        bytes_s2 = call_upnp(gettotalbytessent);
        //temp_str = call_upnp(getcommonlinkproperties);
        //a new request after freq_request second to obtain bytes per second
     }
     else
     {
        Serial.println("Error in WiFi connection");   
     }
     // converts in kbytes per second data collected
     kbytes_r_xfreq = (long int) abs((bytes_r2 - bytes_r1)/1024);
     kbytes_s_xfreq = (long int) abs((bytes_s2 - bytes_s1)/1024);
     if (kbytes_r_xfreq>100000) //sometimes my router gives back random numbers... I limit it to max theoretical download
      kbytes_r_xfreq = 100000;
     //ping check
     Ping.ping(ping_site);
     avg_time_ms = Ping.averageTime();
     #ifdef MQTT_Y
      //Create JSON data to be sent to MQTT queue
      /*stampa_MQTT[1] = "Kb-received: " + String(kbytes_r_xfreq); //received
      stampa_MQTT[2] = "Kb-sent: "+ String(kbytes_s_xfreq); //sent */
      stampa_MQTT["sensor"] = "router";
      stampa_MQTT["Kbsec_rec"] = kbytes_r_xfreq;
      stampa_MQTT["Kbsec_sent"] = kbytes_s_xfreq;
      stampa_MQTT["ping"] = avg_time_ms;
      //Calls update routine to publish MQTT data on queue
      MQTT_update();
     #endif
     #ifdef influx
      InfluxData row("traffic");
      //row.addTag("mode", "pwm");
      row.addValue("Kbsec_rec", kbytes_r_xfreq);
      row.addValue("Kbsec_sent", kbytes_s_xfreq);
      row.addValue("ping", avg_time_ms);
      db_influx.write(row);
     #endif
     // max & min update
     if (max_speed_r < kbytes_r_xfreq)
       max_speed_r = kbytes_r_xfreq;
     if (max_speed_s < kbytes_s_xfreq) 
       max_speed_s = kbytes_s_xfreq;
     disegna (kbytes_r_xfreq, kbytes_s_xfreq, avg_time_ms); //kbytes per second on lcd screen
   #else
    //DEBUG_PRINTLN("subscriber MQTT");
    if (!client.connected())
    {
     reconnect(); // connects to MQTT queue
    }
    client.loop();
   #endif

   //scrivi_ora();
   //DEBUG_PRINTLN("kbytes received per "+ String(freq_request) + " millisecond >>> " + String(kbytes_r_xfreq));
   //DEBUG_PRINTLN("kbytes sent per "+ String(freq_request) + " millisecond >>> " + String(kbytes_s_xfreq));
   //DEBUG_PRINTLN("------ Ping "+String(ping_site)+": "+String(avg_time_ms));
}
