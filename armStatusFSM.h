/*
 * armStatusFSM.h
 *
 *  Created on: Mar 18, 2021
 *      Author: Connor Bondi
 */

#ifndef ARMSTATUSFSM_H_
#define ARMSTATUSFSM_H_

/* I2C slave address */
#define CONTROLLER_ADDR            0x40

/* Servo Values */
#define SERVO_FREQ 50
#define SERVOMIN  150
#define SERVOMAX  600

void *armStatus(void *arg0);
int createArmStatusThread(int threadStackSize, int prio);

#endif /* ARMSTATUSFSM_H_ */
