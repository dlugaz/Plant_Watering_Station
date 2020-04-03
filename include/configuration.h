#ifndef INCLUDED_CONFIGURATION_H
#define INCLUDED_CONFIGURATION_H
#include "watering_task.h"

struct Config
{
tm Last_Watering_Timestamp;
IPAddress Device_Ip;
float Tank_Volume;
float Tank_Height_cm;
float L_per_cm;
String Wifi_Station_Name;
String Wifi_Station_Password;
std::array<Watering_Task,20> tasks_array;
int numberoftasks;

// const char key_tank_volume [] = "TankVolume";
// const char key_wifi_name [] = "WifiName";
// const char key_wifi_password [] = "WifiPass";
// const char key_device_ip [] = "DeviceIp";

// Config()
// {
//   memory_access.begin("settings");
// }

// bool save_settings()
// {
//   memory_access.putString(key_wifi_name,Wifi_Station_Password);
//   memory_access.putString(key_wifi_password,Wifi_Station_Password);
//   memory_access.putFloat(key_tank_volume,Tank_Volume);
//   memory_access.putString(key_device_ip,Device_Ip.toString());
// }
// bool load_settings ()
// {
//   memory_access.getString(key_wifi_name,Wifi_Station_Password);
//   memory_access.getString(key_wifi_password,Wifi_Station_Password);
//   memory_access.getFloat(key_tank_volume,Tank_Volume);
//   memory_access.getString(key_device_ip,Device_Ip.toString());

// }
} configuration;


const char key_tank_volume [] = "TankVolume";
const char key_wifi_name [] = "WifiName";
const char key_wifi_password [] = "WifiPass";
const char key_device_ip [] = "DeviceIp";
const char key_configuration_struct[] ="ConfigStruct";


#endif // !INCLUDE_CONFIGURE_H

