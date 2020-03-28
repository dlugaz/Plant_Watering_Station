#ifndef INCLUDED_CONFIGURE_H
#define INCLUDED_CONFIGURE_H

#include <WiFiClient.h>

void Print_Configure_Site(WiFiClient& client, float Tank_Volume)
{
    client.printf(
        #include <configure.html>
    ,Tank_Volume);

}

#endif // !INCLUDE_CONFIGURE_H

