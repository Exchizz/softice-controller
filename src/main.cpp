#include <Arduino.h>
#include <TaskScheduler.h>
#include <TrueRMS.h>

#define PIN_RELAY1 2
#define PIN_RELAY2 3

Scheduler runner;

void t1Callback();
void sampleADCCallback();

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

Task t1(100, TASK_FOREVER, &t1Callback);
Task sampleADCTask(1, TASK_FOREVER, &sampleADCCallback);

Rms MeasAvg; // Create an instance of Average.

void setup()
{
  MeasAvg.begin(MAX_CURRENT, AVG_WINDOW, ADC_10BIT, BLR_ON, CNT_SCAN);

  MeasAvg.start(); // start measuring

  pinMode(13, OUTPUT);
  pinMode(PIN_RELAY1, OUTPUT);
  pinMode(PIN_RELAY2, OUTPUT);

  Serial.begin(9600);
  Serial.println("Scheduler TEST");

  runner.init();
  Serial.println("Initialized scheduler");

  runner.addTask(t1);
  runner.addTask(sampleADCTask);
  Serial.println("added t1");

  delay(1000);

  t1.enable();
  sampleADCTask.enable();
  Serial.println("Enabled t1");
}

void sampleADCCallback()
{
  static char buffer[100];
  int sample = analogRead(A0);
  MeasAvg.update(sample);
}

void t1Callback()
{
  MeasAvg.publish();
  float power = MeasAvg.rmsVal * SOFTICE_SUPPLY_VOLTAGE;
  Serial.print(">Power:");
  Serial.println(power);
}

void loop()
{
  // put your main code here, to run repeatedly:
  runner.execute();
}