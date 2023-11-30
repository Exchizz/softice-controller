#ifndef BUTTON_STATE_MACHINE_H
#define BUTTON_STATE_MACHINE_H

#include "Arduino.h"

#define ST_BUTTON_IDLE 1
#define ST_BUTTON_PRESSED 2
#define ST_BUTTON_WAIT_RELEASE 3


class ButtonStateMachine{
private:
    int button_pin;
    int sample_pin;

    int current_state = ST_BUTTON_IDLE;
    int next_state = ST_BUTTON_IDLE;
    int cnt_button_motor = 0;
    bool flag_button_pressed = false;

public:
    void update();
    ButtonStateMachine(int button_pin);
    bool isPressed();
    void reset();
};
#endif