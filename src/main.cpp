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
#include "SPI.h"
#include "Wire.h"
#include "Adafruit_VL53L0X.h"
#include "Adafruit_seesaw.h"

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


void setup_Connectivity()
{
    if(WiFi.isConnected()) return; 

    Config configuration = settings.get_Config();

    Status_LED_frequency_ms = 200;

    if(WiFi.setHostname(hostname))Serial.printf("Set hostname to %s \n",hostname);

    if(configuration.Wifi_Station_Name != nullptr && strlen(configuration.Wifi_Station_Name)>0)
    {
      Serial.println("Connecting to saved network:");
      Serial.printf("%s \n",configuration.Wifi_Station_Name);
      wl_status_t Connection_Status = WL_IDLE_STATUS;
      int Connection_Retries = 3;
      while (Connection_Status!= WL_CONNECTED && Connection_Retries > 0)
      {
        Serial.println("Connecting");
         Connection_Status = WiFi.begin(configuration.Wifi_Station_Name,configuration.Wifi_Station_Password);
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
#define SOIL_SENSOR1_ADDRESS 54U
#define SOIL_SENSOR2_ADDRESS 55U

Adafruit_seesaw* setup_soil_sensor(uint8_t address)
{
  Serial.println("Soil sensor init");
  Adafruit_seesaw* sensor = new Adafruit_seesaw(&Wire);
  sensor->begin(address);
  Serial.printf("Test soil sensor \nTemp: %f \nMoisture: %d \n ",sensor->getTemp(),sensor->touchRead(0));
  return sensor;
}

#define WATER_LEVEL_SENSOR_ADDRESS 41U
Adafruit_VL53L0X* setup_water_level_sensor()
{
    //Initialize tank level sensor
    Serial.println("Water Level Sensor Init");
    Adafruit_VL53L0X* sensor = new Adafruit_VL53L0X();
    sensor->begin(WATER_LEVEL_SENSOR_ADDRESS);
    Serial.printf("Test Distance : %d \n",sensor->readRange());
    return sensor;
}

void setup_i2c()
{
  //Initialize I2C
  Serial.println("I2C Initialization");
  Wire.begin(I2C_SDA, I2C_SCL);

  //Scan for connected devices
  for (uint8_t i = 0; i < 127; i++)
  {
    Wire.beginTransmission(i);
    uint8_t error = Wire.endTransmission();
    if (error == 0)
    {
      Serial.printf("I2C Device found at address %d \n", i);
      if (i == WATER_LEVEL_SENSOR_ADDRESS)
      {
        water_distance_sensor = setup_water_level_sensor();
        water_distance_sensor_found = true;
      }
      if (i == SOIL_SENSOR1_ADDRESS)
      {
        soil_sensor1_found = true;
        soil_sensor1 = setup_soil_sensor(SOIL_SENSOR1_ADDRESS);
      }
      if (i == SOIL_SENSOR2_ADDRESS)
      {
        soil_sensor2_found = true;
        soil_sensor2 = setup_soil_sensor(SOIL_SENSOR2_ADDRESS);
      }
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

    setup_Connectivity();   
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
    //I2C
    setup_i2c();

    // Set WiFi to station mode and disconnect from an AP if it was previously connecte
    Serial.println("Setup done");
}


void loop()
{    

    checkConnectivity.update();

    vTaskDelay(pdMS_TO_TICKS(1000));
}