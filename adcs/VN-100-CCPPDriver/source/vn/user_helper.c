#include "vn/user_helper.h"
#include "vn/util.h"
#include "vn/protocol/upack.h"
#include "vn/protocol/upackf.h"
#include "vn/int.h"
#include "vn/sensors.h"
#include <stdio.h>

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