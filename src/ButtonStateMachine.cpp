#include <Arduino.h>
#include <../include/ButtonStateMachine.h>

ButtonStateMachine::ButtonStateMachine(int button_pin)
{
    this->button_pin = button_pin;
}

bool ButtonStateMachine::isPressed()
{
    return flag_button_pressed;
}

void ButtonStateMachine::reset()
{
    flag_button_pressed = false;
}   

void ButtonStateMachine::update()
{
    sample_pin = digitalRead(this->button_pin);

    switch (current_state)
    {
    case ST_BUTTON_IDLE:
        if (sample_pin)
        {
            cnt_button_motor++;
        }
        else
        {
            cnt_button_motor = 0;
        }

        if (cnt_button_motor > 5)
        {
            next_state = ST_BUTTON_PRESSED;
        }
        break;

    case ST_BUTTON_PRESSED:
        flag_button_pressed = true;
        next_state = ST_BUTTON_WAIT_RELEASE;
        break;

    case ST_BUTTON_WAIT_RELEASE:
        if (flag_button_pressed == false && sample_pin == false)
        {
            next_state = ST_BUTTON_IDLE;
        }
        break;
    }
    current_state = next_state;
}
