#include "motor_test.h"
#include <time.h>
#include <unistd.h>

/*
@brief Example of running motor with desired speed
*/
void motor_speedtest () {
    struct DC_Motor motor1;
    motor1.hetRam = hetRAM1;
    motor1.firstpwm = pwm0;
    motor1.secondpwm = pwm1;

    start_motor(&motor1); 
    printf("Running motor1 at 50 percent duty cycle with a 1000us period");
    sleep(1);
    driveMotor(&motor1, 50, 1000);
    //Introducing more motors  
    struct DC_Motor motor2;
    motor2.hetRam = hetRAM1;
    motor2.firstpwm = pwm2;
    motor2.secondpwm = pwm3;

    start_motor(&motor2);
    printf("Running motor2 at 75 percent duty cycle with a 1000us period");
    sleep(1);
    driveMotor(&motor2, 75, 1000);
}

void motor_directiontest() {
    struct DC_Motor motor1;
    motor1.hetRam = hetRAM1;
    motor1.firstpwm = pwm0;
    motor1.secondpwm = pwm1;

    start_motor(&motor1); 
    printf("Running motor1 at 50 percent duty cycle with a 1000us period forwards");
    driveMotor(&motor1, 50, 1000);
    printf("Running motor1 at 50 percent duty cycle with a 1000us period backwards");
    driveMotor(&motor1, -50, 1000);
}
/*
@brief Test for time between changing pwm signals
*/
void PWMsignaltest() {
    pwmEnableNotification(hetRAM1, pwm0, pwmEND_OF_DUTY);
    struct DC_Motor motor1;
    motor1.hetRam = hetRAM1;
    motor1.firstpwm = pwm0;
    motor1.secondpwm = pwm1;

}