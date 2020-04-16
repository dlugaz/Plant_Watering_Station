#ifndef INCLUDED_SITE_HANDLERS_H
#define INCLUDED_SITE_HANDLERS_H

#include <Arduino.h>
#include "WiFi.h"
#include <Preferences.h>
#include <WebServer.h>
#include <time.h>
#include <Update.h>

WebServer webServer;
#include "site_printers.h"
#include "watering_task.h"

volatile struct Process
{
  bool watering_on = false;
  bool pump_on = false;
  bool tasks_on = true;
  int pump_speed = 100;
  float water_level_L;
  float water_flow_L_per_min;
  float water_pumped;
  float water_amount_when_started;
} current_status;

/** 
 * Display / website and handle requests
 */
void handle_root()
{
  String site_body;
  site_body += Print_Main_Website_Header();
  site_body += Print_Local_Time();
  site_body += Print_Main_Website_Footer();
  webServer.send(200, "text/html", site_body.c_str());
}

/** 
 * Display /Control website and handle requests
 */

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
  , current_status.watering_on ? "false" : "true",
   current_status.watering_on ? "Zalaczone" : "Wylaczone",
    current_status.pump_speed, current_status.tasks_on ? "false" : "true"
  , current_status.tasks_on ? "Zalaczone" : "Wylaczone"
  ,current_status.water_level_L
  ,current_status.water_pumped
  ,current_status.water_flow_L_per_min);

  site_body += dynamic_part;
  site_body += Print_Main_Website_Footer();
  webServer.send(200, "text/html", site_body.c_str());
}

/** 
 * Display /Configure website and handle requests
 */
void handle_Configure()
{
  Print_Received_Args();
  Config configuration = settings.get_Config();

  if (webServer.hasArg("Save"))
  {
    configuration.Tank_Volume = webServer.arg(0).toFloat();
    configuration.Tank_Height_cm = webServer.arg(1).toFloat();

    settings.save_settings(configuration);
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

/** 
 * Display /Networks website and handle requests
 */
void handle_Networks()
{
  Print_Received_Args();
  Config configuration = settings.get_Config();

  String site_body;
  site_body += Print_Main_Website_Header();

  if (webServer.argName(0) == "AP_Name")
  {
    configuration.Wifi_Station_Name = webServer.arg(0);
    if (webServer.argName(1) == "psw")
      configuration.Wifi_Station_Password = webServer.arg(1);
    Serial.println("Saving WIFI settings");

    settings.save_settings(configuration);
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

    settings.save_settings(configuration);
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

/** 
 * Display /Tasks website and handle requests
 */
void handle_Tasks()
{
  Config configuration = settings.get_Config();

  Print_Received_Args();

  String site_body;
  site_body += Print_Main_Website_Header();
  tm time = {0};
  char date[40] = {0};
  settings.load_settings();

  if (webServer.hasArg("Add_Task"))
  {
      if (strptime(webServer.arg(0).c_str(), "%Y/%m/%dT%H:%M", &time) != NULL
      ||strptime(webServer.arg(0).c_str(), "%Y-%m-%dT%H:%M", &time) != NULL)
      {
        Watering_Task new_task;
        new_task.start_time = time;
        new_task.interval_days = webServer.arg(1).toInt();
        new_task.duration_seconds = webServer.arg(2).toInt();
        new_task.pump_power_percent = webServer.arg(3).toInt();
        new_task.water_amount = webServer.arg(4).toFloat();
        if(configuration.tasks_array.Add_Task(new_task))
          settings.save_settings(configuration);
        else
          site_body += "Maksymalna ilosc zadan";
      }
      else
        site_body += "Nieprawidlowe dane";
  }
  

  if (webServer.hasArg("Delete"))
  {
    int numberToDelete = webServer.arg(0).toInt();
    if (configuration.tasks_array.Delete_Task(numberToDelete))
      settings.save_settings(configuration);
  }
  
  site_body += Print_Tasks_List();
  
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

const char* updateSite = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='/Upload' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/Upload',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')" 
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";

const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
void handle_Update()
{
    webServer.send(200, "text/html", updateSite);
}
void handle_Upload_GET()
{
    webServer.sendHeader("Connection", "close");
    webServer.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
}

void handle_Upload_POST()
{
      HTTPUpload& upload = webServer.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      Serial.println("Flashing firmware..");
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }

}

#endif // !INCLUDED_SITE_HANDLERS_H
