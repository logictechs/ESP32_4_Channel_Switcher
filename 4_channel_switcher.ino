// ESP32 Dev Module COM16

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <ArduinoOTA.h>
#include <ModbusIP_ESP8266.h>

const char* ssid = "your SSID here";
const char* password = "your password here";

const char apid[] = "PowerController";  //  your network SSID (name)
const char appass[] = "your password here";       // your network password

IPAddress apIP(192, 168, 4, 1);

ModbusIP mb;

const int REG = 100;               // Modbus Hreg Offset

unsigned int outputprog;
unsigned int lastprogress;

int wificounter = 0;

#define RELAY_PIN_1 21
#define RELAY_PIN_2 19
#define RELAY_PIN_3 18
#define RELAY_PIN_4 5
#define LED_PIN     25


void setup()
{
    Serial.begin(115200);
    delay(10);
    Serial.println("booting...");

    pinMode(RELAY_PIN_1, OUTPUT);
    pinMode(RELAY_PIN_2, OUTPUT);
    pinMode(RELAY_PIN_3, OUTPUT);
    pinMode(RELAY_PIN_4, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    delay(100);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.waitForConnectResult() != WL_CONNECTED && wificounter != 2) 
  {    
    delay(5000);
    ++wificounter;
  }

  if(WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Configuring access point...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apid, appass,5);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
  }
  else
  {
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()      
      lastprogress = 0;
    })
    .onEnd([]() {
      Serial.println("\nEnd");      
    })
    .onProgress([](unsigned int progress, unsigned int total) {      
      outputprog = progress / (total / 100);
      if (lastprogress != outputprog)
      {                        
        lastprogress = outputprog;
      }
    })
    .onError([](ota_error_t error) {      
      
    });

  ArduinoOTA.begin();
  
  mb.server();  
  mb.addCoil(100, 0, 4);

}

void loop()
{
  ArduinoOTA.handle();  
  
  if (lastprogress == 0 && WiFi.status() == WL_CONNECTED)
  {
    mb.task();
    digitalWrite(RELAY_PIN_1, mb.Coil(REG));
    digitalWrite(RELAY_PIN_2, mb.Coil(REG + 1));
    digitalWrite(RELAY_PIN_3, mb.Coil(REG + 2));
    digitalWrite(RELAY_PIN_4, mb.Coil(REG + 3));
    delay(10);
  }

}
