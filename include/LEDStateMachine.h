#ifndef LED_STATE_MACHINE_H
#define LED_STATE_MACHINE_H
#include "Arduino.h"

#define LED_BLINK_10HZ 10
#define LED_BLINK_1HZ 100
#define LED_BLINK_5HZ 50
#define LED_ON -2
#define LED_BLINK_OFF -1

class LEDStateMachine
{  

    private:
        int motor_led_blink_delay = LED_BLINK_1HZ;
        int sleep_cnt = 0;
        int led_pin;
    public:
        void update();
        LEDStateMachine(int led_pin);
        void setLED(int blink_delay);
};

#endif