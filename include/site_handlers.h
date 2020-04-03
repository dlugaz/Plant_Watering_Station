#ifndef INCLUDED_SITE_HANDLERS_H
#define INCLUDED_SITE_HANDLERS_H

#include <Arduino.h>
#include "WiFi.h"
#include <Preferences.h>
#include <WebServer.h>
#include <time.h>

WebServer webServer;
Preferences preferences;
#include "site_printers.h"
#include "watering_task.h"

struct Process
{
  bool watering_on = false;
  bool pump_on = false;
  bool tasks_on = true;
  int pump_speed = 100;
  float water_level_L;
  float water_flow_L_per_sec;
} current_status;

void handle_root()
{
  String site_body;
  site_body += Print_Main_Website_Header();
  site_body += Print_Local_Time();
  site_body += Print_Main_Website_Footer();
  webServer.send(200, "text/html", site_body.c_str());
}

void handle_Control()
{
  Print_Received_Args();

  if (webServer.argName(0) == "Water")
    current_status.watering_on = webServer.arg(0) == "true";
  if (webServer.argName(0) == "Tasks")
    current_status.tasks_on = webServer.arg(0) == ("true");
  if (webServer.argName(0) == "Pump_Power")
    current_status.pump_speed = webServer.arg(0).toInt();
  if (current_status.pump_speed > 100)
    current_status.pump_speed = 100;
  if (current_status.pump_speed < 0)
    current_status.pump_speed = 0;

  String site_body;
  char dynamic_part[1024] = {0};
  site_body += Print_Main_Website_Header();
  snprintf(dynamic_part, sizeof(dynamic_part),
#include <control.html>
           , current_status.watering_on ? "false" : "true", current_status.watering_on ? "Zalaczone" : "Wylaczone", current_status.pump_speed, current_status.tasks_on ? "false" : "true", current_status.tasks_on ? "Zalaczone" : "Wylaczone");

  site_body += dynamic_part;
  site_body += Print_Main_Website_Footer();
  webServer.send(200, "text/html", site_body.c_str());
}
void handle_Configure()
{
  Print_Received_Args();
  if (webServer.hasArg("Save"))
  {
    configuration.Tank_Volume = webServer.arg(0).toFloat();
    configuration.Tank_Height_cm = webServer.arg(1).toFloat();
  }

  String site_body;
  char dynamic_part[1024] = {0};
  site_body += Print_Main_Website_Header();
  snprintf(dynamic_part, sizeof(dynamic_part),
#include <configure.html>
           , configuration.Tank_Volume, configuration.Tank_Height_cm);
  site_body += dynamic_part;
  site_body += Print_Main_Website_Footer();
  webServer.send(200, "text/html", site_body.c_str());
}
void handle_Networks()
{
  Print_Received_Args();
  String site_body;
  site_body += Print_Main_Website_Header();

  if (webServer.argName(0) == "AP_Name")
  {
    configuration.Wifi_Station_Name = webServer.arg(0);
    if (webServer.argName(1) == "psw")
      configuration.Wifi_Station_Password = webServer.arg(1);
    Serial.println("Saving WIFI settings");
    preferences.clear();
    preferences.putString(key_wifi_name, configuration.Wifi_Station_Name);
    preferences.putString(key_wifi_password, configuration.Wifi_Station_Password);
    preferences.putBytes(key_configuration_struct,&configuration,sizeof(configuration)+1);
  }
  if (webServer.hasArg("connect"))
  {
    if (!configuration.Wifi_Station_Name.isEmpty())
    {
      esp_restart();
    }
  }

  if (webServer.hasArg("forget"))
  {
    configuration.Wifi_Station_Name = "";
    configuration.Wifi_Station_Password = "";
    preferences.putString(key_wifi_name, configuration.Wifi_Station_Name);
    preferences.putString(key_wifi_password, configuration.Wifi_Station_Password);
  }
  if (WiFi.isConnected())
    site_body += "Polaczono z " + WiFi.SSID() + "<br>";
  char dynamic_part[1024] = {0};

  snprintf(dynamic_part, sizeof(dynamic_part),
           R"(
  Obecna zapamietana siec: %s <br>
  <form method="POST">
  %s
  <button class="button type="submit" name="Scan">Skanuj</button>
  </form>
  )"
  ,configuration.Wifi_Station_Name.c_str()
  ,!configuration.Wifi_Station_Name.isEmpty() ?  //if there is saved station name
  R"(
  <button class="button" type="submit" name="connect">Polacz</button><br>
  <button class="button type="submit" name="forget">Zapomnij</button><br>
  )"
  : "");//if theres not

  site_body += dynamic_part;

  //WIFI not connected - show stations
  if (!WiFi.isConnected() || webServer.hasArg("Scan"))
  {
    site_body += Print_Available_Networks();
  }

  site_body += Print_Main_Website_Footer();
  webServer.send(200, "text/html", site_body.c_str());
}
void handle_Tasks()
{
  Print_Received_Args();

  String site_body;
  site_body += Print_Main_Website_Header();
  tm time = {0};
  char date[40] = {0};


  if (webServer.hasArg("Add_Task"))
  {
    if (configuration.numberoftasks < configuration.tasks_array.size())
    {
      if (strptime(webServer.arg(0).c_str(), "%Y/%m/%dT%H:%M", &time) != NULL)
      {
        Watering_Task *new_task = &(configuration.tasks_array[configuration.numberoftasks]);
        new_task->start_time = time;
        new_task->interval_days = webServer.arg(1).toInt();
        new_task->duration_seconds = webServer.arg(2).toInt();
        new_task->pump_power_percent = webServer.arg(3).toInt();
        new_task->water_amount = webServer.arg(4).toInt();
        configuration.numberoftasks++;
      }
      else
        site_body += "Nieprawidlowe dane";
    }
    else
    site_body += "Maksymalna ilosc zadan";
  }


  if (webServer.hasArg("Delete"))
  {
    int numberToDelete = webServer.arg(0).toInt();
    for (size_t i = 0; i < configuration.tasks_array.size(); i++)
    {
      if(i>=numberToDelete )
      {
        if((i+1)<configuration.tasks_array.size())
          configuration.tasks_array[i] = configuration.tasks_array[i+1];
        else
          configuration.tasks_array[i] = Watering_Task();

      }
    }
    
    configuration.numberoftasks--;
  }
  if (configuration.numberoftasks > 0)
  {
    site_body +=
        R"(
            <form method="POST">
            <p>Lista zadan</p>)";

    for (int i = 0; i< configuration.numberoftasks;i++)
    {
      Watering_Task* task = &(configuration.tasks_array[i]); //make alias
      // Print SSID and RSSI for each network found
      site_body += R"(<input type="radio" name="Task_Num" value=")";
      site_body += i;
      site_body += R"(">)";
      strftime(date, sizeof(date), "Start %Y/%m/%dT%H:%M ", &(task->start_time));
      site_body += date;
      site_body += "Interwal " + task->interval_days;
      site_body += "Czas " + task->duration_seconds;
      site_body += "Moc " + task->pump_power_percent;
      site_body += "Objetosc ";
      site_body += task->water_amount;
      site_body += R"(<br>)";
    }
    site_body +=
        R"(
                <br>               
                <button class="button" type="submit" name="Delete">Usun</button>
                <br><br>
                </form>
                )";
  }

char dynamic_part[1024] = {0};

getLocalTime(&time);
strftime(date, sizeof(date), "%Y/%m/%dT%H:%M", &time);
//2019/03/23T22:20
snprintf(dynamic_part, sizeof(dynamic_part),
#include <tasks.html>
         , date);

site_body += dynamic_part;
site_body += Print_Main_Website_Footer();
webServer.send(200, "text/html", site_body.c_str());
}

#endif // !INCLUDED_SITE_HANDLERS_H
