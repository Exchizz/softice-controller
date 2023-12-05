#include "../include/CompressorStateMachine.h"



void CompressorStateMachine::update()
{
    switch (current_state)
    {
    case ST_COMPRESSOR_INIT:
        next_state = ST_COMPRESSOR_IDLE;
        break;

    case ST_COMPRESSOR_IDLE:
        if (motorBtnSTM->isPressed())
        {
            motorBtnSTM->reset();
            next_state = ST_COMPRESSOR_ON;
        }
        break;

    case ST_COMPRESSOR_RUNNING:
        if (motorBtnSTM->isPressed())
        {
            motorBtnSTM->reset();
            next_state = ST_COMPRESSOR_OFF;
        }

        if(flag_ready){
            motorLEDSTM->setLED(LED_ON);
        } else {
            motorLEDSTM->setLED(LED_BLINK_5HZ);
        }

        break;

    case ST_COMPRESSOR_OFF:
        setMotorState(false);
        motorLEDSTM->setLED(LED_BLINK_1HZ);
        next_state = ST_COMPRESSOR_IDLE;
        break;

    case ST_COMPRESSOR_ON:
        setMotorState(true);
        motorLEDSTM->setLED(LED_BLINK_5HZ);
        next_state = ST_COMPRESSOR_RUNNING;
        break;
    }

    current_state = next_state;
}