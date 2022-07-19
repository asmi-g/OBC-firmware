#include "motor_test.h"

/*
@brief Example of running motor with desired speed
*/
void motor_speedtest () {
    struct DC_Motor motor1;
    motor1.hetRam = hetRAM1;
    motor1.firstpwm = pwm0;
    motor1.secondpwm = pwm1;
    start_motor(&motor1); 
    printf("Running motor at 50 percent duty cycle");
    driveMotor(&motor1, 50);
}

void motor_directiontest() {
    
}

void PWMsignaltest() {

}