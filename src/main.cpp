#include <Arduino.h>
#include <TaskScheduler.h>
#include <TrueRMS.h>

#define PIN_RELAY1 2
#define PIN_RELAY2 3
#define PIN_MOTOR_BUTTON 7
#define PIN_MOTOR_LED 10


//#define ENABLE_ENERGY_LOG 1


Scheduler runner;

void t1Callback();
void sampleADCCallback();
void sampleMotorButtonCallback();
void motorBlinkCallback();
void stateMachineCallback();

#define SUPPLY_VOLTAGE 5.0
#define ADC_CENTER SUPPLY_VOLTAGE / 2.0
#define MAX_CURRENT 5
#define AVG_WINDOW 40 // number of samples

#define SOFTICE_SUPPLY_VOLTAGE 230

float fmap(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

bool toggle = true;

#define ST_INIT 0
#define ST_IDLE 1
#define ST_MOTOR_ON 2
#define ST_MOTOR_OFF 3
#define ST_MOTOR_RUNNING 4


#define ST_BUTTON_WAIT_RELEASE 5
#define ST_BUTTON_PRESSED 6


#define LED_BLINK_1HZ 100
#define LED_BLINK_10HZ 10
#define LED_BLINK_5HZ 5
#define LED_BLINK_100HZ 1
#define LED_ON -2
#define LED_BLINK_OFF -1

int current_state = ST_INIT;
int next_state = current_state;

bool flag_button_pressed = true;

int motor_led_blink_delay = 0;

Task t1(100, TASK_FOREVER, &t1Callback);
Task sampleADCTask(1, TASK_FOREVER, &sampleADCCallback);
Task sampleMotorButton(100, TASK_FOREVER, &sampleMotorButtonCallback);
Task stateMachineTask(100, TASK_FOREVER, &stateMachineCallback);
Task motorBlinkTask(100, TASK_FOREVER, &motorBlinkCallback);

Rms MeasAvg; // Create an instance of Average.

void motor_enabled(bool motor_on){
  if(motor_on){
    digitalWrite(PIN_RELAY1, HIGH);
  } else {
    digitalWrite(PIN_RELAY1, LOW);
  }
}


void setup()
{
  MeasAvg.begin(MAX_CURRENT, AVG_WINDOW, ADC_10BIT, BLR_ON, CNT_SCAN);

  MeasAvg.start(); // start measuring

  pinMode(13, OUTPUT);
  pinMode(PIN_RELAY1, OUTPUT);
  pinMode(PIN_RELAY2, OUTPUT);
  pinMode(PIN_MOTOR_LED, OUTPUT);

  Serial.begin(9600);
  Serial.println("Scheduler TEST");

  runner.init();
  Serial.println("Initialized scheduler");

  runner.addTask(t1);
  runner.addTask(sampleADCTask);
  runner.addTask(sampleMotorButton);
  runner.addTask(stateMachineTask);
  runner.addTask(motorBlinkTask);
  Serial.println("added t1");

  delay(1000);

  t1.enable();
  sampleADCTask.enable();
  sampleMotorButton.enable();
  stateMachineTask.enable();
  motorBlinkTask.enable();
  Serial.println("Enabled all tasks");
}

void sampleADCCallback()
{
  int sample = analogRead(A0);
  MeasAvg.update(sample);
}

bool button_motor_pressed = false;
int cnt_button_motor = 0;

bool get_button_state(){
  bool tmp = button_motor_pressed;
  button_motor_pressed = false;
  return tmp;
}


void motor_led_state(int blink_delay){
  motor_led_blink_delay = blink_delay;
}
void stateMachineCallback()
{
  switch(current_state){
    case ST_INIT:
      next_state = ST_IDLE;
    break;

    case ST_IDLE:
      if(flag_button_pressed){
        flag_button_pressed = false;
        next_state = ST_MOTOR_ON;
      }
    break;


    case ST_MOTOR_RUNNING:
    if(flag_button_pressed){
      flag_button_pressed = false;
      next_state = ST_MOTOR_OFF;
    }
    break;


    case ST_MOTOR_OFF:
      motor_enabled(false);
      motor_led_state(LED_BLINK_5HZ);
      next_state = ST_IDLE;
      break;

    case ST_MOTOR_ON:
      motor_enabled(true);
      motor_led_state(LED_ON);
      next_state = ST_MOTOR_RUNNING;
    break;
  }

  current_state = next_state;
}

void sampleMotorButtonCallback(){
  int sample_motor_button = digitalRead(PIN_MOTOR_BUTTON);

  static int current_state = ST_IDLE;
  static int next_state = ST_IDLE;

  switch(current_state){
    case ST_IDLE:
      if(sample_motor_button){
        cnt_button_motor++;
      } else {
        cnt_button_motor = 0;
      }
    
      if(cnt_button_motor > 5){
        next_state = ST_BUTTON_PRESSED;
      }
    break;


    case ST_BUTTON_PRESSED:
      flag_button_pressed = true;
      next_state = ST_BUTTON_WAIT_RELEASE;
    break;


    case ST_BUTTON_WAIT_RELEASE:
      if(flag_button_pressed == false && sample_motor_button == false){
        next_state = ST_IDLE;
      }
    break;
  }
  current_state = next_state;
}


void motorBlinkCallback(){
  switch (motor_led_blink_delay)
  {
  case LED_ON:
    digitalWrite(PIN_MOTOR_LED, LOW);
    return;
    break;
  
  case LED_BLINK_OFF:
    digitalWrite(PIN_MOTOR_LED, HIGH);
    return;
    break;
  }
  static int sleep_cnt = 0;
  Serial.print("sleep blink counter:");
  Serial.println(sleep_cnt);
  if(sleep_cnt++ > motor_led_blink_delay){
    digitalWrite(PIN_MOTOR_LED, !digitalRead(PIN_MOTOR_LED));
    sleep_cnt = 0;
  }
}

void t1Callback()
{
  MeasAvg.publish();
  float power = MeasAvg.rmsVal * SOFTICE_SUPPLY_VOLTAGE;
#ifdef ENABLE_ENERGY_LOG
  Serial.print(">Power:");
  Serial.println(power);
#endif
}

void loop()
{
  // put your main code here, to run repeatedly:
  runner.execute();
}