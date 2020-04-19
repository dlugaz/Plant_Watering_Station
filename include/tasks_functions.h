#ifndef TASKS_FUNCTIONS_H
#define TASKS_FUNCTIONS_H
#include <Arduino.h>
#include "site_handlers.h"
#include "io_definition.h"

bool webServer_run = true;
void webServer_function(void *parameter)
{
    while (webServer_run)
    {
        webServer.handleClient();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

uint16_t Status_LED_frequency_ms = 1000;

bool Status_LED_run = true;
void Status_LED_function(void *parameter)
{
    while (Status_LED_run)
    {
            digitalWrite(LED, !digitalRead(LED));
            vTaskDelay(pdMS_TO_TICKS(Status_LED_frequency_ms));
    }
}


//schedule level measurement

RunningAverage Tank_Level_Average (5);
void Tank_Level_Measurement_function()
{
  Config configuration = settings.get_Config();
  float L_per_cm = ((float)configuration.Tank_Volume/(float)configuration.Tank_Height_cm);
  unsigned long measurement = distanceToWater.ping_median();
  // Serial.println(measurement);
  // measurement = NewPing::convert_cm(measurement);
  float measured_distance = (float)measurement / US_ROUNDTRIP_CM;
  // Serial.println(measured_distance);
  // current_status.water_level_L =  (configuration.Tank_Height_cm - measured_distance)*L_per_cm;
  Tank_Level_Average.addValue((configuration.Tank_Height_cm - measured_distance)*L_per_cm);
  current_status.water_level_L =  Tank_Level_Average.getAverage();

  Serial.printf("Water level %f \n",current_status.water_level_L);
}
Ticker Tank_Level_Measurement(Tank_Level_Measurement_function,1000,0,MILLIS);

//schedule water flow measurement every 5 second
float last_water_level_L = 0.0; 
void Tank_Flow_Measurement_function();
Ticker Tank_Flow_Measurement(Tank_Flow_Measurement_function,10000,0,MILLIS);

long time_of_last_flow_measurement = 0;
void Tank_Flow_Measurement_function()
{
  // Serial.printf("Tank_Flow: last %f, cur %f \n",last_water_level_L,current_status.water_level_L);
  //calculate time from last measurement
  uint32_t elapsed = millis() - time_of_last_flow_measurement;
  // Serial.printf("elapsed time %d", elapsed);
  float time_s = (float)elapsed/1000.0f;
  // Serial.printf("Flow time %f \n",time_s);
  //calculate flow as difference in levels divided by time
  if(last_water_level_L!=0.0) // discard first measurement
    current_status.water_flow_L_per_min = (last_water_level_L - current_status.water_level_L)*(60.0f/time_s);
  //save water level for next iteration
  last_water_level_L = current_status.water_level_L;
  Serial.print("Flow ");Serial.println(current_status.water_flow_L_per_min);
  time_of_last_flow_measurement = millis();
}

extern void setup_Connectivity();

void checkConnectivity_function()
{
  Config configuration = settings.get_Config();
  Serial.println("Checking connectivity");
  Serial.printf("Mode %d Connected %d, WifiStationName %s isEmpty? %d \n",WiFi.getMode(),WiFi.isConnected(),configuration.Wifi_Station_Name.c_str(),configuration.Wifi_Station_Name.isEmpty());
  if(configuration.Wifi_Station_Name && !configuration.Wifi_Station_Name.isEmpty()
  &&(WiFi.getMode() == WIFI_MODE_STA && !WiFi.isConnected()))
    Status_LED_frequency_ms = 200;
  else 
    Status_LED_frequency_ms = 1000;
}
Ticker checkConnectivity(checkConnectivity_function,60000,0,MILLIS);

bool logic_run = true;
void logic_function(void *parameter)
{
    Config configuration;
    AsyncDelay Pump_On_Timer;

    int pump_speed_ramp = 0;
    const int pump_ramp_slope = 10;
    unsigned long pump_started_time = 0;

    //Start timing tasks
    Tank_Flow_Measurement.start();
    Tank_Level_Measurement.start();
    checkConnectivity.start();

    while (logic_run)
    {
        Config configuration = settings.get_Config();

        // logic
        if (current_status.watering_on)
        {
            Serial.println("Watering On");
            current_status.pump_on = true;
            
            if(pump_speed_ramp < current_status.pump_speed) pump_speed_ramp += pump_ramp_slope;
            if(pump_speed_ramp > current_status.pump_speed) pump_speed_ramp = current_status.pump_speed;

            analogWrite(PUMP_PWM_CHANNEL, map(pump_speed_ramp, 0, 100, 170, 255), 255);
            current_status.water_pumped = current_status.water_amount_when_started - current_status.water_level_L;
            current_status.pump_on_time_s = (millis() - pump_started_time)/1000;
            Serial.printf("Pumped water %f \n", current_status.water_pumped);
            Serial.printf("Pump on for %d s \n", current_status.pump_on_time_s);

        }
        else
        {
            Serial.println("Watering Off");
            current_status.pump_on = false;
            current_status.water_amount_when_started = current_status.water_level_L;
            current_status.water_pumped = 0;
            pump_speed_ramp = 0;
            pump_started_time = millis();
            analogWrite(PUMP_PWM_CHANNEL, 0);
        }

        if (current_status.tasks_on)
        {
            Serial.println("Tasks On");
            tm current_time = {0};
            getLocalTime(&current_time);
            //check if any task is due
            int due_task_number = configuration.tasks_array.isAnyTaskDue();
            if (due_task_number >= 0)
            {
                Serial.println("Some task is due");
                Serial.println(due_task_number);
                //make alias for shorter name
                Watering_Task due_task = configuration.tasks_array[due_task_number];
                // Watering_Task due_task = configuration.tasks_array.get_task(due_task_number);
                //start pump for configured time
                if (!current_status.watering_on)
                {
                    Serial.println("Starting Pumps and timer");
                    Serial.println(due_task.duration_seconds);
                    if(due_task.duration_seconds!=0)
                        Pump_On_Timer.start(due_task.duration_seconds*1000, AsyncDelay::units_t::MILLIS);
                    current_status.watering_on = true;
                    current_status.pump_speed = due_task.pump_power_percent;
                }

                //Task has been fulfiled
                if (
                    ((Pump_On_Timer.isExpired() || 
                    (due_task.water_amount!=0 &&(current_status.water_pumped > due_task.water_amount))) && current_status.watering_on)
                    )
                {
                    Serial.println("Task fulfilled");
                    current_status.watering_on = false;
                    // due_task.start_time = current_time; //refresh last start time
                    // calculate new start time
                    tm new_time = due_task.start_time;
                    new_time.tm_mday += due_task.interval_days;
                    time_t temp = mktime(&new_time);
                    tm* new_time_ptr = localtime(&temp);

                    configuration.tasks_array[due_task_number].start_time = *(new_time_ptr);
                    settings.save_settings(configuration);
                }
            }
        }
        //handle all timing tasks events
        Tank_Level_Measurement.update();
        Tank_Flow_Measurement.update();
        //handle all buttons
        StartButton.update();

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

#endif // !TASKS_FUNCTIONS_H
