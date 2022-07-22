/* Include Files */

/* USER CODE BEGIN (1) */
#include "DC_motor.h"
#include <math.h>
#include <time.h>
#include <unistd.h>
/* USER CODE END */

/* USER CODE BEGIN (2) */
/*
@brief initializes motor with default configurations set in HalCoGen
    Default config is duty cycle of 50% and period of 1000ns; TCLK = 110MHz
@param DC_Motor: pointer to motor struct
*/
void init_motor(struct DC_Motor *this){
    hetInit();
    printf("Starting motor with default configurations");
}
void start_motor (struct DC_Motor *this){
    pwmStart(this->hetRam, this->firstpwm);
    pwmStart(this->hetRam, this->secondpwm);
}
/* 
@brief control motor speed with duty cycle and period  
@param DC_Motor: pointer to motor struct
@param speed: value from -100 to 100, determines duty cycle and direction (negative = CCW, positive = CW)
@param per: period of the PWM signal
*/
void driveMotor(struct DC_Motor *this, int speed, int per, int dur){
    hetSIGNAL_t sig; 
    if (speed > 100) { speed = 100; }
    else if (speed < -100 ) { speed = -100; }
    sig.period = per;

    if (speed > 0) {
        sig.duty = speed;
        pwmSetSignal(this->hetRam,this->firstpwm, sig); //Write PWM to forward pin
        pwmSetDuty(this->hetRam,this->secondpwm,0); //Reverse pin is pulled low
    }
    else {
        sig.duty = abs(speed);
        pwmSetDuty(this->hetRam,this->firstpwm,0); //forwards pin is pulled low
        pwmSetSignal(this->hetRam,this->secondpwm, sig); //Write PWM to reverse pin
    }
    sleep(dur); //Just have some sort of delay here, not sure if this even works right now
    DC_break(this);
    return 0;
}
/* 
@brief drive motor with desired speed;  
@param DC_Motor: pointer to motor struct
@param torque: 
@param dur: the duration of how long the motor should run for
*/
void driveMotor_Torque (struct DC_Motor *this, double torque, int per, int dur){
    double speed; 
    speed = 100*(torque/this->torqueConst); //Convert the torque into duty cycle %
    hetSIGNAL_t sig; 
    if (speed > 100) { speed = 100; }
    else if (speed < -100 ) { speed = -100; }
    sig.period = per;
    
    if (speed > 0) {
        sig.duty = speed;
        pwmSetSignal(this->hetRam,this->firstpwm, sig); //Write PWM to forward pin
        pwmSetDuty(this->hetRam,this->secondpwm,0); //Reverse pin is pulled low
    }
    else {
        sig.duty = abs(speed);
        pwmSetDuty(this->hetRam,this->firstpwm,0); //forwards pin is pulled low
        pwmSetSignal(this->hetRam,this->secondpwm, sig); //Write PWM to reverse pin
    }
    sleep(dur); //Just have some sort of delay here, not sure if this even works right now
    DC_break(this);
    return 0;
}
/* 
@brief Drives the motor clockwise at 50% speed 
@param DC_Motor: pointer to motor struct
*/
void forward(struct DC_Motor *this){
    pwmSetDuty(this->hetRam,this->firstpwm, 50); //Write PWM to forward pin
    pwmSetDuty(this->hetRam,this->secondpwm,0); //Reverse pin is pulled low
}
/* 
@brief Drives the motor counterclockwise at 50% speed 
@param DC_Motor: pointer to motor struct
*/
void reverse(struct DC_Motor *this){
    pwmSetDuty(this->hetRam,this->firstpwm,0); //forwards pin is pulled low
    pwmSetDuty(this->hetRam,this->secondpwm,50); //Write PWM to reverse pin
}
/* 
@brief Sets PWM signal to 0, stopping the motor until PWM signal is changed
@param DC_Motor: pointer to motor struct
*/
void brake(struct DC_Motor *this){
    pwmSetDuty(this->hetRam,this->firstpwm,0);
    pwmSetDuty(this->hetRam,this->secondpwm,0);
}
/* 
@brief Stops the motor, must be reinitialized using PWMStart or hetInit
@param DC_Motor: pointer to motor struct
*/
void stop_motor(struct DC_Motor *this){
    pwmStop(this->hetRam, this->firstpwm);
    pwmStop(this->hetRam, this->secondpwm);
}
// static struct DC_Motor new(hetRAMBASE_t* hetNum , uint32* pwmModule1, uint32* pwmModule2) {
//     return (struct DC_Motor){.hetRam = hetNum,.firstpwm = pwmModule1, .secondpwm = pwmModule2, .torqueConst = 1.23};
// }
// const struct DC_MotorClass DC_Motor={.new = &new};

/* USER CODE END */
