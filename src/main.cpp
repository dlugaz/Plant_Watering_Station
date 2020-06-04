/*
 *  This sketch demonstrates how to scan WiFi networks.
 *  The API is almost the same as with the WiFi Shield library,
 *  the most obvious difference being the different file you need to include:
 */
#include "WiFi.h"
#include <Preferences.h>
#include <WebServer.h>
#include <Ticker.h>
#include <AsyncDelay.h>
#include <RunningAverage.h>
#include <ESPmDNS.h>
#include <NetBIOS.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

//My includes
#include "configuration.h"
#include "connect.h"
#include "site_handlers.h"
#include "watering_task.h"
#include "tasks_functions.h"
#include "io_definition.h"

const char AP_Name[] = "Plant_Watering_Station";
const char ntp_server[] = "pool.ntp.org";
const char hostname[] = "watering";

WiFiServer wifiServer;
MDNSResponder mDNSresponder;
NetBIOS netBIOSresponder;

void setup_Connectivity(Config& configuration)
{
    if(WiFi.isConnected()) return; 

    Status_LED_frequency_ms = 200;

    if(WiFi.setHostname(hostname))Serial.printf("Set hostname to %s \n",hostname);

    if(configuration.Wifi_Station_Name && !configuration.Wifi_Station_Name.isEmpty())
    {
      Serial.println("Connecting to saved network:");
      Serial.printf("%s \n",configuration.Wifi_Station_Name.c_str());
      wl_status_t Connection_Status = WL_IDLE_STATUS;
      int Connection_Retries = 3;
      while (Connection_Status!= WL_CONNECTED && Connection_Retries > 0)
      {
        Serial.println("Connecting");
         Connection_Status = WiFi.begin(configuration.Wifi_Station_Name.c_str(),configuration.Wifi_Station_Password.c_str());
         Connection_Retries--;
         WiFi.waitForConnectResult();
         delay(5000);
      }
      Serial.printf("IP address %s \n",WiFi.localIP().toString().c_str());
      Serial.println(WiFi.getHostname());
      configTime(3600,3600,ntp_server);
      Serial.println(Print_Local_Time());
    }

    if(!WiFi.isConnected())
    {
      Serial.println("Couldnt connect to network, creating AP");
      if(WiFi.softAP(AP_Name)){
        Serial.print("AP name:");Serial.println(AP_Name);
        Serial.print("IP Address");Serial.println(WiFi.softAPIP().toString());
      }
      else {
        Serial.println ("Couldnt create AP");
      }
    }

}

TaskHandle_t webServer_task, status_LED_task, logic_task;
void setup()
{
    //Setup Inputs and outputs
    setup_IO();

    Serial.begin(115200);
    //Start status LED
    Serial.println("Creating status led task");
    xTaskCreate(Status_LED_function,"status_LED_task",1000,&Status_LED_frequency_ms,0,&status_LED_task);

    //read settings from non volatile memory 
    Serial.println("Retrieving settings from memory");
    settings.load_settings();
    Config configuration = settings.get_Config();

    //start WiFi
    setup_Connectivity(configuration);

    //start Web Server
    Serial.println("Starting WebServer");
  
    webServer.serveStatic("/",SPIFFS,"/index.html"); //root
    webServer.serveStatic("/filesystem",SPIFFS,"/"); //display filesystem

    webServer.on("/Control", handle_Control);
    webServer.on("/Configure", handle_Configure);
    webServer.on("/Networks", handle_Networks);
    webServer.on("/Tasks", handle_Tasks);
    webServer.on("/Update", handle_Update);
    webServer.on("/Upload", HTTP_POST, handle_Upload_GET, handle_Upload_POST);
    webServer.on("/variable", HTTP_POST, handle_variable);
    webServer.begin();

    //Start mDNS responder - to respond to Bonjour queries from Linux & MAC
    Serial.printf("Starting mDNS with hostname: %s\n",configuration.hostname.c_str());
    if(!mDNSresponder.begin(configuration.hostname.c_str()))Serial.println("Failed starting mdnsresponder");
    // Add service to MDNS-SD
    MDNS.addService("_http", "_tcp", 80);
    //Start NETBIOS responder - to respond to queries from Windows
    netBIOSresponder.begin(configuration.hostname.c_str());

    //end of configuration, change status led frequency
    Status_LED_frequency_ms = 1000;
    //Create webserver task
    Serial.println("Creating webserver task");
    xTaskCreate(webServer_function,"webServer_task",100000,NULL,0,&webServer_task);
    //Create logic task
    Serial.println("Creating logic task");
    xTaskCreate(logic_function,"logic_task",10000,NULL,0,&logic_task);

    // Set WiFi to station mode and disconnect from an AP if it was previously connecte
    Serial.println("Setup done");
}


void loop()
{    

    checkConnectivity.update();

    vTaskDelay(pdMS_TO_TICKS(1000));
}