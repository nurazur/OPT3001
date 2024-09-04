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

// for MEGACOREX interrupt pin must be one out of (10,14,18,22)
// #define OPT3001_INTERRUPT_PIN 18

OPT3001_Config ConfigReg;


/*****************************************************************************/
/***                       Pin Change Interrupts                           ***/
/*****************************************************************************/
uint8_t event_triggered = 0;

// ISR's for the Pin change Interrupt

void wakeUp0() { event_triggered |= 0x1; }


void setup()
{
    Serial.begin(57600);
    Serial.println("OPT3001 Arduino Test: Interrupt within loop()");

    //TiNo2: all I2C devices are powered through this port
    pinMode(25, OUTPUT);
    digitalWrite(25, HIGH);


    //pinMode(OPT3001_INTERRUPT_PIN, INPUT_PULLUP);
    //#if defined (MEGACOREX)
    //#if (OPT3001_INTERRUPT_PIN !=10 && OPT3001_INTERRUPT_PIN !=14 && OPT3001_INTERRUPT_PIN !=18 && OPT3001_INTERRUPT_PIN !=22)
        //#error for MEGACOREX interrupt pin must be one out of (10,14,18,22)
        //#endif
    //#endif

    //attachInterrupt(OPT3001_INTERRUPT_PIN, wakeUp0, FALLING);
    //sei();

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
void loop()
{
    singleshot();
    bool done = false;
    while (!done)
    {
        delay(10);
        //Serial.println(digitalRead(18));
        ConfigReg = opt3001.readConfig();
        if (ConfigReg.ModeOfConversionOperation == 0)
        {
            done = true;   
        }
    }
    
    // Serial.print("event: ");Serial.println(event_triggered);
    // if (event_triggered == 1)event_triggered=0;
    // Serial.print("event: ");Serial.println(event_triggered);
    // Serial.print("FH: "); Serial.println(ConfigReg.FlagHigh);
    // Serial.print("FL: "); Serial.println(ConfigReg.FlagLow);
    
    OPT3001 result = opt3001.readResult();
    printResult("OPT3001", result);
    
    ConfigReg = opt3001.readConfig();
    //Serial.print("FH: "); Serial.println(ConfigReg.FlagHigh);
    //Serial.print("FL: "); Serial.println(ConfigReg.FlagLow);
    
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
    // So each time a result is available a interrupt is triggered.
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