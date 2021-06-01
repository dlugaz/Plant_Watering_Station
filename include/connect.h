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
String Print_Available_Networks()
{
    String website;
    int n = WiFi.scanNetworks();
    if (n == 0) {
        website += "no networks found";
    } else {
        website +=
            R"(
                <form method="POST">
                <p>Wybierz sieć:</p>)";
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            website += R"(<input type="radio" name="AP_Name" value=")";
            website +=  WiFi.SSID(i);
            website += R"(">)";
            website += WiFi.SSID(i);
            website += R"(<br>)";
        }
        website +=
            R"(
                <br>
                <label for="psw"><b>Hasło </b></label> 
                <input type="password" placeholder="Wpisz hasło"
                       name="psw"> <br>
                <button class="button" type="submit" name="Save">Zapisz</button>
                <br><br>
                </form>
                )";
    }
    return website;
}



#endif // !INCLUDED_CONNECT_H