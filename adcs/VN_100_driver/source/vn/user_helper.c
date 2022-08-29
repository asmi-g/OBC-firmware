#include "vn/user_helper.h"
#include <sci.h>

void RESFactory() {
	char command [256];
	size_t len;
	VnUartPacket_genCmdRestoreFactorySettings(command, sizeof(command), VNERRORDETECTIONMODE_CHECKSUM, &len);
	UserUart_sendData(command, len);
}
void pauseASYNC() {
	UserUart_sendData("$VNASY,0*XX", 12);
}
void resumeASYNC() {
	UserUart_sendData("$VNASY,1*XX", 12);
}
void tare() {
	char command [256];
	size_t len;
	VnUartPacket_genCmdTare(command, sizeof(command), VNERRORDETECTIONMODE_CHECKSUM, &len);
	UserUart_sendData(command, len);
}
void resetIMU(){
	char command [256];
	size_t len;
	VnUartPacket_genCmdReset(command, sizeof(command), VNERRORDETECTIONMODE_CHECKSUM, &len);
	UserUart_sendData(command, len);
}

void readModelNumREG(){
	char modelNum[24]; /* Max model number size is 24 characters */
	size_t len;
	UserUart_sendData("$VNWRG,1*XX",12);
	UserUart_checkForReceivedData(modelNum, len);
	printf("The model number is: %s", modelNum);
}
/* Initialize the device's UART port. */
void UserUart_initialize(void)
{
	sciInit();
	sciSetBaudrate(scilinREG, 115200); /* Use LIN PORT configured as SCI2 
	and set baud rate to vn-100 factory default of 115200 */
}

/* Query the environment's UART for any data received.*/
bool UserUart_checkForReceivedData(char* buffer, size_t* numOfBytesReceived)
{
	if (sciIsRxReady == SCI_RX_INT) {
		unsigned char buf[128] = {'\0'};
		int i = 0;

		/* Read byte by byte until an '*' is received */
		sciReceive(scilinREG, 1, (uint8_t *)&buf[i]);
		while(buf[i] != '*') {
			i++;
			if (i >= 128) {
				break;
			}
			sciReceive(scilinREG, 1, (uint8_t *)&buf[i]);
		}
		/* Add the star back to the buffer */
		buf[i] = '*';
		i++;
		/* Get the last six characters "xx\r\n" */
		int limit = i+6;
		while (i < limit){
			sciReceive(scilinREG, 1, (uint8_t *)&buf[i]);
			i++;
		}
		/* copy local buffer to buffer outside */
		memcpy(buffer, buf, i);
		*numOfBytesReceived = i;
		/* Slight delay */
		for (int j = 0; j < 10000; j++);
		return true;
	}
	else {
		return false;
	}
}

/* Sending data to VN-100 Sensor. */
void UserUart_sendData(char *data, size_t size)
{
	/* Send message across SCI module on 115200 baud rate */
    sciSend(scilinREG, size, data);
}