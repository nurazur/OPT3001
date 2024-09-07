/*
This is example for ClosedCube OPT3001 Digital Ambient Light Sensor breakout board 

Initial Date: 02-Dec-2015

Hardware connections for Arduino Uno:
VDD to 3.3V DC
SDA to A4
SCL to A5
GND to common ground

Written by AA for ClosedCube

MIT License

Modified by nurarzur to fit into the TiNo series of battery operated sensors

*/

#include "Arduino.h"
#include <TiNo2_OPT3001.h>

// for architecture ATmega328P use lower baud rate on serial port and different ports for lED and I2C VDD
#if (__AVR_ARCH__ == 5)
    #define SERIAL_BAUD 9600
    #define LED 8
    #define I2C_POWER 9
#else
    #define SERIAL_BAUD 57600
    #define LED 19
    #define I2C_POWER 25
#endif


// address pin on my board is tied to VDD
#define OPT3001_ADDRESS 0x45
ClosedCube_OPT3001 opt3001;


void setup()
{
	Serial.begin(SERIAL_BAUD);
    pinMode(I2C_POWER, OUTPUT);
    digitalWrite(I2C_POWER, HIGH);
    
	Serial.println("ClosedCube OPT3001 Arduino Test: continuous acquisition");

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
	OPT3001 result = opt3001.readResult();
	printResult("OPT3001", result);
	delay(3000);
}

void configureSensor() {
	OPT3001_Config newConfig;
	
	newConfig.RangeNumber = B1100;	
	newConfig.ConvertionTime = B0;
	newConfig.Latch = B1;
	newConfig.ModeOfConversionOperation = B11;

	OPT3001_ErrorCode errorConfig = opt3001.writeConfig(newConfig);
	if (errorConfig != NO_ERROR)
		printError("OPT3001 configuration", errorConfig);
	else {
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