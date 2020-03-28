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
#include "control.h"
#include "configure.h"
#include "connect.h"
#include "tasks.h"

struct Watering_Task
{
  tm start_time;
  int duration_seconds;
  uint8_t pump_power_percent; //0-100% 
};
struct Retain_Memory
{
String Wifi_Station_Name;
String Wifi_Station_Password;
tm Last_Watering_Timestamp;
IPAddress Board_Ip;
float Tank_Volume;

};

struct Process
{
  bool watering_on = false;
  bool pump_on = false;
  bool tasks_on = true;
  int pump_speed = 0;
  String command;
}current_status;
const char memory []= "settings";

const char AP_Name[] = "Plant_Watering_Station";
const char ntp_server[] = "pool.ntp.org";
Preferences Settings;
WiFiServer wifiServer;
WebServer webServer;


void setup()
{
    Serial.begin(115200);
    //read settings from non volatile memory
    Settings.begin(memory);
    char buffer [sizeof(Retain_Memory)] = {0};
    Settings.getBytes(memory,buffer,sizeof(buffer));
    Serial.println("Retrieved data from memory:");
    Serial.print(buffer);
    Retain_Memory* Saved_State;
    Saved_State = (Retain_Memory*) buffer; //cast buffer to struct
    
    if(Saved_State->Wifi_Station_Name && !Saved_State->Wifi_Station_Name.isEmpty())
    {
      Serial.print("Connecting to saved network:"); Serial.print(Saved_State->Wifi_Station_Name);
      wl_status_t Connection_Status = WL_IDLE_STATUS;
      int Connection_Retries = 3;
      while (Connection_Status!= WL_CONNECTED && Connection_Retries > 0)
      {
         Connection_Status = WiFi.begin(Saved_State->Wifi_Station_Name.c_str(),Saved_State->Wifi_Station_Password.c_str());
         Connection_Retries--;
         delay(5000);
      }
      
    }
    if(!WiFi.isConnected())
    {
      Serial.println("Couldnt connect to network, creating AP");
      WiFi.softAP(AP_Name);
      Serial.print("AP name:");Serial.println(AP_Name);
      Serial.print("IP Address");Serial.println(WiFi.softAPIP().toString());
    }
    
      Serial.println("Starting WebServer");
      wifiServer.begin(80);
    
    
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
    String currentLine;
    bool disconnect_client = false;
    if(wifiServer)
    {
      WiFiClient client;
      client = wifiServer.available();
      if (client) {
        Serial.println("new client");
        // an http request ends with a blank line
        bool currentLineIsBlank = true;
        while (client.connected()) {
          if (client.available()) {
            char c = client.read();
            Serial.write(c);
            // if you've gotten to the end of the line (received a newline
            // character) and the line is blank, the http request has ended,
            // so you can send a reply
            if (c == '\n' && currentLineIsBlank) {
              // send a standard http response header
              client.println("HTTP/1.1 200 OK");
              client.println("Content-Type: text/html");
              client.println("Connection: close");  // the connection will be closed after completion of the response
              // client.println("Refresh: ");  // refresh the page automatically every 5 sec
              client.println();
              const char website_body[] =
              #include "csswebsite.html"
              ;
              client.print(website_body);
              
              switch(current_page)
              {
                
                case Control:
                Serial.println("Control Site");
                  Print_Control_Site(client,current_status.watering_on,current_status.tasks_on);
                  break;
                case Configure:
                Serial.println("Configure Site");
                  Print_Configure_Site(client,30.0);
                  break;
                case Tasks:
                  Serial.println("Tasks Site");
                  Print_Tasks_Site(client);
                break;
                case Networks:
                  Serial.println("Tasks Site");
                  Print_Available_Networks(client);
                break;
                default:
                  break;
              }

              const char website_ending[]=
              #include "css_stopka.html"
              ;
              client.print(website_ending);
              // client.println("<html>");

              // Print_Available_Networks(client);
              // client.println("</html>");
              break;
            }
            if (c == '\n') {
              // you're starting a new line
              currentLineIsBlank = true;
            } else if (c != '\r') {  // if you got anything else but a carriage return character,
              currentLine += c;      // add it to the end of the currentLine
            }

            // Check to see if the client request was "GET /H" or "GET /L":
            if (currentLine.startsWith("GET /Control")) {
              Serial.println("\nControl");
              current_page = Control;
              String command = currentLine.substring(currentLine.indexOf('?')+1);
              Serial.println(command);
              if(command.startsWith("WaterOn"))current_status.watering_on = true;
              if(command.startsWith("WaterOff"))current_status.watering_on = false;
              if(command.startsWith("TasksOn"))current_status.tasks_on = true;
              if(command.startsWith("TasksOff"))current_status.tasks_on = false;
              Serial.print("Watering:"); Serial.println(current_status.watering_on?"true":"false");
              Serial.print("Tasks:"); Serial.println(current_status.tasks_on?"true":"false");
            }
            if (currentLine.startsWith("GET /Configure")) {
              current_page = Configure;
            }
            if (currentLine.startsWith("GET /Tasks")) {
              current_page = Tasks;
            }
            if (currentLine.startsWith("GET /Networks")) {
              current_page = Networks;
            }
          }
        }
        // give the web browser time to receive the data
        delay(1);

        // close the connection:
        if(disconnect_client){
        client.stop();
        Serial.println("client disonnected");
        }
      }

    }

}