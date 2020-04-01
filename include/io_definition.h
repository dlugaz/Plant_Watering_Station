   #ifndef INCLUDED_IO_DEFINITION_H
   #define INCLUDED_IO_DEFINITION_H
      
    #define START_BUTTON 5
    #define PUMP1_PIN 4
    #define LED 2
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
      // use 5000 Hz as a LEDC base frequency
      #define PWM_BASE_FREQ     5000

      ledcSetup(PUMP_PWM_CHANNEL, PWM_BASE_FREQ, PWM_TIMER_13_BIT);
      ledcAttachPin(PUMP1_PIN, PUMP_PWM_CHANNEL);

      //Start/stop button
      
      pinMode(START_BUTTON,INPUT_PULLUP);
    }
    
   #endif // !INCLUDED_IO_DEFINITION_H

