/*
 * rtos.hpp
 * Author: Eyal Cohen
 *
 * A C++ wrapper around some freeRTOS functions
 */


#ifndef __RTOS_HPP__
#define __RTOS_HPP__

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"


class RTOS {
  public:
    RTOS() : length(0) {
      memset(tasks, 0, sizeof(tasks));
    }

    enum { MaxTasks = 4 };

    enum State {
      Ready = 0,
      Running = 1,
      Blocked = 2,
      Suspended = 3,
      Deleted = 4
    };

    struct Tasks {
      TaskHandle_t handle;
      uint16_t stack;
    };

    struct TaskStats {
      const char* name;
      State state;
      int16_t stackUsed;
      uint16_t stack;
    };

    bool createTask(void (*fcn)(void*), const char* name, uint32_t stack,
                    uint32_t priority) {
      if (length == MaxTasks)
        return false;

      if (!fcn || !name || stack <= 0)
        return false;

      xTaskCreate(fcn, name, stack, NULL, priority, &tasks[length].handle);
      tasks[length].stack = stack;
      length++;
      return true;
    }

    static void sleep_ms(uint32_t ms) {
      vTaskDelay(ms * configTICK_RATE_HZ / 1000);
    }

    void getTaskStats(TaskStats* stats, uint8_t len) const {
      for (uint32_t i = 0; i < len; i++) {
        stats[i].name = pcTaskGetTaskName(tasks[i].handle);
        stats[i].state = (State)eTaskGetState(tasks[i].handle);
        stats[i].stack = tasks[i].stack;
        stats[i].stackUsed = tasks[i].stack -
                              uxTaskGetStackHighWaterMark(tasks[i].handle);
      }
    }

    uint8_t count() const { return length; }

  private:
    Tasks tasks[MaxTasks];
    uint8_t length;

};

#endif
