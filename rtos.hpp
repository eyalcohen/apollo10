/*
 * rtos.hpp
 * Author: Eyal Cohen
 *
 * A wrapper around the RTOS that contains some task handle state information
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

    bool createTask(void (*fcn)(void*), const char* name, uint32_t stack,
                    uint32_t priority) {
      if (length == maxTasks)
        return false;

      if (!fcn || !name || stack <= 0)
        return false;

      xTaskCreate(fcn, name, stack, NULL, priority, tasks[length]);
      length++;
      return true;
    }

    static void sleep_ms(uint32_t ms) {
      vTaskDelay(ms * configTICK_RATE_HZ / 1000);
    }

  private:

    enum { maxTasks = 4 };
    TaskHandle_t* tasks[maxTasks];
    uint8_t length;

};

#endif
