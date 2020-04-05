#ifndef INCLUDED_CONFIGURATION_H
#define INCLUDED_CONFIGURATION_H
#include "watering_task.h"
#include <Preferences.h>

const char key_tank_volume[] = "TankVolume";
const char key_tank_Height[] = "TankHeight";
const char key_L_per_cm[] = "LperCm";
const char key_wifi_name[] = "WifiName";
const char key_wifi_password[] = "WifiPass";
const char key_device_ip[] = "DeviceIp";
const char key_configuration_struct[] = "ConfigStruct";
const char key_tasks_array[] = "TasksArray";
const char key_numberoftasks[] = "NumberOfTasks";
const char key_crc[] = "crc";
const char key_settings[] = "settings";
const char settings_filename[] = "/settings.txt";
const char CRC_filename[] = "/CRC.txt";
#include <mutex>
 struct Config
{
    tm Last_Watering_Timestamp;
    IPAddress Device_Ip;
    float Tank_Volume;
    float Tank_Height_cm;
    float L_per_cm;
    String Wifi_Station_Name;
    String Wifi_Station_Password;
    Watering_Tasks tasks_array;
}; 
class Settings
{
    std::mutex mutex;
    Config configuration;
    int config_crc;
    Preferences memory_access;

public:

    Config get_Config() 
    {
        std::lock_guard<std::mutex> lock(mutex);
        // Serial.println("Get Config");
        return configuration;
    }
    void set_Config(Config value) 
    {
        std::lock_guard<std::mutex> lock(mutex);
        // Serial.println("Set Config");
        configuration = value;
    }

    uint8_t calc_crc8(const uint8_t *buffer, size_t len)
    {
        Serial.print("Calculating CRC ");
        uint8_t out = 0;
        for (int i = 0; i < len; i++)
        {
            out = (buffer[i] + out) % 255;
        }
        Serial.println(out);
        return out;
    }
    size_t calc_buffer_size()
    {
        return (sizeof(Config)+1);
    }
    void load_settings() 
    {
        std::lock_guard<std::mutex> lock(mutex);

        Serial.println("Load");
        try
        {
            if(!memory_access.begin(key_settings))throw "Unable to access memory";

            uint8_t buffer[calc_buffer_size()] = {0};
            size_t read_bytes = memory_access.getBytes(key_configuration_struct,&buffer,sizeof(buffer));
            Serial.printf("Read bytes %d \n",read_bytes);
            if(read_bytes != sizeof(buffer)) throw "Invalid bytes read";
            
            int read_crc = memory_access.getUInt(key_crc);
            int crc = calc_crc8(buffer, sizeof(buffer));
            Serial.printf("Read CRC %d \n",read_crc);
            if (read_crc != crc) throw "Invalid CRC";

            memcpy(&configuration, buffer, sizeof(Config));
           
            Serial.println("Load successful");

        }

        catch (const char *error)
        {
            Serial.println(error);
        }
        memory_access.end();
    }
    void save_settings (Config value)
    {
        set_Config(value);
        save_settings();
    }
    void save_settings() 
    {
        Serial.println("Save");
        std::lock_guard<std::mutex> lock(mutex);
        Config backup = configuration;

        try
        {
            if(!memory_access.begin(key_settings))throw "Unable to access memory";

            uint8_t buffer[calc_buffer_size()] = {0};
            //put configuration info buffer
            memcpy(buffer, &configuration, sizeof(Config));
            //calculate crc
            config_crc = calc_crc8(buffer, sizeof(buffer));

            size_t written_bytes = memory_access.putBytes(key_configuration_struct,buffer,sizeof(buffer));
            Serial.printf("Written bytes %d \n",written_bytes);

            if(written_bytes!=sizeof(buffer)) throw "Invalid written bytes";

            memory_access.putUInt(key_crc,config_crc);
            Serial.println("Save successful");

        }
        catch (const char *error)
        {
            Serial.println(error);
        }
            memory_access.end();

    }
    void reset_settings() 
    {
        memory_access.clear();
    }
};
 Settings settings;
#endif // !INCLUDE_CONFIGURE_H
