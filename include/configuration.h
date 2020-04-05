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
const char CRC_filename[] = "/CRC";


class Settings
{
    xSemaphoreHandle config_mutex;

    Settings()
    {
        config_mutex = xSemaphoreCreateMutex(); 
        if(config_mutex) xSemaphoreGive(config_mutex);
    }
};

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


} configuration;
int calc_crc8(const char *buffer, size_t len)
{
    Serial.println("Calculating CRC");
    long out = 0;
    for (int i = 0; i < len; i++)
    {
        out = (buffer[i] + out) % 255;
    }
    Serial.println(out);
    return out;
}
void load_settings()
{
    Serial.println("Load2");
    try
    {
        if (!SPIFFS.begin(true))
            throw "SPIFFS cannot be mounted";

        File settings_file = SPIFFS.open(filename, FILE_READ);
        if (!settings_file)
            throw "Cannot open settings file";

        uint8_t buffer[sizeof(Config)] = {0};

        size_t read_bytes = settings_file.read(buffer, sizeof(buffer));
        if (read_bytes != sizeof(buffer))
            throw "Unsuccessful load";
        int crc = calc_crc8((char *)buffer, sizeof(buffer));

        memcpy(&configuration, buffer, sizeof(buffer));
        settings_file.close();

        Serial.println("Load successful");
        Serial.println(read_bytes);

        SPIFFS.end();
    }
    catch (const char *error)
    {
        Serial.println(error);
    }
}
void save_settings()
{
    Serial.println("Save2");
    try
    {
        if (!SPIFFS.begin(true))
            throw "SPIFFS cannot be mounted";

        File settings_file = SPIFFS.open(filename, FILE_WRITE);
        if (!settings_file)
            throw "Cannot open settings file";
        uint8_t buffer[sizeof(Config)] = {0};
        memcpy(buffer, &configuration, sizeof(configuration));
        size_t written_bytes = settings_file.write(buffer, sizeof(buffer));
        if (written_bytes != sizeof(buffer))
            throw "Unsuccessful save";

        int crc = calc_crc8((char *)buffer, sizeof(buffer));
        settings_file.close();

        Serial.println("Verification");
        Config backup = configuration;
        load_settings();
        Serial.println(backup.Wifi_Station_Name);
        Serial.println(configuration.Wifi_Station_Name);
        if (backup.Wifi_Station_Name != configuration.Wifi_Station_Name)
            throw "data corrupted";
        Serial.println("Save successful");
        Serial.println(written_bytes);

        SPIFFS.end();
    }
    catch (const char *error)
    {
        Serial.println(error);
    }
}
void reset_settings()
{
    SPIFFS.remove(filename);
}

#endif // !INCLUDE_CONFIGURE_H
