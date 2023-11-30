#include "../include/MotorStateMachine.h"

MotorStateMachine::MotorStateMachine(int motor_pin)
{
    this->motor_pin = motor_pin;
}




void MotorStateMachine::setButtonStateMachine(ButtonStateMachine *buttonSTM)
{
    motorBtnSTM = buttonSTM;
}   

void MotorStateMachine::setLEDStateMachine(LEDStateMachine *ledSTM)
{
    motorLEDSTM = ledSTM;
}   


void MotorStateMachine::setMotorState(bool motor_on)
{
  if (motor_on)
  {
    digitalWrite(motor_pin, LOW);
  }
  else
  {
    digitalWrite(motor_pin, HIGH);
  }
}


void MotorStateMachine::update()
{
    switch (current_state)
    {
    case ST_MOTOR_INIT:
        next_state = ST_MOTOR_IDLE;
        break;

    case ST_MOTOR_IDLE:
        if (motorBtnSTM->isPressed())
        {
            motorBtnSTM->reset();
            next_state = ST_MOTOR_ON;
        }
        break;

    case ST_MOTOR_RUNNING:
        if (motorBtnSTM->isPressed())
        {
            motorBtnSTM->reset();
            next_state = ST_MOTOR_OFF;
        }
        break;

    case ST_MOTOR_OFF:
        setMotorState(false);
        motorLEDSTM->setLED(LED_BLINK_1HZ);
        next_state = ST_MOTOR_IDLE;
        break;

    case ST_MOTOR_ON:
        setMotorState(true);
        motorLEDSTM->setLED(LED_ON);
        next_state = ST_MOTOR_RUNNING;
        break;
    }

    current_state = next_state;
}