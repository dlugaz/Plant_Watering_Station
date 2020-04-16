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



//schedule level measurement

RunningAverage Tank_Level_Average (5);
void Tank_Level_Measurement_function()
{
  Config configuration = settings.get_Config();
  float L_per_cm = ((float)configuration.Tank_Volume/(float)configuration.Tank_Height_cm);
  unsigned long measurement = distanceToWater.ping_median();
  // Serial.println(measurement);
  // measurement = NewPing::convert_cm(measurement);
  float measured_distance = (float)measurement / US_ROUNDTRIP_CM;
  // Serial.println(measured_distance);
  // current_status.water_level_L =  (configuration.Tank_Height_cm - measured_distance)*L_per_cm;
  Tank_Level_Average.addValue((configuration.Tank_Height_cm - measured_distance)*L_per_cm);
  current_status.water_level_L =  Tank_Level_Average.getAverage();

  Serial.printf("Water level %f \n",current_status.water_level_L);
}
Ticker Tank_Level_Measurement(Tank_Level_Measurement_function,1000,0,MILLIS);

//schedule water flow measurement every 5 second
float last_water_level_L = 0.0; 
void Tank_Flow_Measurement_function();
Ticker Tank_Flow_Measurement(Tank_Flow_Measurement_function,10000,0,MILLIS);

long time_of_last_flow_measurement = 0;
void Tank_Flow_Measurement_function()
{
  // Serial.printf("Tank_Flow: last %f, cur %f \n",last_water_level_L,current_status.water_level_L);
  //calculate time from last measurement
  uint32_t elapsed = millis() - time_of_last_flow_measurement;
  // Serial.printf("elapsed time %d", elapsed);
  float time_s = (float)elapsed/1000.0f;
  // Serial.printf("Flow time %f \n",time_s);
  //calculate flow as difference in levels divided by time
  if(last_water_level_L!=0.0) // discard first measurement
    current_status.water_flow_L_per_min = (last_water_level_L - current_status.water_level_L)*(60.0f/time_s);
  //save water level for next iteration
  last_water_level_L = current_status.water_level_L;
  Serial.print("Flow ");Serial.println(current_status.water_flow_L_per_min);
  time_of_last_flow_measurement = millis();
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
    Status_LED_frequency_ms = 200;
    //read settings from non volatile memory 
    Serial.println("Retrieving settings from memory");
    settings.load_settings();

    Config configuration = settings.get_Config();

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
        configuration.Wifi_Station_Name = AP_Name;
        Serial.print("AP name:");Serial.println(AP_Name);
        Serial.print("IP Address");Serial.println(WiFi.softAPIP().toString());
      }
      else {
        Serial.println ("Couldnt create AP");
      }
    }
    
      Serial.println("Starting WebServer");

      webServer.on("/",handle_root);
      webServer.on("/Control",handle_Control);
      webServer.on("/Configure",handle_Configure);
      webServer.on("/Networks",handle_Networks);
      webServer.on("/Tasks",handle_Tasks);
      webServer.on("/Update",handle_Update);
      webServer.on("/Upload",HTTP_POST,handle_Upload_GET,handle_Upload_POST);

      webServer.begin(80);
    //
    Serial.printf("Starting mDNS with hostname: %s\n",hostname);
    if(!mDNSresponder.begin(hostname))Serial.println("Failed starting mdnsresponder");
        // Add service to MDNS-SD
    MDNS.addService("_http", "_tcp", 80);

    //end of configuration, change status led frequency
    Status_LED_frequency_ms = 1000;
    //Create webserver task
    Serial.println("Creating webserver task");
    xTaskCreate(webServer_function,"webServer_task",100000,NULL,0,&webServer_task);
    //Create logic task
    Serial.println("Creating logic task");
    xTaskCreate(logic_function,"logic_task",10000,NULL,0,&logic_task);

    //Start timing tasks
    Tank_Flow_Measurement.start();
    Tank_Level_Measurement.start();
    // Set WiFi to station mode and disconnect from an AP if it was previously connecte
    Serial.println("Setup done");
}


void loop()
{    
    //handle all timing tasks events
    Tank_Level_Measurement.update();
    Tank_Flow_Measurement.update();
    //handle all buttons
    StartButton.update();

    
    vTaskDelay(pdMS_TO_TICKS(100));
}