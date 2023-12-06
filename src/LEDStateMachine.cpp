#include "../include/LEDStateMachine.h"

LEDStateMachine::LEDStateMachine(int led_pin)
{
  this->led_pin = led_pin;
}

void LEDStateMachine::setLED(int blink_delay)
{
  motor_led_blink_delay = blink_delay;
}

bool LEDStateMachine::update(unsigned long blinkTick, bool follow_state)
{
  switch (motor_led_blink_delay)
  {
  case LED_ON:
    digitalWrite(led_pin, LOW);
    needs_update = true;
    break;

  case LED_BLINK_OFF:
    digitalWrite(led_pin, HIGH);
    break;

  default:
    if (motor_led_blink_delay != last_motor_led_blink_delay)
    {
      needs_update = true;
      Serial.println("Changing blink state");
    }
    if (needs_update)
    {
      needs_update = false;
      digitalWrite(led_pin, follow_state);
    }

    current_state = digitalRead(led_pin);

    if (blinkTick % motor_led_blink_delay == 0)
    {
      digitalWrite(led_pin, !current_state);
    }
    break;
  }
  last_motor_led_blink_delay = motor_led_blink_delay;
  return current_state;
}