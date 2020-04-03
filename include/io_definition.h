   #ifndef INCLUDED_IO_DEFINITION_H
   #define INCLUDED_IO_DEFINITION_H
      
   #include "newping.h"
   #include "SimpleButton.h"

    #define START_BUTTON_PIN 5
    #define PUMP1_PIN 4
    #define LED 2
    #define LEVEL_ECHO_PIN 18
    #define LEVEL_TRIGGER_PIN LEVEL_ECHO_PIN
    #define MAX_DISTANCE 60

   NewPing distanceToWater(LEVEL_TRIGGER_PIN,LEVEL_ECHO_PIN,MAX_DISTANCE);
   simplebutton::ButtonPullup StartButton(START_BUTTON_PIN);
   void Start_Button_Holding_function()
   {
      preferences.clear();
      Serial.println("Memory has been reset");
   }
   void Start_Button_Pressed_function()
   {
      Serial.println("Button pressed");
      current_status.watering_on = !current_status.watering_on;
   }

    void setup_IO()
    {
       //Setup IO
      //Built in LED 
      pinMode(LED,OUTPUT);
      //Pump Output
      pinMode(PUMP1_PIN,OUTPUT);
      //setup pwm for pump
      
      // use first channel of 16 channels (started from zero)
      #define PUMP_PWM_CHANNEL     0
      // use 13 bit precission for LEDC timer
      #define PWM_TIMER_13_BIT  13
      // LEDC base frequency
      #define PWM_BASE_FREQ     25000

      ledcSetup(PUMP_PWM_CHANNEL, PWM_BASE_FREQ, PWM_TIMER_13_BIT);
      ledcAttachPin(PUMP1_PIN, PUMP_PWM_CHANNEL);

      //Attach events to button
      StartButton.setOnHolding(Start_Button_Holding_function,10000);
      StartButton.setOnPushed(Start_Button_Pressed_function);

    }
    
   #endif // !INCLUDED_IO_DEFINITION_H

