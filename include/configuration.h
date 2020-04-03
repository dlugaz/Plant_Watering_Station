#ifndef INCLUDED_CONFIGURATION_H
#define INCLUDED_CONFIGURATION_H
#include "watering_task.h"

const char key_tank_volume[] = "TankVolume";
const char key_tank_Height[] = "TankHeight";
const char key_L_per_cm[] = "LperCm";
const char key_wifi_name[] = "WifiName";
const char key_wifi_password[] = "WifiPass";
const char key_device_ip[] = "DeviceIp";
const char key_configuration_struct[] = "ConfigStruct";
const char key_tasks_array[] = "TasksArray";
const char key_numberoftasks[] = "NumberOfTasks";

Preferences memory_access;

struct Config
{

public:
    tm Last_Watering_Timestamp;
    IPAddress Device_Ip;
    float Tank_Volume;
    float Tank_Height_cm;
    float L_per_cm;
    String Wifi_Station_Name;
    String Wifi_Station_Password;
    // std::array<Watering_Task, 20> tasks_array;
    // int numberoftasks;
    Watering_Tasks tasks_array;
// void save_tasks()
// {
//     memory_access.begin("settings");
//     memory_access.putBytes(key_tasks_array, &tasks_array, sizeof(tasks_array));
//     memory_access.putInt(key_numberoftasks, numberoftasks);
//     memory_access.end();

// }
// void load_tasks()
// {
//     memory_access.begin("settings");
//     memory_access.getBytes(key_tasks_array, &tasks_array, sizeof(tasks_array));
//     numberoftasks = memory_access.getInt(key_numberoftasks, numberoftasks);
//     memory_access.end();

// }
} configuration;


void save_settings()
{
    memory_access.begin("settings");
    // memory_access.putString(key_wifi_name, Wifi_Station_Password);
    // memory_access.putString(key_wifi_password, Wifi_Station_Password);
    // memory_access.putFloat(key_tank_volume, Tank_Volume);
    // memory_access.putFloat(key_tank_Height, Tank_Height_cm);
    // memory_access.putFloat(key_L_per_cm, L_per_cm);
    // memory_access.putBytes(key_tasks_array, &tasks_array, sizeof(tasks_array));
    // memory_access.putString(key_device_ip, Device_Ip.toString());
    // memory_access.putInt(key_numberoftasks,numberoftasks);
    memory_access.putBytes(key_configuration_struct, &configuration, sizeof(configuration));
    //verify
    Config test;
    memory_access.getBytes(key_configuration_struct, &test, sizeof(test));
    Serial.println(configuration.Wifi_Station_Name);
    Serial.println(test.Wifi_Station_Name);
    memory_access.end();
}
void load_settings()
{
    memory_access.begin("settings");
    // Wifi_Station_Password = memory_access.getString(key_wifi_name);
    // Wifi_Station_Password = memory_access.getString(key_wifi_password);
    // Tank_Volume = memory_access.getFloat(key_tank_volume);
    // Tank_Height_cm = memory_access.getFloat(key_tank_Height);
    // L_per_cm = memory_access.getFloat(key_L_per_cm);
    // memory_access.getBytes(key_tasks_array, &tasks_array, sizeof(tasks_array));
    // Device_Ip.fromString (memory_access.getString(key_device_ip));
    // numberoftasks = memory_access.getInt(key_numberoftasks,numberoftasks);
    memory_access.getBytes(key_configuration_struct, &configuration, sizeof(configuration));
    memory_access.end();
}



#endif // !INCLUDE_CONFIGURE_H
