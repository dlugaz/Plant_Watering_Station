#ifndef TASKS_FUNCTIONS_H
#define TASKS_FUNCTIONS_H
#include <Arduino.h>
#include "site_handlers.h"
#include "io_definition.h"

void webServer_function (void* parameter)
{
    while(true){
        webServer.handleClient();
        vTaskDelay(10);
        }

}

uint16_t Status_LED_frequency_ms = 1000;

void Status_LED_function (void* parameter)
{ 
    long LED_blinking_timer = 0;
    while(true){
        
        if(millis() > LED_blinking_timer){
            LED_blinking_timer = millis() + Status_LED_frequency_ms;
            digitalWrite(LED,!digitalRead(LED));
            vTaskDelay(100);
        }
        

    }
}

#endif // !TASKS_FUNCTIONS_H
