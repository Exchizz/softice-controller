#include <Arduino.h>
#include <TaskScheduler.h>
#include <TrueRMS.h>
#include <ButtonStateMachine.h>
#include <LEDStateMachine.h>
#include <MotorStateMachine.h>
#include <CompressorStateMachine.h>


// Debug flags
//#define ENABLE_SERIAL_DEBUG

#define THRESHOLD_SOFTICE_DONE_WATT 500

#define PIN_RELAY1 2
#define PIN_RELAY2 3
#define PIN_MOTOR_BUTTON 7
#define PIN_MOTOR_LED 10
#define PIN_COMPRESSOR_LED 11
#define PIN_COMPRESSOR_BUTTON 6

#define PIN_ADC_CURRENT A0
#define PIN_ADC_VOLTAGE A1


// Transformer for 230 volt measurement
#define TRANS_RATIO 67 // 230/70 = ~3.3


#define ENABLE_ENERGY_LOG 1

Scheduler runner;

void t1Callback();
void sampleADCCallback();
void sampleMotorButtonCallback();
void motorBlinkCallback();
void stateMachineCallback();

#define SUPPLY_VOLTAGE 5.0
#define ADC_CENTER SUPPLY_VOLTAGE / 2.0
#define MAX_CURRENT 25 // Approx
#define VOLTAGE_DIVIDER_VOLTAGE 0.4 // 10/(15+10)
#define MAX_VOLTAGE 897// 5 = (x/70)*1.4*0.4-2.5, x = 312.5
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
bool leader_state = false;

int motor_led_blink_delay = LED_BLINK_1HZ;

Task t1(100, TASK_FOREVER, &t1Callback);
Task sampleADCTask(1, TASK_FOREVER, &sampleADCCallback);
Task sampleMotorButton(100, TASK_FOREVER, &sampleMotorButtonCallback);
Task stateMachineTask(100, TASK_FOREVER, &stateMachineCallback);
Task motorBlinkTask(10, TASK_FOREVER, &motorBlinkCallback);

Power acPower; // Create an instance of Average.

ButtonStateMachine *motorBtnSTM;
ButtonStateMachine *compressorBtnSTM;

LEDStateMachine *motorLEDSTM;
LEDStateMachine *compressorLEDSTM;

MotorStateMachine *motorSTM;
CompressorStateMachine *compressorSTM;
unsigned long blinkTick = 0;

void setup()
{
//	acPower.begin(acVoltRange, acCurrRange, RMS_WINDOW, ADC_10BIT, BLR_ON, SGL_SCAN);
  acPower.begin(MAX_VOLTAGE, MAX_CURRENT, AVG_WINDOW, ADC_10BIT, BLR_ON, SGL_SCAN);

  acPower.start(); // start measuring

  pinMode(13, OUTPUT);
  pinMode(PIN_RELAY1, OUTPUT);
  pinMode(PIN_RELAY2, OUTPUT);
  pinMode(PIN_MOTOR_LED, OUTPUT);
  pinMode(PIN_COMPRESSOR_LED, OUTPUT);

  pinMode(PIN_MOTOR_BUTTON, INPUT);
  pinMode(PIN_COMPRESSOR_BUTTON, INPUT);

  digitalWrite(PIN_RELAY1, HIGH);
  digitalWrite(PIN_RELAY2, HIGH);

  Serial.begin(115200);
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

  compressorSTM = new CompressorStateMachine(PIN_RELAY2);
  compressorSTM->setLEDStateMachine(compressorLEDSTM);
  compressorSTM->setButtonStateMachine(compressorBtnSTM);

  t1.enable();
  sampleADCTask.enable();
  sampleMotorButton.enable();
  stateMachineTask.enable();
  motorBlinkTask.enable();
  Serial.println("Enabled all tasks");
}

void sampleADCCallback()
{
  int sampleCurrent = analogRead(PIN_ADC_CURRENT);
  int sampleVoltage = analogRead(PIN_ADC_VOLTAGE);

  // *2 due to voltage-divider
  float sampleVoltageNormalized = (sampleVoltage * 5.0 / 1024) - 2.5;
  float voltageSecVolt = sampleVoltageNormalized/VOLTAGE_DIVIDER_VOLTAGE;
  float voltagePriVolt = voltageSecVolt * TRANS_RATIO;

  // *2 due to voltage-divider
  acPower.update(sampleVoltage, sampleCurrent);

#ifdef ENABLE_ENERGY_LOG
  float currentvoltage = sampleCurrent * 5.0 / 1024;
  Serial.print(">ADC(0):");
  Serial.print(currentvoltage);
  Serial.println(",np");

  Serial.print(">ADC(1):");
  Serial.print(sampleVoltage);
  Serial.println(",np");

  Serial.print(">V:");
  Serial.print(voltagePriVolt);
  Serial.println(",np");

#endif
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
  blinkTick = ++blinkTick % 200;
  leader_state = motorLEDSTM->update(blinkTick, leader_state);
  leader_state = compressorLEDSTM->update(blinkTick, leader_state);
}

void t1Callback()
{
  if(acPower.acqRdy){
    acPower.publish();
    acPower.start();
  } else {
    Serial.print(">Publishing:");
	  Serial.print(0);
    Serial.println(",np");
  }

  Serial.print(">RMSval1:");
	Serial.print(acPower.rmsVal1);
  Serial.println(",np");

  Serial.print(">RMSval2:");
	Serial.print(acPower.rmsVal2);
  Serial.println(",np");

  Serial.print(">AppPwr:");
	Serial.print(acPower.apparentPwr);
  Serial.println(",np");

  Serial.print(">RealPwr:");
	Serial.print(acPower.realPwr);
  Serial.println(",np");

  Serial.print(">PF:");
	Serial.print(acPower.pf);
  Serial.println(",np");
//  float power = acPower.rmsVal * SOFTICE_SUPPLY_VOLTAGE;
#ifdef ENABLE_SERIAL_DEBUG
  String incomingMSG;
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingMSG = Serial.readStringUntil('\n');
    Serial.print("Received message:-");
    incomingMSG.replace("\n", "");
    incomingMSG.replace("\r", "");
    
    Serial.print(incomingMSG);
    Serial.println("-");

    if(incomingMSG.equals("ready")){
      Serial.println("Softice ready!");
      compressorSTM->set_ready_flag();
    }
    if(incomingMSG.equals("notready")){
      Serial.println("Softice not ready!");
      compressorSTM->clear_ready_flag();
    }
  }

#endif
#ifdef ENABLE_ENERGY_LOG
//  Serial.print(">Power:");
//  Serial.println(power);
#endif
#ifdef SIMULATE_POWER
  if(power > THRESHOLD_SOFTICE_DONE_WATT){
      compressorSTM->set_ready_flag();
  } else {
      compressorSTM->clear_ready_flag();
  }
#endif
}

void loop()
{
  // put your main code here, to run repeatedly:
  runner.execute();
}