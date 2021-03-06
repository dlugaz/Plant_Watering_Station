#ifndef INCLUDED_IO_DEFINITION_H
#define INCLUDED_IO_DEFINITION_H

#include "newping.h"
#include "SimpleButton.h"
#include "configuration.h"

#define START_BUTTON_PIN 5
#define PUMP1_PIN 4
#define LED 2
#define LEVEL_ECHO_PIN 18
#define LEVEL_TRIGGER_PIN 19  
#define MAX_DISTANCE 60
void analogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255)
{
   // calculate duty, 8191 from 2 ^ 13 - 1
   uint32_t duty = (8191 / valueMax) * min(value, valueMax);

   // write duty to LEDC
   ledcWrite(channel, duty);
}

NewPing distanceToWater(LEVEL_TRIGGER_PIN, LEVEL_ECHO_PIN, MAX_DISTANCE);
simplebutton::ButtonPullup StartButton(START_BUTTON_PIN);
void Start_Button_Holding_function()
{
   settings.reset_settings();
   Serial.println("Memory has been reset");
   esp_restart();
}
void Start_Button_Released_function()
{
   Serial.println("Button pressed");
   current_status.watering_on = !current_status.watering_on;
}

void setup_IO()
{
   //Setup IO
   //Built in LED
   pinMode(LED, OUTPUT);
   //Pump Output
   pinMode(PUMP1_PIN, OUTPUT);

//setup pwm for pump

// use first channel of 16 channels (started from zero)
#define PUMP_PWM_CHANNEL 0
// use 13 bit precission for LEDC timer
#define PWM_TIMER_13_BIT 13
// LEDC base frequency
#define PWM_BASE_FREQ 25000

   ledcSetup(PUMP_PWM_CHANNEL, PWM_BASE_FREQ, PWM_TIMER_13_BIT);
   ledcAttachPin(PUMP1_PIN, PUMP_PWM_CHANNEL);

   //Attach events to button
   StartButton.setOnHolding(Start_Button_Holding_function, 10000);
   StartButton.setOnReleased(Start_Button_Released_function);
}

#endif // !INCLUDED_IO_DEFINITION_H
