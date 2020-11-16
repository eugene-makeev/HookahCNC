/*
 * Functions.h
 *
 *  Created on: May 28, 2020
 *      Author: user
 */

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include "SerialCommand.h" //nice lib from Stefan Rado, https://github.com/kroimon/Arduino-SerialCommand

extern SerialCommand SCmd;

void makeComInterface(void);

void sendAck();
#endif /* FUNCTIONS_H_ */
