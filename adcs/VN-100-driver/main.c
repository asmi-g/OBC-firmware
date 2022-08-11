/* Includes */
#include <string.h>
#include <stdio.h>
#include <sci.h> 

#include "vn/util.h"
#include "vn/protocol/upack.h"
#include "vn/protocol/upackf.h"
#include "vn/int.h"
#include "vn/sensors.h"
#include "vn/user_helper.h"


void packetFoundHandler(void *userData, VnUartPacket *packet, size_t runningIndexOfPacketStart);
void UserUart_initialize(void);
bool UserUart_checkForReceivedData(char* buffer, size_t* numOfBytesReceived);
void UserUart_sendData(char *data, size_t size);

/* Global Variables */
bool gIsCheckingForModelNumberResponse = false;
bool gIsCheckingForAsyncOutputFreqResponse = false;
bool gIsCheckingForVpeBasicControlResponse = false;
bool sciTriggered = false;
bool stopReceving = false;
uint8_t gEnable, gHeadingMode, gFilteringMode, gTuningMode;

char buffer[256];
size_t numOfBytes, readModelNumberSize, readVNYMR, readVNYPR, writeAsyncOutputFreqSize, readVpeBasicControlSize, writeVpeBasicControlSize;
size_t writeBinaryOutput1Size;

/* Buffers to hold commands */
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


int main (void){
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


	/* We will first illustrate querying the sensor's model number. First we
	* generate a read register command. This is subject to change depending on the desired configuration */
	VnUartPacket_genReadModelNumber(
			genReadModelNumberBuffer, /* Store command in this buffer*/
			sizeof(genReadModelNumberBuffer),
			VNERRORDETECTIONMODE_CHECKSUM,
			&readModelNumberSize);

	/* Now send the data to the sensor. */
	gIsCheckingForModelNumberResponse = true;
	UserUart_sendData(genReadModelNumberBuffer, readModelNumberSize);

	UserUart_checkForReceivedData(buffer, &numOfBytes);
	VnUartPacketFinder_processData(&up, (uint8_t*) buffer, numOfBytes);
	gIsCheckingForModelNumberResponse = false;


	/* Read YPR, magentic, acceleration and angular rate */
	VnUartPacket_genReadYawPitchRollMagneticAccelerationAndAngularRates(
		genReadVNYMR,
		sizeof(genReadVNYMR),
		VNERRORDETECTIONMODE_CHECKSUM,
		&readVNYMR);
	UserUart_sendData(genReadVNYMR, readVNYMR);
	/* Continually checking for new UART data and then passing any received data to the 
	VnUartProtocol to build, parse and verify data packets. */
	while (1) {
		if (UserUart_checkForReceivedData(buffer, &numOfBytes)) {
			VnUartPacketFinder_processData(&up, (uint8_t*) buffer, numOfBytes);
		}
	}

	/* Read only YPR */
	VnUartPacket_genReadYawPitchRoll(
		genReadVNYPR,
		sizeof(genReadVNYPR),
		VNERRORDETECTIONMODE_CHECKSUM,
		&readVNYPR);
	UserUart_sendData(genReadVNYPR, &readVNYPR);
	while (1) {
		if (UserUart_checkForReceivedData(buffer, &numOfBytes)) {
			/* Now process the data that our UART port received */
			VnUartPacketFinder_processData(&up, (uint8_t*) buffer, numOfBytes);
		}
	}

	/* Changing ASNYC output frequency to 2Hz */
	VnUartPacket_genWriteAsyncDataOutputFrequency(
		genWriteAsyncOutputFrequencyBuffer,
		sizeof(genWriteAsyncOutputFrequencyBuffer),
		VNERRORDETECTIONMODE_CHECKSUM,
		&writeAsyncOutputFreqSize,
		2);
	UserUart_sendData(genWriteAsyncOutputFrequencyBuffer);
	while(1) {
		if (UserUart_checkForReceivedData(buffer, &numOfBytes)){
			VnUartPacketFinder_processData(&up, (uint8_t*)buffer, numOfBytes);
		}
	}
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
/*
@brief 
@param errorMessage
@param errorCode
 */
int processErrorReceived(char* errorMessage, VnError errorCode)
{
	char errorCodeStr[100];
	strFromVnError(errorCodeStr, errorCode);
	printf("%s\nERROR: %s\n", errorMessage, errorCodeStr);
	return -1;
}
/*
@brief Function called when notification of valid packet occurs. 
Parses the packet according to the informantion within the packet
@param 	userData Pointer to user supplied data which will be sent on all callback notifications.
@param 	packet Pointer to packet finder data structure
@param	runningIndexOfPacketStart Used for correlating the position in the received raw 
data stream where packets are found.
*/
void packetFoundHandler(void *userData, VnUartPacket *packet, size_t runningIndexOfPacketStart)
{
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
			else if (asyncType == VNYPR)
			{
				vec3f ypr;
				VnUartPacket_parseVNYPR(packet, &yprStr[100]);
				str_vec3f(yprStr, ypr);
				printf("[Found VNYPR Packet]\n");
				printf("  YawPitchRoll: %s\n", yprStr);
			}
			else if (asyncType == VNACC)
			{
				vec3f accel;
				VnUartPacket_parseVNACC(packet, &yprStr[100]);
				str_vec3f(accelStr, accel);
				printf("[Found VNACC Packet]\n");
				printf("  Acceleration: %s\n", accelStr);
			}
			else if (asyncType == VNMAG)
			{
				vec3f mag;
				VnUartPacket_parseVNMAG(packet, &yprStr[100]);
				str_vec3f(magStr, mag);
				printf("[Found VNMAG Packet]\n");
				printf("  Magnetic: %s\n", magStr);

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
