#include "vn/util.h"
#include "vn/protocol/upack.h"
#include "vn/protocol/upackf.h"
#include "vn/int.h"
#include "vn/user_helper.h"

#include <string.h>
#include <stdio.h>
#include <sci.h> 

/* Buffer that temporarily stores the data received from the sensor*/
char buffer[256];
/* Holds the number of elements within the received buffer */
size_t numOfBytes;

/* The number of bytes within each respective command buffer */
size_t readVNYMR, writeAsyncFreq;

/* Buffers to hold commands that will be sent to the sensor */
char genReadModelNumberBuffer[256];
char genReadVNYMR[256];
char genReadVNYPR[256];
char genWriteAsyncOutputFrequencyBuffer[256];

/* Holds the current data received */
char yprStr[100], magStr[100], accelStr[100], angularRateStr[100];
char enableStr[100], headingModeStr[100], filteringModeStr[100], tuningModeStr[100];

int main (void) {
	/* Initialize the UART port */
	UserUart_initialize();

	/* Read YPR, magentic, acceleration and angular rate */
    genReadVNYMR = "$VNWRG,06,14*5C\r\n"; /* Basic command to read asyncronously from ASYNC OUPUT register */
	UserUart_sendData(genReadVNYMR, 19);
	/* Continually checking for new UART data and then passing any received data to the 
	VnUartProtocol to build, parse and verify data packets. */
	UserUart_checkForReceivedData(buffer, &numOfBytes);
    if (ErrorReceived(buffer) == 0){
        VnUartPacket packet;
        VnUartPacket_initialize(packet, buffer, numOfBytes);
        vec3f ypr, mag, accel, angularRate;
        VnUartPacket_parseVNYMR(packet, &ypr, &mag, &accel, &angularRate);
        str_vec3f(yprStr, ypr);
        str_vec3f(magStr, mag);
        str_vec3f(accelStr, accel);
        str_vec3f(angularRateStr, angularRate);
        /* This should give us each of the respective readings */
    }
    else {
        processErrorReceived(buffer, numOfBytes);
    }

    /* Change the output frequency of the asyncronous data output to 2Hz */
    VnUartPacket_genWriteAsyncDataOutputFrequency(
    genWriteAsyncOutputFrequencyBuffer,
    sizeof(genWriteAsyncOutputFrequencyBuffer),
    VNERRORDETECTIONMODE_CHECKSUM,
    &writeAsyncFreq,
    2);

    UserUart_sendData(genWriteAsyncOutputFrequencyBuffer, writeAsyncFreq);
    UserUart_checkForReceivedData(buffer, &numOfBytes);
    if (ErrorReceived(buffer) == 0){
        VnUartPacket packet;
        VnUartPacket_initialize(packet, buffer, numOfBytes);
        uint32_t asyncOutputFreq;
        VnUartPacket_parseAsyncDataOutputFrequency(packet, &asyncOutputFreq);
    }
    else {
        processErrorReceived(buffer, numOfBytes);
    }
}

int ErrorReceived(char* message) {
    char command[6];
    for (int i = 0; i < 6; i++){
        command[i] = message[i];
    }
    if (strcmp(command, "$VNERR") != 0){
        return 1;
    }
    else {
        return 0;
    }
}

int processErrorReceived(char* errorMessage, size_t len)
{
	char errorCodeStr[100] = {'\0'};
    for (int i = 7; i < len; i++){
        errorCodeStr[i] = errorMessage[i];
    }
    if (strcmp(errorCodeStr, '1') == 0){
        printf("hard fault has occured, processor forcing a restart\n");
    }
    else if (strcmp(errorCodeStr, '2') == 0){
        printf("Serial Buffer Overflow\n");
    }
    else if (strcmp(errorCodeStr, '3') == 0){
        printf("Invalid Checksum\n");
    }
    else if (strcmp(errorCodeStr, '4') == 0){
        printf("Invalid command\n");
    }
    else if (strcmp(errorCodeStr, '5') == 0){
        printf("Not enough parameters\n");
    }
    else if (strcmp(errorCodeStr, '6') == 0){
        printf("Too many parameters\n");
    }
    else if (strcmp(errorCodeStr, '7') == 0){
        printf("Invalid paramter\n");
    }
    else if (strcmp(errorCodeStr, '8') == 0){
        printf("Invalid register\n");
    }
    else if (strcmp(errorCodeStr, '9') == 0){
        printf("Unautherized access\n");
    }
    else if (strcmp(errorCodeStr, '10') == 0){
        printf("Watchdog reset\n");
    }
    else if (strcmp(errorCodeStr, '11') == 0){
        printf("Output buffer overflow\n");
    }
    else if (strcmp(errorCodeStr, '12') == 0){
        printf("Insufficient baud rate\n");
    }
    else if (strcmp(errorCodeStr, "255") == 0){
        printf("Error buffer overflow\n");
    }
    return -1
}

