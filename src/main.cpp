/*
 *  This sketch demonstrates how to scan WiFi networks.
 *  The API is almost the same as with the WiFi Shield library,
 *  the most obvious difference being the different file you need to include:
 */
#include "WiFi.h"
#include <Preferences.h>
#include <WebServer.h>


//My includes
#include "configuration.h"
#include "connect.h"
#include "site_handlers.h"
#include "watering_task.h"
#include "tasks_functions.h"
#include "io_definition.h"

const char AP_Name[] = "Plant_Watering_Station";
const char ntp_server[] = "pool.ntp.org";

WiFiServer wifiServer;


void analogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  // calculate duty, 8191 from 2 ^ 13 - 1
  uint32_t duty = (8191 / valueMax) * min(value, valueMax);

  // write duty to LEDC
  ledcWrite(channel, duty);
}



TaskHandle_t webServer_task;
TaskHandle_t status_LED_task;

void setup()
{
    //Setup IO
    setup_IO();

    Serial.begin(115200);
    //Start status LED
    Serial.println("Creating status led task");
    xTaskCreate(Status_LED_function,"status_LED_task",1000,&Status_LED_frequency_ms,0,&status_LED_task);
    Status_LED_frequency_ms = 500;
    //read settings from non volatile memory 
    Serial.println("Retrieving settings from memory");
    preferences.begin("settings");
    configuration.Wifi_Station_Name = preferences.getString(key_wifi_name);
    configuration.Wifi_Station_Password = preferences.getString(key_wifi_password);
    Serial.println(configuration.Wifi_Station_Name);
    Serial.println(configuration.Wifi_Station_Password);

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
        configuration.Wifi_Station_Name = AP_Name;
        Serial.print("AP name:");Serial.println(AP_Name);
        Serial.print("IP Address");Serial.println(WiFi.softAPIP().toString());
      }
      else {
        Serial.println ("Couldnt create AP");
      }
    }
    
      Serial.println("Starting WebServer");


      //
      webServer.on("/",handle_root);
      webServer.on("/Control",handle_Control);
      webServer.on("/Configure",handle_Configure);
      webServer.on("/Networks",handle_Networks);
      webServer.on("/Tasks",handle_Tasks);

      webServer.begin(80);

    //end of configuration, change status led frequency
    Status_LED_frequency_ms = 1000;
      
    Serial.println("Creating webserver task");
    xTaskCreate(webServer_function,"webServer_task",10000,NULL,0,&webServer_task);

    // Set WiFi to station mode and disconnect from an AP if it was previously connecte
    Serial.println("Setup done");
}


void loop()
{    
    if(digitalRead(START_BUTTON)==0&& current_status.start_button == false)
    {
      Serial.println("Button pressed");
      current_status.start_button = true;
      current_status.start_button_time_ms = millis();
    }
    if(digitalRead(START_BUTTON)==1&& current_status.start_button == true)
    {
      Serial.println("Button depressed");
      current_status.start_button = false;
      current_status.start_button_time_ms = 0;
    }

    if(current_status.start_button_time_ms > 10000)
    {
      preferences.clear();
      Serial.println("Memory has been reset");
    }
    
    // logic 
    if(current_status.watering_on || current_status.start_button)
    {
      current_status.pump_on = true;
      analogWrite(PUMP_PWM_CHANNEL,current_status.pump_speed,100);
    }else
    {
       current_status.pump_on = false;
      analogWrite(PUMP_PWM_CHANNEL,0);
    }
    vTaskDelay(10);
}