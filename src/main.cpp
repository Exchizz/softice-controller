#include <Arduino.h>
#include <TaskScheduler.h>
#include <TrueRMS.h>
#include <ButtonStateMachine.h>
#include <LEDStateMachine.h>
#include <MotorStateMachine.h>

#define PIN_RELAY1 2
#define PIN_RELAY2 3
#define PIN_MOTOR_BUTTON 7
#define PIN_MOTOR_LED 10
#define PIN_COMPRESSOR_LED 11

#define PIN_COMPRESSOR_BUTTON 6

// #define ENABLE_ENERGY_LOG 1

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

// #define ST_BUTTON_WAIT_RELEASE 5
// #define ST_BUTTON_PRESSED 6

int current_state = ST_INIT;
int next_state = current_state;

int motor_led_blink_delay = LED_BLINK_1HZ;

Task t1(100, TASK_FOREVER, &t1Callback);
Task sampleADCTask(1, TASK_FOREVER, &sampleADCCallback);
Task sampleMotorButton(100, TASK_FOREVER, &sampleMotorButtonCallback);
Task stateMachineTask(100, TASK_FOREVER, &stateMachineCallback);
Task motorBlinkTask(100, TASK_FOREVER, &motorBlinkCallback);

Rms MeasAvg; // Create an instance of Average.

ButtonStateMachine *motorBtnSTM;
ButtonStateMachine *compressorBtnSTM;

LEDStateMachine *motorLEDSTM;
LEDStateMachine *compressorLEDSTM;

MotorStateMachine *motorSTM;
MotorStateMachine *compressorSTM;

void setup()
{
  MeasAvg.begin(MAX_CURRENT, AVG_WINDOW, ADC_10BIT, BLR_ON, CNT_SCAN);

  MeasAvg.start(); // start measuring

  pinMode(13, OUTPUT);
  pinMode(PIN_RELAY1, OUTPUT);
  pinMode(PIN_RELAY2, OUTPUT);
  pinMode(PIN_MOTOR_LED, OUTPUT);
  pinMode(PIN_COMPRESSOR_LED, OUTPUT);

  pinMode(PIN_MOTOR_BUTTON, INPUT);
  pinMode(PIN_COMPRESSOR_BUTTON, INPUT);

  digitalWrite(PIN_RELAY1, HIGH);
  digitalWrite(PIN_RELAY2, HIGH);

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

  motorBtnSTM = new ButtonStateMachine(PIN_MOTOR_BUTTON);
  compressorBtnSTM = new ButtonStateMachine(PIN_COMPRESSOR_BUTTON);

  motorLEDSTM = new LEDStateMachine(PIN_MOTOR_LED);
  compressorLEDSTM = new LEDStateMachine(PIN_COMPRESSOR_LED);

  motorSTM = new MotorStateMachine(PIN_RELAY1);
  motorSTM->setLEDStateMachine(motorLEDSTM);
  motorSTM->setButtonStateMachine(motorBtnSTM);

  compressorSTM = new MotorStateMachine(PIN_RELAY2);
  compressorSTM->setLEDStateMachine(compressorLEDSTM);
  compressorSTM->setButtonStateMachine(compressorBtnSTM);

  //  motorSTM = new MotorStateMachine(PIN_RELAY2);

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

void stateMachineCallback()
{
  motorSTM->update();
  compressorSTM->update();
}

void sampleMotorButtonCallback()
{
  motorBtnSTM->update();
  compressorBtnSTM->update();
}

void motorBlinkCallback()
{
  motorLEDSTM->update();
  compressorLEDSTM->update();
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