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

Written by AA for ClosedCube

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

OPT3001_Config ConfigReg;

void setup()
{
    pinMode(9,OUTPUT);
    digitalWrite(9, HIGH);
    Serial.begin(9600);
    Serial.println("ClosedCube OPT3001 Arduino Test");
    
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


// polling method. When OPT3001 shuts down, data are available.
// so we poll the 2 bits of ModeOfConversionOperation (00 = shut down)
// this method is blocking. While the measurement is ongoing 
// no other tasks can be performed.
void loop()
{
    singleshot();
    bool done = false;
    while (!done)
    {
        delay(10);
        ConfigReg = opt3001.readConfig();
        if (ConfigReg.ModeOfConversionOperation == 0)
        {
            done = true;   
        }
    }
        
    OPT3001 result = opt3001.readResult();
    printResult("OPT3001", result);
    
    ConfigReg = opt3001.readConfig();
    
    delay(3000);
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