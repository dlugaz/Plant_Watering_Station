/*
 *  This sketch demonstrates how to scan WiFi networks.
 *  The API is almost the same as with the WiFi Shield library,
 *  the most obvious difference being the different file you need to include:
 */
#define MULTI_LINE_STRING(...) #__VA_ARGS__
#include "WiFi.h"
#include <Preferences.h>
#include <WebServer.h>


//My includes
#include "configure.h"
#include "connect.h"

struct Watering_Task
{
  tm start_time;
  int duration_seconds;
  uint8_t pump_power_percent; //0-100% 
};
struct Config
{
tm Last_Watering_Timestamp;
IPAddress Device_Ip;
float Tank_Volume;
String Wifi_Station_Name;
String Wifi_Station_Password;

Preferences memory_access;

// const char key_tank_volume [] = "TankVolume";
// const char key_wifi_name [] = "WifiName";
// const char key_wifi_password [] = "WifiPass";
// const char key_device_ip [] = "DeviceIp";

// Config()
// {
//   memory_access.begin("settings");
// }

// bool save_settings()
// {
//   memory_access.putString(key_wifi_name,Wifi_Station_Password);
//   memory_access.putString(key_wifi_password,Wifi_Station_Password);
//   memory_access.putFloat(key_tank_volume,Tank_Volume);
//   memory_access.putString(key_device_ip,Device_Ip.toString());
// }
// bool load_settings ()
// {
//   memory_access.getString(key_wifi_name,Wifi_Station_Password);
//   memory_access.getString(key_wifi_password,Wifi_Station_Password);
//   memory_access.getFloat(key_tank_volume,Tank_Volume);
//   memory_access.getString(key_device_ip,Device_Ip.toString());

// }
} configuration;


const char key_tank_volume [] = "TankVolume";
const char key_wifi_name [] = "WifiName";
const char key_wifi_password [] = "WifiPass";
const char key_device_ip [] = "DeviceIp";
struct Process
{
  bool watering_on = false;
  bool pump_on = false;
  bool tasks_on = true;
  int pump_speed = 0;
  bool start_button = false;
  
}current_status;



const char AP_Name[] = "Plant_Watering_Station";
const char ntp_server[] = "pool.ntp.org";
Preferences preferences;
WiFiServer wifiServer;
WebServer webServer;

void analogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  // calculate duty, 8191 from 2 ^ 13 - 1
  uint32_t duty = (8191 / valueMax) * min(value, valueMax);

  // write duty to LEDC
  ledcWrite(channel, duty);
}
String Print_Local_Time()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return String("Failed to obtain time");
  }
  char buffer[64] = {0};
  strftime(buffer,sizeof(buffer),"%A, %B %d %Y %H:%M:%S",&timeinfo);
  
  return String(buffer);
}
void Print_Received_Args()
{
  Serial.println(webServer.uri());
    int args_count = webServer.args();
  Serial.printf("Number of args %d \n", args_count);
  if(args_count)
  {
    for(int i=0; i<args_count;++i)
    Serial.printf("Argument %d %s %s\n" ,i,webServer.argName(i).c_str(),webServer.arg(i).c_str());
  }
}
String Print_Main_Website_Header()
{
  String website(
    #include <csswebsite.html>
  );
  return website;
}
String Print_Main_Website_Footer()
{
  String website(
    #include <css_stopka.html>
  );

  return website;
}
void handle_root()
{
  String site_body;
  site_body += Print_Main_Website_Header();
  site_body += Print_Local_Time();
  site_body += Print_Main_Website_Footer();
  webServer.send(200,"text/html", site_body.c_str());
}

void handle_Control()
{
  Print_Received_Args();

  if(webServer.argName(0) == "Water")
    current_status.watering_on = webServer.arg(0) == "true";
  if(webServer.argName(0) == "Tasks")
    current_status.tasks_on = webServer.arg(0) == ("true");
  if(webServer.argName(1) == "Pump_Power")
    current_status.pump_speed = webServer.arg(1).toInt();
    if(current_status.pump_speed>100)current_status.pump_speed=100;
    if(current_status.pump_speed<0)current_status.pump_speed =0;

  String site_body;
  char dynamic_part [1024] = {0};
  site_body += Print_Main_Website_Header();
  snprintf(dynamic_part,sizeof(dynamic_part),
  #include <control.html>
  ,current_status.watering_on?"false":"true"
  ,current_status.watering_on?"Zalaczone":"Wylaczone"
  ,current_status.pump_speed
  ,current_status.tasks_on?"false":"true"
  ,current_status.tasks_on?"Zalaczone":"Wylaczone");
  
  site_body += dynamic_part;
  site_body += Print_Main_Website_Footer();
  webServer.send(200,"text/html", site_body.c_str());
}
void handle_Configure()
{
  Print_Received_Args();
  if(webServer.hasArg("Save"))
  {
    configuration.Tank_Volume = webServer.arg(0).toFloat();
  }

  String site_body;
  char dynamic_part [1024] = {0};
  site_body += Print_Main_Website_Header();
  snprintf(dynamic_part,sizeof(dynamic_part),
  #include <configure.html>
  ,configuration.Tank_Volume
  );
  site_body += dynamic_part;
  site_body += Print_Main_Website_Footer();
  webServer.send(200,"text/html", site_body.c_str());
}
void handle_Networks()
{
  Print_Received_Args();
  String site_body;
  site_body += Print_Main_Website_Header();


  if(webServer.argName(0) == "AP_Name"){
    configuration.Wifi_Station_Name = webServer.arg(0);
    if(webServer.argName(1) == "psw")
      configuration.Wifi_Station_Password = webServer.arg(1);
    Serial.println("Saving WIFI settings");
    preferences.clear();
    preferences.putString(key_wifi_name,configuration.Wifi_Station_Name);
    preferences.putString(key_wifi_password,configuration.Wifi_Station_Password);
  }
  if(webServer.hasArg("disconnect"))
  {
    WiFi.disconnect();
  }
    if(webServer.hasArg("forget"))
  {
    configuration.Wifi_Station_Name = "";
    configuration.Wifi_Station_Password ="";
    preferences.putString(key_wifi_name,configuration.Wifi_Station_Name);
    preferences.putString(key_wifi_password,configuration.Wifi_Station_Password);
  }
  
  char dynamic_part [1024] = {0};

  snprintf(dynamic_part,sizeof(dynamic_part),
  R"(
  "Obecna siec: %s <br>"
  <form method="POST">
  <button class="button" type="submit" name="disconnect">Rozlacz</button><br>
  <button class="button type="submit" name="forget">Zapomnij</button><br>
  <button class="button type="submit" name="Scan">"Skanuj"</button>
  </form>
  )"
  ,configuration.Wifi_Station_Name.c_str());

  site_body += dynamic_part;

  //WIFI not connected - show stations
  if(!WiFi.isConnected() || webServer.hasArg("Scan"))
  {
     site_body += Print_Available_Networks();
  }

  site_body += Print_Main_Website_Footer();
  webServer.send(200,"text/html", site_body.c_str());
}
void handle_Tasks()
{
  Print_Received_Args();
  String site_body;
  site_body += Print_Main_Website_Header();
    char dynamic_part [1024] = {0};
  char todays_date [40] = {0};
  tm today = {0};
  getLocalTime(&today);
  strftime(todays_date, sizeof(todays_date), "%Y/%m/%dT%H:%M", &today);
  //2019/03/23T22:20
  snprintf(dynamic_part,sizeof(dynamic_part),
  #include <tasks.html>
  ,configuration.Wifi_Station_Name.c_str());

  site_body += dynamic_part;
  site_body += Print_Main_Website_Footer();
  webServer.send(200,"text/html", site_body.c_str());
}


void setup()
{
    Serial.begin(115200);
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
    
    #define START_BUTTON 0
    #define PUMP1_PIN 3
    #define LED 2
      //Setup IO
      //Built in LED 
      pinMode(LED,OUTPUT);
      //Pump Output
      pinMode(PUMP1_PIN,OUTPUT);
      //setup pwm for pump
      
      // use first channel of 16 channels (started from zero)
      #define PUMP_PWM_CHANNEL     0
      // use 13 bit precission for LEDC timer
      #define PWM_TIMER_13_BIT  13
      // use 5000 Hz as a LEDC base frequency
      #define PWM_BASE_FREQ     5000

      ledcSetup(PUMP_PWM_CHANNEL, PWM_BASE_FREQ, PWM_TIMER_13_BIT);
      ledcAttachPin(PUMP1_PIN, PUMP_PWM_CHANNEL);
      //for test only
      ledcAttachPin(LED, PUMP_PWM_CHANNEL);
      //Start/stop button
      pinMode(START_BUTTON,INPUT_PULLUP);


    // Set WiFi to station mode and disconnect from an AP if it was previously connecte
    Serial.println("Setup done");
}


enum pages {
  Control = 0,
  Configure,
  Tasks,
  Networks
} current_page;

void loop()
{
    webServer.handleClient();
    current_status.start_button = digitalRead(START_BUTTON) !=1;
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
}