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

bool logic_run = true;
void logic_function(void *parameter)
{
    Config configuration;
    AsyncDelay Pump_On_Timer;

    int pump_speed_ramp = 0;
    const int pump_ramp_slope = 10;

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
            Serial.printf("Pumped water %f \n", current_status.water_pumped);
        }
        else
        {
            Serial.println("Watering Off");
            current_status.pump_on = false;
            current_status.water_amount_when_started = current_status.water_level_L;
            current_status.water_pumped = 0;
            pump_speed_ramp = 0;
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
                    || !current_status.watering_on)
                {
                    Serial.println("Task fulfilled");
                    current_status.watering_on = false;
                    // due_task.start_time = current_time; //refresh last start time
                    configuration.tasks_array[due_task_number].start_time = current_time;
                    settings.save_settings(configuration);
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

#endif // !TASKS_FUNCTIONS_H
