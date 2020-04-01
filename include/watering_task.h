#ifndef INCLUDED_WATERING_TASK_H
#define INCLUDED_WATERING_TASK_H

#include <Arduino.h>

struct Watering_Task
{
  tm start_time;
  int interval_days;
  int duration_seconds;
  float water_amount;
  uint8_t pump_power_percent; //0-100% 
};
#endif // !INCLUDED_WATERING_TASK_H

