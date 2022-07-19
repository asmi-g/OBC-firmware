#ifndef C_DC_MotorLib_LIBRARY_H
#define C_DC_MotorLib_LIBRARY_H

#include "het.h"
#include "sys_common.h"
#include "system.h"

#include <stdbool.h>
/*
@brief DC_Motor struct used to hold the motor's pwm modules and which HET driver its running on
@param hetRam pointer to HET RAM
    hetRAM1: HET1 RAM pointer
    hetRAM2: HET2 RAM pointer 
@param firstpwm pointer to pwm driver that controls FIN (forwards pin of motor driver) 
@param firstpwm pointer to pwm driver that controls FIN (forwards pin of motor driver)
    pwm0: Pwm 0
    pwm1: Pwm 1
    pwm2: Pwm 2
    pwm3: Pwm 3
    pwm4: Pwm 4
    pwm5: Pwm 5
    pwm6: Pwm 6
    pwm7: Pwm 7
*/
struct DC_Motor {
    hetRAMBASE_t* hetRam
    uint32* firstpwm, secondpwm;
    double const torqueConst; //Torque constant should be the torque produced at 100% duty cycle
};

extern const struct DC_MotorClass {
    struct DC_Motor (*new)(etpwmBASE_t * firstPin, etpwmBASE_t * secondPin);
} DC_Motor;

#endif //C_DC_MotorLib_LIBRARY_H
