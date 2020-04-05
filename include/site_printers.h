#ifndef INCLUDED_SITE_PRINTERS_H
#define INCLUDED_SITE_PRINTERS_H

#include <Arduino.h>

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

String Print_Tasks_List()
{
  Config configuration = settings.get_Config();

  char date[40];
  String tasks_body;
  // if (configuration.tasks_array.Count() > 0)
  // {
  //   tasks_body +=
  //       R"(
  //           <form method="POST"> 
  //           <p>Lista zadan</p>)";

  //   for (int i = 0; i< configuration.tasks_array.Count();i++)
  //   {
  //     Watering_Task task = configuration.tasks_array.get_task(i); //make alias
  //     tasks_body += R"(<input type="radio" name="Task_Num" value=")";
  //     tasks_body += i;
  //     tasks_body += R"(">)";
  //     strftime(date, sizeof(date), "Start %Y/%m/%dT%H:%M ", &(task.start_time));
  //     tasks_body += date;
  //     tasks_body += " Interwal "; tasks_body += task.interval_days;
  //     tasks_body += " Czas " ; tasks_body += task.duration_seconds;
  //     tasks_body += " Moc " ; tasks_body += task.pump_power_percent;
  //     tasks_body += " Objetosc "; tasks_body += task.water_amount;
  //     tasks_body += R"(<br>)";
  //   }
  //   tasks_body +=
  //       R"(
  //               <br>               
  //               <button class="button" type="submit" name="Delete">Usun</button>
  //               <br><br>
  //               </form>
  //               )";
    
  // }
  return tasks_body; 
}

#endif // !INCLUDED_SITE_PRINTERS_H
