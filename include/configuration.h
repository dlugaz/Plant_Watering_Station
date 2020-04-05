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
    // Watering_Tasks tasks_array;
}; 
class Settings
{
    std::mutex mutex;
    Config configuration;
    int config_crc;

public:
    Config get_Config()
    {
        std::lock_guard<std::mutex> lock(mutex);
        return configuration;
    }
    void set_Config(Config value)
    {
        std::lock_guard<std::mutex> lock(mutex);
        configuration = value;
    }

    uint8_t calc_crc8(const uint8_t *buffer, size_t len)
    {
        Serial.println("Calculating CRC");
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
        return (sizeof(Config)+sizeof(config_crc)+1);
    }
    void load_settings()
    {
        std::lock_guard<std::mutex> lock(mutex);

        Serial.println("Load2");
        try
        {
            if (!SPIFFS.begin(true))
                throw "SPIFFS cannot be mounted";

            File file = SPIFFS.open(settings_filename, FILE_READ);
            if (!file)
                throw "Cannot open settings file";
            Serial.println(file.position());

            size_t file_size = file.size();
            Serial.printf("Filesize %d should be %d \n",file_size,calc_buffer_size());
            if(file_size<calc_buffer_size()) throw "Invalid file size";
            uint8_t buffer[file_size] = {0};

            size_t read_bytes = file.read(buffer, sizeof(buffer));
            if (read_bytes != sizeof(buffer))
                throw "Unsuccessful load";
            int read_crc = buffer[sizeof(Config)];

            int crc = calc_crc8(buffer, sizeof(Config));
            if (read_crc != crc)
                throw "Invalid CRC";

            memcpy(&configuration, buffer, sizeof(buffer));
            file.close();

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
        std::lock_guard<std::mutex> lock(mutex);
        Config backup = configuration;
        File settings_file;

        try
        {
            if (!SPIFFS.begin(true))
                throw "SPIFFS cannot be mounted";

            settings_file = SPIFFS.open(settings_filename, FILE_WRITE);
            if (!settings_file)
                throw "Cannot open settings file";

            uint8_t buffer[calc_buffer_size()] = {0};
            //put configuration info buffer
            memcpy(buffer, &configuration, sizeof(Config));
            //calculate crc
            config_crc = calc_crc8(buffer, sizeof(Config));
            //put crc into buffer
            buffer [sizeof(Config)] = config_crc ;
            size_t written_bytes = settings_file.write(buffer, sizeof(buffer));
            if (written_bytes != sizeof(buffer))
                throw "Unsuccessful save";

            Serial.println("Save successful");
            Serial.println(written_bytes);

        }
        catch (const char *error)
        {
            Serial.println(error);
        }
            if (!settings_file) settings_file.close();
            SPIFFS.end();

    }
    void reset_settings()
    {
        SPIFFS.remove(settings_filename);
    }
}settings;
#endif // !INCLUDE_CONFIGURE_H
