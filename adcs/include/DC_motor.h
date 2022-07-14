#ifndef C_DC_MotorLib_LIBRARY_H
#define C_DC_MotorLib_LIBRARY_H
#include <stdbool.h>

struct DC_Motor {
    hetRAMBASE_t* hetRam
    uint32* firstpwm, secondpwm;
    double const torqueConst; //Torque constant should be the torque produced at 100% duty cycle
};

extern const struct DC_MotorClass {
    struct DC_Motor (*new)(etpwmBASE_t * firstPin, etpwmBASE_t * secondPin);
} DC_Motor;

#endif //C_DC_MotorLib_LIBRARY_H
