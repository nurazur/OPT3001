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

// for architecture ATmega328P use lower baud rate on serial port and different ports for lED and I2C VDD
#if (__AVR_ARCH__ == 5)
    #define SERIAL_BAUD 9600
    #define LED 8
    #define I2C_POWER_PIN 9
#else
    #define SERIAL_BAUD 57600
    #define LED 19
    #define I2C_POWER_PIN 25
#endif

#define END_OF_CONVERSION_MODE 0xC000

// address pin is tied to VDD
#define OPT3001_ADDRESS 0x45

// address pin is tied to GND
//#define OPT3001_ADDRESS 0x44

ClosedCube_OPT3001 opt3001;
OPT3001_Config ConfigReg;

void setup()
{
    pinMode(LED, OUTPUT);
    
    pinMode(I2C_POWER_PIN,OUTPUT);
    digitalWrite(I2C_POWER_PIN, HIGH);
    Serial.begin(SERIAL_BAUD);
    Serial.println("ClosedCube OPT3001 Arduino Test: singleshot, nonblocking");
    
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


void loop()
{
    OPT3001 result_lux;
    static bool opt3001_done = false;
    
    opt3001_done = nonblockingSingleShot(&result_lux);
   
    if (opt3001_done)
    {
        printResult("OPT3001", result_lux);
        
        // do whatever you want to do periodically
        digitalWrite(LED, !digitalRead(LED));
        
        opt3001_done = false;
        delay(1000); //or sleep
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

// returns true when a measurement is available.
// starts a new measurement if no data acquisition is ongoing.
bool nonblockingSingleShot(OPT3001 *result)
{
    static bool running =false;
    
    if (!running)
    {
        singleshot(); // start a measurement
        running= true;
    }
    
    if (running)
    {
        ConfigReg = opt3001.readConfig();
        if (ConfigReg.ModeOfConversionOperation == 0)
        {
            running = false;
            *result = opt3001.readResult();
            ConfigReg = opt3001.readConfig(); // clear all flags
            return true;
        }             
    }
    
    return false;
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