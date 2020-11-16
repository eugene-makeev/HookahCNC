#include <Arduino.h>
#include "Config.h"
#include "Cmd.h"

#if (ENABLE_SERIAL_INTERFACE == 1)
SerialCommand SCmd;
extern void cycleStart();
extern long microsteps_per_revolution;
extern int rot_axis_speed;
extern int rot_axis_accel;

void inline sendAck()
{
	Serial.println("OK");
}

void inline sendError()
{
	Serial.println("Invalid command or argument");
}

void setConfiguration()
{
	char *arg, *val1, *val2;
	arg = SCmd.next();
	val1 = SCmd.next();
  val2 = SCmd.next();
  
	if ((arg != NULL) && (val1 != NULL) && (val2 != NULL))
	{
		long steps = atoi(arg);
		int spd = atoi(val1);
    int accel = atoi(val2);

    microsteps_per_revolution = steps;
    rot_axis_speed = spd;
    rot_axis_accel = accel;
    
    // TODO: save values
    
		sendAck();
	}
	else
	{
		sendError();
	}
}

void sendVersion(void)
{
	Serial.println(VERSION_STR);
}

void unrecognized(const char *command)
{
	sendError();
}

void makeComInterface(void)
{
	SCmd.addCommand("v", sendVersion);
	SCmd.addCommand("sc", setConfiguration);
	SCmd.addCommand("sm", cycleStart);
	SCmd.setDefaultHandler(unrecognized);
}
#endif
