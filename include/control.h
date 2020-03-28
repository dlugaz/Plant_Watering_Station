#ifndef INCLUDED_CONTROL_H
#define INCLUDED_CONTROL_H
#include <WiFiClient.h>

void Print_Control_Site(WiFiClient& client, bool WateringOn, bool ProcessOn)
{
    client.printf(
        #include <control.html>
    ,WateringOn?"Zalaczony":"Wylaczony",ProcessOn?"Zalaczony":"Wylaczony");

}
#endif // !INCLUDED_CONTROL_H
