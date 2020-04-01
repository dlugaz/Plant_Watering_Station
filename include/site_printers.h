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

#endif // !INCLUDED_SITE_PRINTERS_H
