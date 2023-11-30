#include "../include/LEDStateMachine.h"


LEDStateMachine::LEDStateMachine(int led_pin)
{
    this->led_pin = led_pin;
}

void LEDStateMachine::setLED(int blink_delay)
{
  motor_led_blink_delay = blink_delay;
}

void LEDStateMachine::update() {
    switch (motor_led_blink_delay)
    {
    case LED_ON:
      digitalWrite(led_pin, LOW);
      return;
      break;

    case LED_BLINK_OFF:
      digitalWrite(led_pin, HIGH);
      return;
      break;
    }
    Serial.print("sleep blink counter:");
    Serial.println(sleep_cnt);
    if (sleep_cnt++ >= motor_led_blink_delay)
    {
      digitalWrite(led_pin, !digitalRead(led_pin));
      sleep_cnt = 0;
    }
}