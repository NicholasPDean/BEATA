#include <SoftwareSerial.h>

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>

//i2c
Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();

#define LSM9DS1_SCK A5
#define LSM9DS1_MISO 12
#define LSM9DS1_MOSI A4
#define LSM9DS1_XGCS 6
#define LSM9DS1_MCS 5

#define SPEAKER 3
#define BUTTON 11

#define C4 261
#define D4 294
#define E4 330
#define F4 349
#define G4 392
#define A4 440
#define B4 494
#define C5 523

const int txPin = 1;
const int rxPin = 0;

SoftwareSerial BTSerial(rxPin, txPin); // RX, TX

//Starting character value that will be replaced
char inputChar = 'a'; 

//Initialize starting button state
bool buttonState = false;
bool stolen = false;
int numCycles = 0;

void setupSensor()
{
  lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G);
  lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);
  lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
}

void alarm()
{
  tone(SPEAKER, 2093);
  delay(1000);
  noTone(SPEAKER);
  delay(300);
}

void lockSound()
{
  playNote(C4, 100);
  playNote(D4, 100);
  playNote(E4, 100);
  playNote(F4, 100);
  playNote(G4, 100);
  playNote(A4, 100);
  playNote(B4, 100);
  playNote(C5, 100);
}

void unlockSound()
{
  playNote(C5, 100);
  playNote(B4, 100);
  playNote(A4, 100);
  playNote(G4, 100);
  playNote(F4, 100);
  playNote(E4, 100);
  playNote(D4, 100);
  playNote(C4, 100);
}

void playNote(int note, int delayAmount)
{
  tone(SPEAKER, note);
  delay(delayAmount);
  noTone(SPEAKER);
  delay(delayAmount * (4/5));
}

void setup()
{
  //Initialize
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON, OUTPUT);
  Serial.begin(9600);
  BTSerial.begin(38400);

  while (!Serial)
  {
    delay(1);
  }

  if (!lsm.begin())
  {
    Serial.println("Unable to initialize the LSM9DS1.");
    while (1);
  }
  Serial.println("Found LSM9DS1");

  setupSensor();
}

void loop() {
  if (BTSerial.available() > 0) // The value returned by Serial.available will be 0 if there has not been a transmission.
  {
    inputChar = Serial.read(); // Store character read from serial
    //Serial.println(inputChar); // This sends the received value back to the Bluetooth master (your computer)
    switch (inputChar)
    {
      case 'b': //Button
      if (buttonState)
      {
        digitalWrite(BUTTON, LOW);
        buttonState = false;
        stolen = false;
        unlockSound();
        Serial.println("Device locked");
      }
      else
      {
        digitalWrite(BUTTON, HIGH);
        buttonState = true;
        lockSound();
        Serial.println("Device unlocked");
      }
      break;
    }
    
  }
  if (buttonState)
  {
    lsm.read();
    sensors_event_t a, m, g, temp;
    lsm.getEvent(&a, &m, &g, &temp);

    if (a.acceleration.z < 9)
    {
      if (numCycles >= 5)
      {
        stolen = true;
        Serial.println("Your laptop is being stolen!");
      } 
      else
      {
        numCycles = numCycles + 1;
      }
    }
    else
    {
      numCycles = 0;
    }
    
    if (stolen)
    {
      alarm();
    }
  }
}
