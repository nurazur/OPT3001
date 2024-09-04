/**
This is example for OPT3001 Digital Ambient Light Sensor in single shot mode

There are 3 different approaches to singleshot measurement:

1. polling config register, wait until ModeOfConversionOperation bits are B00
2. waiting (polling) for interrupt port to go low after data acquisition, 2 examples
3. sleep CPU, wake up on interrupt

Examples use all the same setup() function.

loop function for each example needs to be uncommented.

Initial Date: 02-Dec-2015

Hardware connections for Arduino Uno:
VDD to 3.3V DC
SDA to A4
SCL to A5
GND to common ground

Written by nurazur <nurazur@gmail.com>

MIT License

**/

#include "Arduino.h"
#include <avr/sleep.h>
#include <TiNo2_OPT3001.h>

#define END_OF_CONVERSION_MODE 0xC000

ClosedCube_OPT3001 opt3001;

// address pin is tied to VDD
#define OPT3001_ADDRESS 0x45

// address pin is tied to GND
//#define OPT3001_ADDRESS 0x44

#define OPT3001_INTERRUPT_PIN 6

OPT3001_Config ConfigReg;


/*****************************************************************************/
/***                       Pin Change Interrupts                           ***/
/*****************************************************************************/
#include <PinChangeInterrupt.h>
uint8_t event_triggered = 0;

// ISR's for the Pin change Interrupt

void wakeUp0() { event_triggered |= 0x1; }


void setup()
{
    Serial.begin(9600);
    Serial.println("OPT3001 Arduino Test: Interrupt within loop()");

    pinMode(9, OUTPUT);
    digitalWrite(9, HIGH);


    pinMode(OPT3001_INTERRUPT_PIN, INPUT_PULLUP);
    attachPCINT(digitalPinToPCINT(OPT3001_INTERRUPT_PIN), wakeUp0, FALLING);
    //attachInterrupt(1, wakeUp0, FALLING); // possible on Pins D2(INT 0) and D3 (INT 1)
    sei();

    opt3001.begin(OPT3001_ADDRESS);
    Serial.print("OPT3001 Manufacturer ID");
    Serial.println(opt3001.readManufacturerID());
    Serial.print("OPT3001 Device ID");
    Serial.println(opt3001.readDeviceID());

    configureSensor();
    printResult("High-Limit", opt3001.readHighLimit());
    printResult("Low-Limit", opt3001.readLowLimit());
    Serial.println("----");
}


// interupt with polling loop
void loop()
{
    static uint8_t measurement_running =0;
    static unsigned long time_expired = 0;

    if (millis() >= time_expired)
    {
        //delay end, so start a measurement
        time_expired = millis() + 4000; // start new delay time
        if (!measurement_running)
        {
            measurement_running = 1;
            event_triggered = 0;
            singleshot();
        }

        //add your other tasks here
    }

    if (measurement_running && event_triggered)
    {
        measurement_running = 0;
        event_triggered = 0;
        OPT3001 result = opt3001.readResult();
        printResult("OPT3001", result);
        ConfigReg = opt3001.readConfig(); // clear all flags
    }
}


// start a single-shot measurement
void singleshot()
{
    OPT3001_Config newConfig;
    newConfig.RangeNumber = B1100;
    newConfig.ConvertionTime = B0;
    newConfig.Latch = B1;
    newConfig.ModeOfConversionOperation = B01;

    OPT3001_ErrorCode errorConfig = opt3001.writeConfig(newConfig);
    if (errorConfig != NO_ERROR)
        printError("OPT3001 configuration", errorConfig);
}


void configureSensor() {
    OPT3001_Config newConfig;

    newConfig.RangeNumber = B1100;  // aut range
    newConfig.ConvertionTime = B0;
    newConfig.Latch = B1;

    // shut down device
    newConfig.ModeOfConversionOperation = B00;

    // to trigger a interrupt, the latched window-style is used.
    // Latch field =1
    // End-of-Conversion-Mode is used, so the 2 MSB's of the
    // Low-Limit Register expononent are set to B11.
    // This way, each time a result is available a interrupt is triggered.
    opt3001.writeHighLimit(0);
    opt3001.writeLowLimit(END_OF_CONVERSION_MODE);

    OPT3001_ErrorCode errorConfig = opt3001.writeConfig(newConfig);
    if (errorConfig != NO_ERROR)
        printError("OPT3001 configuration", errorConfig);
    else {
        // clear all flags and interrupt line
        OPT3001_Config sensorConfig = opt3001.readConfig();
        Serial.println("OPT3001 Current Config:");
        Serial.println("------------------------------");

        Serial.print("Conversion ready (R):");
        Serial.println(sensorConfig.ConversionReady,HEX);
        Serial.print("Conversion time (R/W):");
        Serial.println(sensorConfig.ConvertionTime, HEX);

        Serial.print("Fault count field (R/W):");
        Serial.println(sensorConfig.FaultCount, HEX);

        Serial.print("Flag high field (R-only):");
        Serial.println(sensorConfig.FlagHigh, HEX);

        Serial.print("Flag low field (R-only):");
        Serial.println(sensorConfig.FlagLow, HEX);

        Serial.print("Latch field (R/W):");
        Serial.println(sensorConfig.Latch, HEX);

        Serial.print("Mask exponent field (R/W):");
        Serial.println(sensorConfig.MaskExponent, HEX);

        Serial.print("Mode of conversion operation (R/W):");
        Serial.println(sensorConfig.ModeOfConversionOperation, HEX);

        Serial.print("Polarity field (R/W):");
        Serial.println(sensorConfig.Polarity, HEX);

        Serial.print("Overflow flag (R-only):");
        Serial.println(sensorConfig.OverflowFlag, HEX);

        Serial.print("Range number (R/W):");
        Serial.println(sensorConfig.RangeNumber, HEX);

        Serial.println("------------------------------");
    }
}

void printResult(String text, OPT3001 result) {
    if (result.error == NO_ERROR) {
        Serial.print(text);
        Serial.print(": ");
        Serial.print(result.lux);
        Serial.println(" lux");
    }
    else {
        printError(text,result.error);
    }
}

void printError(String text, OPT3001_ErrorCode error) {
    Serial.print(text);
    Serial.print(": [ERROR] Code #");
    Serial.println(error);
}