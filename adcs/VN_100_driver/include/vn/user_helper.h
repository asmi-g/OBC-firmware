#ifndef _VNHELPER_H_
#define _VNHELPER_H_

#include "vn/protocol/upack.h"
#include "vn/protocol/upackf.h"

/* Common commands */
void RESFactory();
void pauseASYNC();
void resumeASYNC();
void tare();
void readModelNumREG();

/* UART USER FUNCTIONS */
void UserUart_initialize(void);
bool UserUart_checkForReceivedData(char* buffer, size_t* numOfBytesReceived);
void UserUart_sendData(char *data, size_t size);

#endif