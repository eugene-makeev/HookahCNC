/*
 * Config.h
 *
 *  Created on: May 28, 2020
 *      Author: user
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define VERSION_STR "HookahCNC Firmware Version 1.0.0"

#define USE_FAST_ACCEL_STEPPER_LIB (0)
#define ENABLE_SERIAL_INTERFACE (0)
#define LOOPBACK_PIN_ANLOG      (0)

// Config
#define MICROSTEPS_PER_REVOLUTION		(18800)

// Buttons
#define START_BUTTON_PIN    (5)

// ROTATION AXIS
#define ROT_AXIS_STEP_PIN		  (10)
#define ROT_AXIS_STEP_ACTIVE  (HIGH)
#define ROT_AXIS_DIR_PIN		  (13)
#define ROT_AXIS_DIR_ACTIVE   (HIGH)
#define ROT_AXIS_EN_PIN			  (14)
#define ROT_AXIS_EN_ACTIVE    (HIGH)
#define ROT_AXIS_MAX_SPEED		(10000.0) // microsteps per second
#define ROT_AXIS_ACCELERATION	(ROT_AXIS_MAX_SPEED * 2)

// puncher servo
#define SERVO0_PIN          (6)
#define SERVO0_PULSE_WIDTH  (100)  // ms
#define SERVO0_ACTIVE       (HIGH)

// loopback pin for end of puncher servo cycle
#if (LOOPBACK_PIN_ANALOG == 1)
    #define LOOPBACK_PIN        (A5)
    #define ADC_RESOLUTION      0.0049 // volts per item
    #define LOOPBACK_ACTIVE_THRESHOLD     (2.5) //volts
#else
    #define LOOPBACK_PIN        (7)
    #define LOOPBACK_ACTIVE     (LOW)
#endif
#define LOOPBACK_TIMEOUT    (3000) // ms

// puncher angle shifter 
#define SERVO1_PIN          (9)
#define SERVO1_PULSE_WIDTH  (500)  // ms
#define SERVO1_ACTIVE       (HIGH)

// divisor selector pins
#define SELECTOR_BIT0_PIN   (0)
#define SELECTOR_BIT1_PIN   (2)
#define SELECTOR_BIT2_PIN   (4)
#define SELECTOR_BIT3_PIN   (3)

#endif /* CONFIG_H_ */
