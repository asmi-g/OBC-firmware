#ifndef _VNHELPER_H_
#define _VNHELPER_H_

void packetFoundHandler(void *userData, VnUartPacket *packet, size_t runningIndexOfPacketStart);
void UserUart_initialize(void);
bool UserUart_checkForReceivedData(char* buffer, size_t bufferSize, size_t* numOfBytesReceived);
void UserUart_mockReceivedData(char* buffer, size_t bufferSize);
void UserUart_sendData(char *data, size_t size);

#endif