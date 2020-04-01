#ifndef INCLUDED_SITE_HANDLERS_H
#define INCLUDED_SITE_HANDLERS_H

#include <Arduino.h>
#include "WiFi.h"
#include <Preferences.h>
#include <WebServer.h>

WebServer webServer;
Preferences preferences;
#include "site_printers.h"
#include "watering_task.h"

struct Process
{
  bool watering_on = false;
  bool pump_on = false;
  bool tasks_on = true;
  int pump_speed = 0;
  bool start_button = false;
  Watering_Task tasks_array[20];
  uint16_t start_button_time_ms;
}current_status;



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
  if(webServer.argName(0) == "Pump_Power")
    current_status.pump_speed = webServer.arg(0).toInt();
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
  Obecna siec: %s <br>
  <form method="POST">
  <button class="button" type="submit" name="disconnect">Rozlacz</button><br>
  <button class="button type="submit" name="forget">Zapomnij</button><br>
  <button class="button type="submit" name="Scan">Skanuj</button>
  </form>
  )"
  ,WiFi.SSID().c_str());

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
  if(webServer.hasArg("Add_Task"))
  {

  }
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
  ,todays_date);

  site_body += dynamic_part;
  site_body += Print_Main_Website_Footer();
  webServer.send(200,"text/html", site_body.c_str());
}

#endif // !INCLUDED_SITE_HANDLERS_H

