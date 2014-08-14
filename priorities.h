/*
 * priorities.h - defines priorities and stack sizes for threads
 */

#ifndef __PRIORITIES_H__
#define __PRIORITIES_H__

#define SERIAL_PORT_PRIORITY    1
#define SERIAL_PORT_STACK   100 

#define CLI_PORT_PRIORITY    1
#define CLI_PORT_STACK   200 

#define LED_PORT_PRIORITY    1
#define LED_PORT_STACK    100

#endif // __PRIORITIES_H__
