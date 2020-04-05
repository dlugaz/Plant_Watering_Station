#ifndef TASKS_FUNCTIONS_H
#define TASKS_FUNCTIONS_H
#include <Arduino.h>
#include "site_handlers.h"
#include "io_definition.h"

void webServer_function(void *parameter)
{
    while (true)
    {
        webServer.handleClient();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

uint16_t Status_LED_frequency_ms = 1000;

void Status_LED_function(void *parameter)
{

    while (true)
    {
            digitalWrite(LED, !digitalRead(LED));
            vTaskDelay(pdMS_TO_TICKS(Status_LED_frequency_ms));
    }
}
AsyncDelay Pump_On_Timer;

void logic_function(void *parameter)
{
    while (true)
    {
        // logic
        if (current_status.watering_on)
        {
            Serial.println("Watering On");
            current_status.pump_on = true;
            analogWrite(PUMP_PWM_CHANNEL, map(current_status.pump_speed, 0, 100, 170, 255), 255);
        }
        else
        {
            Serial.println("Watering Off");
            current_status.pump_on = false;
            analogWrite(PUMP_PWM_CHANNEL, 0);
        }

        if (current_status.tasks_on)
        {
            Serial.println("Tasks On");
            tm current_time = {0};
            getLocalTime(&current_time);
            //check if any task is due
            int due_task_number = configuration.tasks_array.isAnyTaskDue();
            if (due_task_number)
            {
                Serial.println("Some task is due");
                Serial.println(due_task_number);
                //make alias for shorter name
                // Watering_Task due_task = configuration.tasks_array[due_task_number];
                Watering_Task due_task = configuration.tasks_array.get_task(due_task_number);
                //start pump for configured time
                if (!current_status.watering_on)
                {
                    Serial.println("Starting Pumps and timer");
                    Serial.println(due_task.duration_seconds);
                    Pump_On_Timer.start(due_task.duration_seconds, AsyncDelay::units_t::MILLIS);
                    current_status.watering_on = true;
                    current_status.pump_speed = due_task.pump_power_percent;
                }
                //Task has been fulfiled
                if (Pump_On_Timer.isExpired() && current_status.watering_on)
                {
                    Serial.println("Task fulfilled");
                    current_status.watering_on = false;
                    due_task.start_time = current_time; //refresh last start time
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

#endif // !TASKS_FUNCTIONS_H
