#include "vn/user_helper.h"
#include <sci.h>

void RESFactory() {
	UserUart_sendData("$VNRFS*5F", 10);
}
void pauseASYNC() {
	UserUart_sendData("$VNASY,0*XX", 12);
}
void resumeASYNC() {
	UserUart_sendData("$VNASY,1*XX", 12);
}
void tare() {
	UserUart_sendData("$VNTAR*5F", 10);
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