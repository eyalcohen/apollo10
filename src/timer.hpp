/*
 * timer.hpp
 * Author: Eyal Cohen
 *
 * Wraps the driverlib timer class, providing some utility functions to
 * start timers
 */

#ifndef __TIMER_HPP__
#define __TIMER_HPP__

#include "inc/hw_memmap.h"
#include "driverlib/timer.h"
#include "driverlib/sysctl.h"

class Timer {
  public:

    enum Timers {
      Timer0 = TIMER0_BASE,
      Timer1 = TIMER1_BASE,
      Timer2 = TIMER2_BASE
    };

    static void initTimer(Timers timer) {
      uint32_t periphEnable;
      switch (timer) {
        case Timer0:
          periphEnable = SYSCTL_PERIPH_TIMER0;
          break;
        case Timer1:
          periphEnable = SYSCTL_PERIPH_TIMER1;
          break;
        case Timer2:
          periphEnable = SYSCTL_PERIPH_TIMER2;
          break;
        default:
          return;

      }
      MAP_SysCtlPeripheralEnable(periphEnable);
      MAP_TimerConfigure(timer, TIMER_CFG_PERIODIC_UP);
      MAP_TimerEnable(timer, TIMER_BOTH);
    }

    static uint32_t getTicks(Timers timer) {
      return MAP_TimerValueGet(timer, TIMER_A);
    }

    static uint64_t getTicks64(Timers timer) {
      return MAP_TimerValueGet64(timer);
    }

};


#endif
