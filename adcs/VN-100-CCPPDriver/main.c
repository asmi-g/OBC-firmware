/* Includes */
#include <string.h>
#include <stdio.h>
#include "vn/util.h"
#include "vn/protocol/upack.h"
#include "vn/protocol/upackf.h"
#include "vn/int.h"
#include "vn/sensors.h"
#include "sci.h"

void packetFoundHandler(void *userData, VnUartPacket *packet, size_t runningIndexOfPacketStart);
void UserUart_initialize(void);
bool UserUart_checkForReceivedData(char* buffer, size_t bufferSize, size_t* numOfBytesReceived);
void UserUart_mockReceivedData(char* buffer, size_t bufferSize);
void UserUart_sendData(char *data, size_t size);

bool gIsCheckingForModelNumberResponse = false;
bool gIsCheckingForAsyncOutputFreqResponse = false;
bool gIsCheckingForVpeBasicControlResponse = false;
uint8_t gEnable, gHeadingMode, gFilteringMode, gTuningMode;

int main (void){
	char buffer[256];
    size_t numOfBytes, readModelNumberSize, writeAsyncOutputFreqSize, readVpeBasicControlSize, writeVpeBasicControlSize;
    size_t writeBinaryOutput1Size;
    char genReadModelNumberBuffer[256];
    char genWriteAsyncOutputFrequencyBuffer[256];
    char genReadVpeBasicControlBuffer[256];
    char genWriteVpeBasicControlBuffer[256];
    char genWriteBinaryOutput1Buffer[256];

	VnUartPacketFinder up;

	/* Iniitalize the data structure */
	VnUartPacketFinder_initialize(&up);

	/* First thing you should do is initialize the data structure. */
	VnUartPacketFinder_initialize(&up);

	/* Register our callback method for when the VnUartPacketFinder finds an
		ASCII asynchronous packet. */
	VnUartPacketFinder_registerPacketFoundHandler(&up, packetFoundHandler, NULL);

	/* Initialize the UART port */
		UserUart_initialize();

	/* Continually checking for new UART data and then passing any received data to the 
		VnUartProtocol to build, parse and verify data packets. */
	// while (1) {
	// 	size_t numOfBytes; 
	// 	if (UserUart_checkForReceivedData(buffer, sizeof(buffer), &numOfBytes)) {
	// 		VnUartPacketFinder_processReceivedData(buffer, numOfBytes);
	// 		break;
	// 	}
	// }

	/* We will first illustrate querying the sensor's model number. First we
	* generate a read register command. This is subject to change depending on the desired configuration */
	VnUartPacket_genReadModelNumber(
			genReadModelNumberBuffer,
			sizeof(genReadModelNumberBuffer),
			VNERRORDETECTIONMODE_CHECKSUM,
			&readModelNumberSize);

		/* Now send the data to the sensor. */
		gIsCheckingForModelNumberResponse = true;
		UserUart_sendData(genReadModelNumberBuffer, readModelNumberSize);

		/* Now process the mock data that our fake UART port received and hand it
		* over to our UART packet finder. */
		UserUart_checkForReceivedData(buffer, sizeof(buffer), &numOfBytes);
		VnUartPacketFinder_processData(&up, (uint8_t*)buffer, numOfBytes);
		gIsCheckingForVpeBasicControlResponse = false;
}

void asciiAsyncMessageReceived(void *userData, VnUartPacket *packet, size_t runningIndex)
{
	vec3f ypr;
	char strConversions[50];

	/* Silence 'unreferenced formal parameters' warning in Visual Studio. */
	(userData);
	(runningIndex);

	/* Make sure we have an ASCII packet and not a binary packet. */
	if (VnUartPacket_type(packet) != PACKETTYPE_ASCII)
		return;

	/* Make sure we have a VNYPR data packet. */
	if (VnUartPacket_determineAsciiAsyncType(packet) != VNYPR)
		return;

	/* We now need to parse out the yaw, pitch, roll data. */
	VnUartPacket_parseVNYPR(packet, &ypr);

	/* Now print out the yaw, pitch, roll measurements. */
	str_vec3f(strConversions, ypr);
	printf("ASCII Async YPR: %s\n", strConversions);
}

void asciiOrBinaryAsyncMessageReceived(void *userData, VnUartPacket *packet, size_t runningIndex)
{
	vec3f ypr;
	char strConversions[50];

	/* Silence 'unreferenced formal parameters' warning in Visual Studio. */
	(userData);
	(runningIndex);

	if (VnUartPacket_type(packet) == PACKETTYPE_ASCII && VnUartPacket_determineAsciiAsyncType(packet) == VNYPR)
	{
		VnUartPacket_parseVNYPR(packet, &ypr);
		str_vec3f(strConversions, ypr);
		printf("ASCII Async YPR: %s\n", strConversions);

		return;
	}

	if (VnUartPacket_type(packet) == PACKETTYPE_BINARY)
	{
		uint64_t timeStartup;

		/* First make sure we have a binary packet type we expect since there
		 * are many types of binary output types that can be configured. */
		if (!VnUartPacket_isCompatible(packet,
			COMMONGROUP_TIMESTARTUP | COMMONGROUP_YAWPITCHROLL,
			TIMEGROUP_NONE,
			IMUGROUP_NONE,
			GPSGROUP_NONE,
			ATTITUDEGROUP_NONE,
			INSGROUP_NONE,
      GPSGROUP_NONE))
			/* Not the type of binary packet we are expecting. */
			return;

		/* Ok, we have our expected binary output packet. Since there are many
		 * ways to configure the binary data output, the burden is on the user
		 * to correctly parse the binary packet. However, we can make use of
		 * the parsing convenience methods provided by the Packet structure.
		 * When using these convenience methods, you have to extract them in
		 * the order they are organized in the binary packet per the User Manual. */
		timeStartup = VnUartPacket_extractUint64(packet);
		ypr = VnUartPacket_extractVec3f(packet);

		str_vec3f(strConversions, ypr);
		printf("Binary Async TimeStartup: %" PRIu64 "\n", timeStartup);
		printf("Binary Async YPR: %s\n", strConversions);

		return;
	}
}

int processErrorReceived(char* errorMessage, VnError errorCode)
{
	char errorCodeStr[100];
	strFromVnError(errorCodeStr, errorCode);
	printf("%s\nERROR: %s\n", errorMessage, errorCodeStr);
	return -1;
}
void packetFoundHandler(void *userData, VnUartPacket *packet, size_t runningIndexOfPacketStart)
{
    /* Silence 'unreferenced formal parameters' warning in Visual Studio. */
    (runningIndexOfPacketStart);
    (userData);
    /* When this function is called, the packet will already have been
     * validated so no checksum/CRC check is required. */
    /* First see if this is an ASCII or binary packet. */
    if (VnUartPacket_type(packet) == PACKETTYPE_ASCII)
    {
        /* Now that we know this is an ASCII packet, we can call the various
         * ASCII functions to further process this packet. */
        if (VnUartPacket_isAsciiAsync(packet))
        {
            /* We know we have an ASCII asynchronous data packet. Let's see if
             * this is a message type we are looking for. */
            VnAsciiAsync asyncType = VnUartPacket_determineAsciiAsyncType(packet);
            if (asyncType == VNYMR)
            {
                /* Parse the VNYMR message. */
                vec3f ypr, mag, accel, angularRate;
                char yprStr[100], magStr[100], accelStr[100], angularRateStr[100];
                VnUartPacket_parseVNYMR(packet, &ypr, &mag, &accel, &angularRate);
                str_vec3f(yprStr, ypr);
                str_vec3f(magStr, mag);
                str_vec3f(accelStr, accel);
                str_vec3f(angularRateStr, angularRate);
                printf("[Found VNYMR Packet]\n");
                printf("  YawPitchRoll: %s\n", yprStr);
                printf("  Magnetic: %s\n", magStr);
                printf("  Acceleration: %s\n", accelStr);
                printf("  Angular Rate: %s\n", angularRateStr);
            }
        }
        else if (VnUartPacket_isResponse(packet))
        {
            if (gIsCheckingForModelNumberResponse)
            {
                char modelNumber[100];
                VnUartPacket_parseModelNumber(packet, modelNumber);
                printf("Model Number: %s\n", modelNumber);
            }
            else if (gIsCheckingForAsyncOutputFreqResponse)
            {
                uint32_t asyncOutputFreq;
                VnUartPacket_parseAsyncDataOutputFrequency(packet, &asyncOutputFreq);
                printf("Asynchronous Output Frequency: %u Hz\n", asyncOutputFreq);
            }
            else if (gIsCheckingForVpeBasicControlResponse)
            {
                char enableStr[100], headingModeStr[100], filteringModeStr[100], tuningModeStr[100];
                VnUartPacket_parseVpeBasicControl(packet, &gEnable, &gHeadingMode, &gFilteringMode, &gTuningMode);
                strFromBool(enableStr, (bool) gEnable);
                strFromHeadingMode(headingModeStr, gHeadingMode);
                strFromFilterMode(filteringModeStr, gFilteringMode);
                strFromFilterMode(tuningModeStr, gTuningMode);
                printf("[VPE Basic Control]\n");
                printf("  Enable: %s\n", enableStr);
                printf("  Heading Mode: %s\n", headingModeStr);
                printf("  Filtering Mode: %s\n", filteringModeStr);
                printf("  Tuning Mode: %s\n", tuningModeStr);
            }
        }
        else if (VnUartPacket_isError(packet))
        {
            uint8_t error;
            char errorStr[100];
            VnUartPacket_parseError(packet, &error);
            strFromSensorError(errorStr, (SensorError) error);
            printf("Sensor Error: %s\n", errorStr);
        }
    }
    else if (VnUartPacket_type(packet) == PACKETTYPE_BINARY) {
        uint64_t timeStartup;
        vec3f ypr;
        char yprStr[100];
        /* See if this is a binary packet type we are expecting. */
        if (!VnUartPacket_isCompatible(
            packet,
            COMMONGROUP_TIMESTARTUP | COMMONGROUP_YAWPITCHROLL,
            TIMEGROUP_NONE,
            IMUGROUP_NONE,
            GPSGROUP_NONE,
            ATTITUDEGROUP_NONE,
            INSGROUP_NONE))
        {
            /* Not the type of binary packet we are expecting. */
            return;
        }
        /* Ok, we have our expected binary output packet. Since there are many
         * ways to configure the binary data output, the burden is on the user
         * to correctly parse the binary packet. However, we can make use of
         * the parsing convenience methods provided by the VnUartPacket structure.
         * When using these convenience methods, you have to extract them in
         * the order they are organized in the binary packet per the User Manual. */
        timeStartup = VnUartPacket_extractUint64(packet);
        ypr = VnUartPacket_extractVec3f(packet);
        str_vec3f(yprStr, ypr);
        printf("[Binary Packet Received]\n");
        printf("  TimeStartup: %u\n", (uint32_t) timeStartup);
        printf("  Yaw Pitch Roll: %s\n", yprStr);
    }
}
/* Initialize the device's UART port. */
void UserUart_initialize(void)
{
	sciInit();
	sciSetBaudrate(scilinREG, 115200); /* Use LIN PORT configured as SCI2 
	and set baud rate to vn-100 factor default of 115200 */
	sciEnableNotification(scilinREG, SCI_RX_INT);
}
/* Query the environment's UART for any data received.*/
bool UserUart_checkForReceivedData(char* buffer, size_t bufferSize, size_t* numOfBytesReceived)
{
    (bufferSize);
	unsigned char recievedData[256];
	/* If SCI module not ready to recieve, return false */
    if (sciIsIdleDetected(scilinREG) == SCI_IDLE)
        return false;
	int bytesToRead = 256; /* Set some large upper limit to read from */
	/* recieve data and store in recievedData */
	sciReceive(scilinREG, bytesToRead, recievedData);
	char newBuffer [256];
	int pos = 0;
	/* Extract unwanted 0xFF from the previous array */
	for (int i = 0; i < bytesToRead; i++) {
		if (recievedData[i] != 0x000000FF) { 
			printf("Byte #%d = %c\n" i, recievedData[i]);
			newBuffer[pos] = recievedData[i];
			pos++;
		}
	}
	int sizeofNewBuffer = pos+1;
	memcpy(buffer, newBuffer, sizeofNewBuffer);
    *numOfBytesReceived = sizeofNewBuffer;
    return true;	
}
/* Sending data to VN-100 Sensor. */
void UserUart_sendData(char *data, size_t size)
{
    (data);
    (size);
	sciSend(scilinREG, size, data);
}