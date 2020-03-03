// v.04
// api router tiscali HUAWEI
// http://192.168.0.1:49152/gatedesc.xml
// http://192.168.0.1:49152/gateicfgSCPD.xml

//#define debug_serial //comment out if not debugging
#define name_sensor_default "RTrafficPSALA" // BOARD NAME -- HOSTNAME
#define MQTT_Y //use MQTT (publisher or subscriber), comment it out if no MQTT is used
#define publisher // publisher or subscriber of MQTT, comment it out if no MQTT is used or if you want it as subscriber
#define rotazione 1 //1 usb connector on the left, 3 usb connector on the right
#define influx //se attivo invia dati a influxdb

//BOARD NAME AND WIF-OTA CONFIGURATION by default
#define password_AP "passwordota"

//Internet gateway router address and upnp call path
String _ip = "http://192.168.0.1";
String _port = "49152";
String _path = "/upnp/control/WANCommonIFC1";
#define ping_site "8.8.8.8"
#define max_readings 300 //5 minutes entry for hystorical data
int freq_request = 990; //UPNP request every 1000 msec approx
int max_download_rate = 7000; // max download rate of my internet connection
int max_upload_rate = 800; // max upload rate of my internet connection
#ifdef MQTT_Y 
  char mqtt_server[40] = "192.168.0.7"; // MQTT server ip address
  #define max_tentativi_MQTT 10 //10 times retry to connect to MQTT queue
  #define MQTT_QUEUE "router_stats" //  MQTT queue name
#endif
#ifdef influx
  #define INFLUXDB_HOST "192.168.0.7"
  #define INFLUXDB_USER "rtstat"
  #define INFLUXDB_PASS "maxsoft"
  #include <InfluxDb.h>
  Influxdb db_influx(INFLUXDB_HOST);
#endif
long int kbytes_r_xfreq, kbytes_s_xfreq = 0; //global variables with data to bee displayed
int avg_time_ms = 0; //global variables with data to bee displayed
unsigned long max_speed_r, max_speed_s = 0;

//---------- UPNP IDG configuration --- Internet gateway
//actions&services available on my router
String gettotalbytesreceived = "GetTotalBytesReceived";
String bytesreceivedresult = "NewTotalBytesReceived";
String gettotalbytessent = "GetTotalBytesSent";
String bytessentresult = "NewTotalBytesSent";
String gettotalpacketssent = "GetTotalPacketsSent";
String packetsentresult = "NewTotalPacketsSent";
String gettotalpacketsreceived = "GetTotalPacketsReceived";
String packetreceivedresult = "NewTotalPacketsReceived";
String getcommonlinkproperties = "GetCommonLinkProperties";
String linkpropertiesresult = "NewPhysicalLinkStatus";
String _service = "\"urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1\"";

// DAYLIGHT saving routine
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>
#include <ESP8266mDNS.h>
#include <WifiUDP.h>
time_t local, utc;
TimeChangeRule usEDT = {"CEST", Last, Sun, Mar, 2, 60};  //UTC - 5 hours - change this as needed
TimeChangeRule usEST = {"EST", Last, Sun, Oct, 3, 0};   //UTC - 6 hours - change this as needed
Timezone Italy(usEDT, usEST); // Timezone for Italy
unsigned long pm_ntp, cm_ntp; 
int r_ntp = 10000; // NTP time sync every 10 second
// Define NTP properties
#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "europe.pool.ntp.org"  // change this to whatever pool is closest (see ntp.org)
// Set up the NTP UDP client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);
const char * days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"} ;
const char * months[] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"} ;
const char * ampm[] = {"AM", "PM"} ;
String twoDigits(int digits);
//time_t compileTime(void);

//PINS MAP
#define D0 16
#define D1 5 // I2C Bus SCL (clock)
#define D2 4 // I2C Bus SDA (data)
#define D3 0
#define D4 2 // Same as "LED_BUILTIN", but inverted logic
#define D5 14 // SPI Bus SCK (clock)
#define D6 12 // SPI Bus MISO
#define D7 13 // SPI Bus MOSI
#define D8 15 // SPI Bus SS (CS)
#define D9 3 // RX0 (Serial console)
#define D10 1 // TX0 (Serial console)

#ifdef debug_serial
 #define DEBUG_PRINT(x) Serial.print(x)
 #define DEBUG_PRINTLN(x) Serial.println(x)
#endif
#ifndef debug_serial
 #define DEBUG_PRINT(x) 
 #define DEBUG_PRINTLN(x) 
#endif

// MQTT: useful if you want to have more than one traffic stat or collect data in other tools
#ifdef MQTT_Y 
  StaticJsonBuffer<1024> buffer_MQTT;
  JsonObject& stampa_MQTT = buffer_MQTT.createObject();
  
  #include <PubSubClient.h> // per MQTT
  WiFiClient espClient;
  PubSubClient client(espClient);
  int collegato_MQTT = 0; // 1 if it is connected to MQTT queue
  void callback(char* topic, byte* payload, unsigned int length);
  void MQTT_update (); //MQTT Publish
  void reconnect(); //MQTT reconnect
#endif

#include "display_mgm_espi.h"

#ifdef MQTT_Y 
  void callback(char* topic, byte* payload, unsigned int length) 
  {
   #ifndef publisher
    StaticJsonBuffer<1024> buffer2_MQTT;
    //JsonObject& stampa_MQTT = buffer2_MQTT.createObject();
    String messaggio;
    char msg[1024] = "";
    JsonVariant read_MQTT;
    
    int i;
    for (int i = 0; i < length; i++) 
    {
      messaggio += (char)payload[i];
    }
    DEBUG_PRINT("MQTT Message [");
    DEBUG_PRINT(messaggio);
    DEBUG_PRINTLN("] ");
    messaggio.toCharArray(msg,1024);
    read_MQTT = buffer2_MQTT.parseObject(msg); //only deserialization, assignement to variables to graph data is made in main code
    if (!read_MQTT.success())
    {
      DEBUG_PRINT("Json parsing error"); //additional things?
      return;
    }
    else
    {
      kbytes_r_xfreq = read_MQTT[String("Kbsec_rec")];
      kbytes_s_xfreq = read_MQTT[String("Kbsec_sent")];
      avg_time_ms = read_MQTT[String("ping")];
      DEBUG_PRINTLN("kbsec_r = "+String(kbytes_r_xfreq)+" kbsec_s = "+String(kbytes_s_xfreq)+" avg_ping = "+String(avg_time_ms));
    }
    // max & min update
    if (max_speed_r < kbytes_r_xfreq)
      max_speed_r = kbytes_r_xfreq;
    if (max_speed_s < kbytes_s_xfreq) 
      max_speed_s = kbytes_s_xfreq;
    disegna (kbytes_r_xfreq, kbytes_s_xfreq, avg_time_ms); //kbytes per second on lcd screen
    messaggio = "";
   #endif
  }
  
  void reconnect() 
  {
    // Loop until we're reconnected
    if (!client.connected())
    {
      DEBUG_PRINTLN("Attempting MQTT connection...");
      if (client.connect(name_sensor_default)) 
      {
        DEBUG_PRINTLN("MQTT Connected");
        // Once connected, publish an announcement...
        #ifdef publisher
         //client.publish(MQTT_QUEUE, (char*) name_sensor_default );
        #else
         client.subscribe(MQTT_QUEUE);
         DEBUG_PRINTLN("Subscribe: " + String(MQTT_QUEUE));
        #endif
        collegato_MQTT = 1;
      } 
      else 
      {
        DEBUG_PRINT("failed, rc=");
        DEBUG_PRINT(client.state());
        DEBUG_PRINTLN(" try again in 5 seconds");
        collegato_MQTT = 0;
      }
    }
  }
  
  #ifdef publisher   //--------------- routine to publish messages on MQ queue
    void MQTT_update ()
    {
      char output_mqtt[1024];
      if (!client.connected()) 
      {
       DEBUG_PRINTLN("MQTT UPDATE reconnect");
       reconnect();
      }
      stampa_MQTT.printTo(output_mqtt);
      client.loop();
      // pubblico messaggi su coda MQTT
      if (client.publish (MQTT_QUEUE, (char *) output_mqtt) == 1) //? &output_mqtt
        DEBUG_PRINTLN("MQTT Messagge sent "+ String(output_mqtt));
      delay(1);
     }
  #endif 
#endif
