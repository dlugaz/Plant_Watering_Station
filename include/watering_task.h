#ifndef INCLUDED_WATERING_TASK_H
#define INCLUDED_WATERING_TASK_H

#include <Arduino.h>

struct Watering_Task
{
  tm start_time;
  int interval_days;
  int duration_seconds;
  float water_amount;
  int pump_power_percent; //0-100%

  void addIntervalToStartTime()
  {
    tm new_time = start_time;
    new_time.tm_mday += interval_days;
    time_t temp = mktime(&new_time);
    tm *new_time_ptr = localtime(&temp);

    start_time = *(new_time_ptr);
  }
};

class Watering_Tasks
{
  std::array<Watering_Task, 20> array;
  int numberoftasks;

public:
  bool Add_Task(Watering_Task task_to_add)
  {
    if (numberoftasks < array.size())
    {
      array[numberoftasks] = task_to_add;
      numberoftasks++;
      Serial.println("Task added");
      return true;
    }
    Serial.println("Cant add task");
    return false;
  }
  bool Delete_Task(int numberToDelete)
  {
    if (numberToDelete <= numberoftasks)
    {
      Serial.println(array.size());
      for (size_t i = 0; i < array.size(); i++)
      {
        if (i >= numberToDelete)
        {
          if ((i + 1) < array.size())
            array[i] = array[i + 1];
          else
            array[i] = Watering_Task();
        }
      }

      numberoftasks--;
      if (numberoftasks < 0)
        numberoftasks = 0;
      Serial.println("Task deleted");
      return true;
    }
    return false;
  }
  int Count()
  {
    return numberoftasks;
  }

  bool isTaskDue(int task_number)
  {
    // Serial.print("isTaskDue ");Serial.println(task_number);
    if (task_number < numberoftasks)
    {
      tm current_time = {0};
      getLocalTime(&current_time);
      tm execution_time = array[task_number].start_time;
      execution_time.tm_mday = execution_time.tm_mday;
      time_t time_ex = mktime(&execution_time);
      // Serial.printf("time_ex %ld ",time_ex);
      time_t time_cur = mktime(&current_time);
      // Serial.printf("time_cur %ld ",time_cur);
      // double out = difftime(time_ex,time_cur);
      // Serial.print(" difftime says:");
      // Serial.println(out);
      bool isdue = (time_cur > time_ex);
      // Serial.printf("Is due %d \n",isdue);
      return isdue;
    }
    return false;
  }

  int isAnyTaskDue()
  {
    for (int i = 0; i < numberoftasks; i++)
    {
      if (isTaskDue(i))
      {
        Serial.printf("Task is due %d", i);
        return i;
      }
    }
    return -1;
  }
  Watering_Task get_task(int index)
  {
    return array[index];
  }

  Watering_Task &operator[](int index)
  {
    return array[index];
  }
};

#endif // !INCLUDED_WATERING_TASK_H
