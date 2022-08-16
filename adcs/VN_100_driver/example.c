#include "vn/util.h"
#include "vn/protocol/upack.h"
#include "vn/protocol/upackf.h"
#include "vn/int.h"
#include "vn/user_helper.h"

#include <string.h>
#include <stdio.h>
#include <sci.h> 

void packetFoundHandler(void *userData, VnUartPacket *packet, size_t runningIndexOfPacketStart);
void UserUart_initialize(void);
bool UserUart_checkForReceivedData(char* buffer, size_t* numOfBytesReceived);
void UserUart_sendData(char *data, size_t size);

/* Global Variables for response message */
bool gIsCheckingForModelNumberResponse = false;
bool gIsCheckingForAsyncOutputFreqResponse = false;
bool gIsCheckingForVpeBasicControlResponse = false;
uint8_t gEnable, gHeadingMode, gFilteringMode, gTuningMode;

/* Buffer that temporarily stores the data received from the sensor*/
char buffer[256];

/* The number of bytes within each respective buffer */
size_t numOfBytes, readModelNumberSize, readVNYMR, readVNYPR, writeAsyncOutputFreqSize, readVpeBasicControlSize, writeVpeBasicControlSize;
size_t writeBinaryOutput1Size;

/* Buffers to hold commands that will be sent to the sensor */
char genReadModelNumberBuffer[256];
char genReadVNYMR[256];
char genReadVNYPR[256];
char genWriteAsyncOutputFrequencyBuffer[256];
char genReadVpeBasicControlBuffer[256];
char genWriteVpeBasicControlBuffer[256];
char genWriteBinaryOutput1Buffer[256];

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
        VnError error;
        processErrorReceived(buffer, )
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
    for (int i = 7; i <= len; i++){
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

