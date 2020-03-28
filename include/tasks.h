#ifndef INCLUDED_TASKS_H
#define INCLUDED_TASKS_H

#include <WiFiClient.h>

void Print_Tasks_Site(WiFiClient& client)
{
    client.printf(
        #include <tasks.html>
    );

}

#endif // !INCLUDED_TASKS_H

