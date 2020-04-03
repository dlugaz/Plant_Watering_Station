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

//schedule level measurement every 1000 ms
void Tank_Level_Measurement_function()
{
  float L_per_cm = ((float)configuration.Tank_Volume/(float)configuration.Tank_Height_cm);
  current_status.water_level_L =  (configuration.Tank_Height_cm - (float)distanceToWater.ping_median())*L_per_cm;
  Serial.print("Water level ");Serial.println(current_status.water_level_L);
}
Ticker Tank_Level_Measurement(Tank_Level_Measurement_function,1000);

//schedule water flow measurement every 5 second
float last_water_level_L = 0.0; 
void Tank_Flow_Measurement_function();
Ticker Tank_Flow_Measurement(Tank_Flow_Measurement_function,5000);

void Tank_Flow_Measurement_function()
{
  //calculate time from last measurement
  float time_s = (float)Tank_Flow_Measurement.elapsed()/1000.0f;
  //calculate flow as difference in levels divided by time
  if(last_water_level_L!=0.0) // discard first measurement
    current_status.water_flow_L_per_sec = (last_water_level_L - current_status.water_level_L)/time_s;
  //save water level for next iteration
  last_water_level_L = current_status.water_level_L;
}


TaskHandle_t webServer_task;
TaskHandle_t status_LED_task;

void setup()
{
    //Setup Inputs and outputs
    setup_IO();

    Serial.begin(115200);
    //Start status LED
    Serial.println("Creating status led task");
    xTaskCreate(Status_LED_function,"status_LED_task",1000,&Status_LED_frequency_ms,0,&status_LED_task);
    Status_LED_frequency_ms = 200;
    //read settings from non volatile memory 
    Serial.println("Retrieving settings from memory");
    preferences.begin("settings");
    // configuration.Wifi_Station_Name = preferences.getString(key_wifi_name);
    // configuration.Wifi_Station_Password = preferences.getString(key_wifi_password);
    // preferences.putBytes(key_configuration_struct,&configuration,sizeof(configuration));

    // preferences.getBytes(key_configuration_struct,&configuration,sizeof(configuration));
    load_settings();
   
    Serial.println(configuration.Wifi_Station_Name);
    Serial.println(configuration.Wifi_Station_Password);
    // Serial.println(configuration2.Wifi_Station_Name);

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
    //Create webserver task
    Serial.println("Creating webserver task");
    xTaskCreate(webServer_function,"webServer_task",10000,NULL,0,&webServer_task);
    //Start timing tasks
    Tank_Flow_Measurement.start();
    Tank_Level_Measurement.start();
    // Set WiFi to station mode and disconnect from an AP if it was previously connecte
    Serial.println("Setup done");
}

AsyncDelay Pump_On_Timer;

void loop()
{    

    // logic 
    if(current_status.watering_on )
    {
      Serial.println("Watering On");
      current_status.pump_on = true;
      analogWrite(PUMP_PWM_CHANNEL,map(current_status.pump_speed,0,100,170,255),255);
    }else
    {
      Serial.println("Watering Off");
       current_status.pump_on = false;
      analogWrite(PUMP_PWM_CHANNEL,0);
    }

    if(current_status.tasks_on)
    {
      Serial.println("Tasks On");
      tm current_time = {0};
      getLocalTime(&current_time);
      //check if any task is due
      int due_task_number = configuration.tasks.isAnyTaskDue();
      if (due_task_number)
      {
        Serial.println("Some task is due");
        Serial.println(due_task_number);
        //make alias for shorter name
        Watering_Task due_task = configuration.tasks[due_task_number];
        //start pump for configured time
        if(!current_status.watering_on){
          Serial.println("Starting Pumps and timer");
          Serial.println(due_task.duration_seconds);
          Pump_On_Timer.start(due_task.duration_seconds, AsyncDelay::units_t::MILLIS);
          current_status.watering_on = true;
          current_status.pump_speed = due_task.pump_power_percent;
        }
        //Task has been fulfiled
        if (Pump_On_Timer.isExpired() && current_status.watering_on)
        {
          Serial.println("Task fulfilled");
          current_status.watering_on = false;
          due_task.start_time = current_time; //refresh last start time
        }
      }

    }

    //handle all timing tasks events
    Tank_Level_Measurement.update();
    Tank_Flow_Measurement.update();
    //handle all buttons
    StartButton.update();

    
    vTaskDelay(1000);
}