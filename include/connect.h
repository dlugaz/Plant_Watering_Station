#ifndef INCLUDED_CONNECT_H
#define INCLUDED_CONNECT_H
#include <WiFiClient.h>


// void Print_Available_Networks(WiFiClient& client)
// {
//    // WiFi.scanNetworks will return the number of networks found
//     int n = WiFi.scanNetworks();
//     client.println("scan done");
//     if (n == 0) {
//         client.println("no networks found");
//     } else {
//         client.print(n);
//         client.println(" networks found");
//         for (int i = 0; i < n; ++i) {
//             // Print SSID and RSSI for each network found
//             client.print(i + 1);
//             client.print(": ");
//             client.print(WiFi.SSID(i));
//             client.print(" (");
//             client.print(WiFi.RSSI(i));
//             client.print(")");
//             client.print((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
//             client.println("<br>");
//         }
//     }
// }
void Print_Available_Networks(WiFiClient& client)
{
    String network_list;
    int n = WiFi.scanNetworks();
    if (n == 0) {
        client.println("no networks found");
    } else {
        client.println(
            R"(
                <form method="post">
                <p>Wybierz siec:</p>)");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            client.println( R"(<input type="radio" name="AP_Name" value=")");
            client.println(  WiFi.SSID(i));
            client.println( R"(">)");
            client.println( WiFi.SSID(i));
            client.println( R"(<br>)");
        }
        client.println(
            R"(
                <br>
                <label for="psw"><b>Password</b></label> 
                <input type="password" placeholder="Enter Password"
                       name="psw"> <br>
                <input type="submit" name="Save!" value="Zapisz">
                <br><br>
                <input type="submit" name="Scan!" value="Skanuj">
                </form>
                )");
    }
}



#endif // !INCLUDED_CONNECT_H