#ifndef INCLUDED_CONFIGURATION_H
#define INCLUDED_CONFIGURATION_H
#include "watering_task.h"
#include <SPIFFS.h>

const char key_tank_volume[] = "TankVolume";
const char key_tank_Height[] = "TankHeight";
const char key_L_per_cm[] = "LperCm";
const char key_wifi_name[] = "WifiName";
const char key_wifi_password[] = "WifiPass";
const char key_device_ip[] = "DeviceIp";
const char key_configuration_struct[] = "ConfigStruct";
const char key_tasks_array[] = "TasksArray";
const char key_numberoftasks[] = "NumberOfTasks";
const char key_success[] = "Success";
const char filename[] = "/settings";


struct Config
{
    private: 

    public:
    tm Last_Watering_Timestamp;
    IPAddress Device_Ip;
    float Tank_Volume;
    float Tank_Height_cm;
    float L_per_cm;
    String Wifi_Station_Name;
    String Wifi_Station_Password;
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


    // bool save()
    // {
    //     Serial.println("saving settings");
    //     memory_access.begin("settings");
    //     memory_access.clear();
    //     memory_access.putString(key_wifi_name, Wifi_Station_Password);
    //     memory_access.putString(key_wifi_password, Wifi_Station_Password);
    //     memory_access.putFloat(key_tank_volume, Tank_Volume);
    //     memory_access.putFloat(key_tank_Height, Tank_Height_cm);
    //     memory_access.putFloat(key_L_per_cm, L_per_cm);
    //     // memory_access.putBytes(key_tasks_array, &tasks_array, sizeof(tasks_array));
    //     memory_access.putString(key_device_ip, Device_Ip.toString());

    //     Serial.println(Wifi_Station_Name);
    //     Serial.println(Wifi_Station_Password);
    //     return false;
    // }

    // bool load()
    // {
    //     Wifi_Station_Password = memory_access.getString(key_wifi_name);
    //     Wifi_Station_Password = memory_access.getString(key_wifi_password);
    //     Tank_Volume = memory_access.getFloat(key_tank_volume);
    //     Tank_Height_cm = memory_access.getFloat(key_tank_Height);
    //     L_per_cm = memory_access.getFloat(key_L_per_cm);
    //     // memory_access.getBytes(key_tasks_array, &tasks_array, sizeof(tasks_array));
    //     Device_Ip.fromString (memory_access.getString(key_device_ip));
    //     Serial.println(Wifi_Station_Name);
    //     Serial.println(Wifi_Station_Password);
    //     return false;
    // }
} configuration;
    void load_settings()
    {
        Serial.println("Load2");
        try{
            if(!SPIFFS.begin(true))throw "SPIFFS cannot be mounted";

            File settings_file = SPIFFS.open(filename,FILE_READ);
            if(!settings_file)throw "Cannot open settings file";
            uint8_t buffer [sizeof(Config)] = {0};
            
            size_t read_bytes = settings_file.read(buffer,sizeof(buffer));
            if(read_bytes != sizeof(buffer))throw "Unsuccessful load";
            memcpy(&configuration,buffer,sizeof(buffer));
            settings_file.close();

            Serial.println("Load successful");
            Serial.println(read_bytes);

        }
        catch (const char* error)
        {
            Serial.println(error);
        }       
    }
    void save_settings()
    {
        Serial.println("Save2");
        try{
            if(!SPIFFS.begin(true))throw "SPIFFS cannot be mounted";

            File settings_file = SPIFFS.open(filename,FILE_WRITE);
            if(!settings_file)throw "Cannot open settings file";
            uint8_t buffer [sizeof(Config)] = {0};
            memcpy(buffer,&configuration,sizeof(configuration));
            size_t written_bytes = settings_file.write(buffer, sizeof(buffer));
            if(written_bytes != sizeof(buffer))throw "Unsuccessful save";
            settings_file.close();
            Serial.println("Verification");
            Config backup = configuration;
            load_settings();
            Serial.println(backup.Wifi_Station_Name);
            Serial.println(configuration.Wifi_Station_Name);
            if(backup.Wifi_Station_Name != configuration.Wifi_Station_Name) throw "data corrupted";
            Serial.println("Save successful");
            Serial.println(written_bytes);
            

        }
        catch (const char* error)
        {
            Serial.println(error);
        }        
    }


// void save_settings()
// {
//     Serial.println("saving settings");
//     preferences.begin("settings");
//     preferences.clear();

//     Serial.println("Writing put bytes");
//     Serial.println(preferences.putBytes(key_configuration_struct, &configuration, sizeof(Config)));
//     // //verify
//     Config test;
//     Serial.println("Verification");
//     size_t got_bytes = preferences.getBytes(key_configuration_struct, &test, sizeof(test));
//     Serial.println(got_bytes);
//     if (got_bytes != sizeof(Config))
//         Serial.println("Wrong data size");
//     Serial.println(configuration.Wifi_Station_Name);
//     Serial.println(test.Wifi_Station_Name);
//     // memory_access.end();
// }
// void load_settings()
// {
//     Serial.println("loading settings");
//     try
//     {
//         if (!preferences.begin("settings"))
//             throw 1;

//         if (!(preferences.getInt(key_success) == 1))
//             throw 2;



//         size_t got_bytes = preferences.getBytes(key_configuration_struct, &configuration, sizeof(Config));
//         Serial.println(got_bytes);
//         if (got_bytes != sizeof(Config))
//         {
//             Serial.println("Wrong data size");
//             throw 3;
//         }
//         Serial.println(configuration.Wifi_Station_Name);
//         Serial.println(configuration.Wifi_Station_Password);
//         Serial.println(configuration.Tank_Volume);
//         Serial.println(configuration.Tank_Height_cm);
//     }
//     catch (int error)
//     {
//         Serial.print("error ");
//     }
// }

#endif // !INCLUDE_CONFIGURE_H
