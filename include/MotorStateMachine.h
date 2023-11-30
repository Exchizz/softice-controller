#ifndef MOTOR_STATE_MACHINE_H
#define MOTOR_STATE_MACHINE_H

#include "Arduino.h"
#include "LEDStateMachine.h"
#include "ButtonStateMachine.h"

#define ST_MOTOR_INIT 0
#define ST_MOTOR_IDLE 1
#define ST_MOTOR_ON 2
#define ST_MOTOR_OFF 3
#define ST_MOTOR_RUNNING 4

class MotorStateMachine
{
private:
    int motor_pin;
    int current_state = ST_MOTOR_IDLE;
    int next_state = ST_MOTOR_IDLE;
    ButtonStateMachine *motorBtnSTM;    
    LEDStateMachine *motorLEDSTM;

public:
    MotorStateMachine(int motor_pin);
    void setLEDStateMachine(LEDStateMachine *ledSTM);
    void setButtonStateMachine(ButtonStateMachine *buttonSTM);
    void update();
    void setMotorState(bool motor_on);
};
#endif