/* Include Files */

/* USER CODE BEGIN (1) */
#include "DC_motor.h"
#include <math.h>
/* USER CODE END */

/* USER CODE BEGIN (2) */

static void start_motor (struct DC_Motor *this){
    hetInit();
    //Default config is duty cycle of 50% and period of 1000ns; TCLK = 110MHz
    pwmStart(this->hetRam, this->firstpwm);
    pwmStart(this->hetRam, this->secondpwm);
}
/* speed also controls the direction, +speed = CW, -speed = CCW */
static void driveMotor(struct DC_Motor *this, int speed){
    if (speed > 100) { speed = 100; }
    else if (speed < -100 ) { speed = -100; }
    //This function will depend on what the datasheet of the motor driver says; H-Bridge
    //Should be able to handle all polarity reversals for us
    if (speed > 0) {
        pwmSetDuty(this->hetRam,this->firstpwm,speed); //Write PWM to forward pin
        pwmSetDuty(this->hetRam,this->secondpwm,0); //Reverse pin is pulled low
    }
    else {
        pwmSetDuty(this->hetRam,this->firstpwm,0); //forwards pin is pulled low
        pwmSetDuty(this->hetRam,this->secondpwm,abs(speed)); //Write PWM to reverse pin
    }
}
static void driveMotor_Torque (struct DC_Motor *this, double torque){
    double speed;
    speed = 100*(torque/this->torqueConst); //Convert the torque into duty cycle %
    if (speed > 0) {
        pwmSetDuty(this->hetRam,this->firstpwm,speed); //Write PWM to forward pin
        pwmSetDuty(this->hetRam,this->secondpwm,0); //Reverse pin is pulled low
    }
    else {
        pwmSetDuty(this->hetRam,this->firstpwm,0); //forwards pin is pulled low
        pwmSetDuty(this->hetRam,this->secondpwm,abs(speed)); //Write PWM to reverse pin
    }

}
/* 
@brief Drives the motor clockwise at 50% speed 
@param pointer to the DC_Motor struct
*/
void forward(struct DC_Motor *this){
    pwmSetDuty(this->hetRam,this->firstpwm, 50); //Write PWM to forward pin
    pwmSetDuty(this->hetRam,this->secondpwm,0); //Reverse pin is pulled low
}
/* 
@brief Drives the motor counterclockwise at 50% speed 
@param pointer to the DC_Motor struct
*/
void reverse(struct DC_Motor *this){
    pwmSetDuty(this->hetRam,this->firstpwm,0); //forwards pin is pulled low
    pwmSetDuty(this->hetRam,this->secondpwm,50); //Write PWM to reverse pin
}
/* 
@brief Sets PWM signal to 0, stopping the motor until PWM signal is changed
@param pointer to the DC_Motor struct which will 
*/
void DC_break(struct DC_Motor *this){
    pwmSetDuty(this->hetRam,this->firstpwm,0);
    pwmSetDuty(this->hetRam,this->secondpwm,0);
}
/* 
@brief Stops the motor, must be reinitialized using PWMStart or hetInit
@param pointer to the DC_Motor struct which will 
*/
void stop_motor(struct DC_Motor *this){
    pwmStop(this->hetRam, this->firstpwm);
    pwmStop(this->hetRam, this->secondpwm);
}
static struct DC_Motor new(hetRAMBASE_t* hetNum , uint32* pwmModule1, uint32* pwmModule2) {
    return (struct DC_Motor){.hetRam = hetNum,.firstpwm = pwmModule, .secondpwm = pwmModule2, .torqueConst = 1.23};
}
const struct DC_MotorClass DC_Motor={.new=new};

/* USER CODE END */
