/**
This is example for OPT3001 Digital Ambient Light Sensor in single shot mode

This sketch demonstrates how the OPT3001 can be used in a battery operated system with
the ATmega4808. The TiNo2 Board package needs to be installed under Arduino IDE.
This package comes with all drivers needed for extreme low power consumption. 

Written by AA for ClosedCube 2015
Modified 03-Sep-2024 by nurazur <nurazur@gmail.com>

MIT License

**/

#include "Arduino.h"
#include <avr/sleep.h>
#include "tino2.h"
#include <TiNo2_OPT3001.h>

#define END_OF_CONVERSION_MODE 0xC000

ClosedCube_OPT3001 opt3001;

// address pin is tied to VDD
#define OPT3001_ADDRESS 0x45

#define OPT3001_INTERRUPT_PIN 18

OPT3001_Config ConfigReg;



// Input sense configuration (ISC)
void disablePinISC(uint8_t pin)
{
  PORT_t *port = digitalPinToPortStruct(pin);
  // Get bit position for getting pin ctrl reg
  uint8_t bit_pos = digitalPinToBitPosition(pin);

  // Calculate where pin control register is
  volatile uint8_t *pin_ctrl_reg = getPINnCTRLregister(port, bit_pos);

  // Disable ISC
  *pin_ctrl_reg = PORT_ISC_INPUT_DISABLE_gc;
}

/*****************************************************************************/
/***                       Pin Change Interrupts                           ***/
/*****************************************************************************/
uint8_t event_triggered = 0;

// ISR's for the Pin change Interrupt

void wakeUp0() { event_triggered |= 0x1; }



/*****************************************************************************/
/***                   Sleep mode                                          ***/
/*****************************************************************************/
uint16_t watchdog_counter=2;

/*****************************************************************************/
/******                   Periodic Interrupt Timer and RTC setup         *****/
/*****************************************************************************/
PITControl PIT;

// interrupt service routine for RTC periodic timer
ISR(RTC_PIT_vect)
{
    RTC.PITINTFLAGS = RTC_PI_bm;   // clear interrupt flag
    watchdog_counter++;
}


/*****************************************************************************/
void setup()
{
    // *** disable all GPIO's except USART0 *** /
    // this saves approximately 30 to 40 µA during sleep
    for (uint8_t i = 2; i < 26; i++)
    {
        pinMode(i, INPUT_PULLUP);
        disablePinISC(i);
    }

    // ***   Enable SERIAL PORT  RX   *** /
    pinConfigure(1, PIN_DIR_INPUT, PIN_PULLUP_ON, PIN_INPUT_ENABLE);

    pinMode(25, OUTPUT);
    digitalWrite(25, HIGH);
    
    Serial.begin(57600);
    Serial.println("OPT3001 Arduino Test: Interrupt driven with sleep mode");

    pinMode(OPT3001_INTERRUPT_PIN, INPUT_PULLUP);
    register_pci(0, OPT3001_INTERRUPT_PIN, wakeUp0, FALLING);

    // Periodic Interrupt Timer
    PIT.init(0, 0); // internal 32kHz ULPO Oscillator, wakeup every 8s
    PIT.enable();

    // sleep mode init
    set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode = power-down, < 1uA in sleep mode.
    sleep_enable();    // enable sleep control
    sei();             // enable interrupts

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

// interrupt driven, sleep CPU during data acquisition
// The ISR of the RTC will increment the watchdog counter.
// The period of the RTC interrupt is initialized to 8s.
// The ISR of the pin change interrupt will set event_triggered to 1
void loop()
{
    if (watchdog_counter >=2) // here 2x8s = 16s
    {
        watchdog_counter =0;

        // start LUX measurement
        singleshot();
        sleep_cpu(); // sleep until ready.
        if(event_triggered)
        {
            event_triggered = 0;
            OPT3001 result = opt3001.readResult();
            printResult("OPT3001", result);
            ConfigReg = opt3001.readConfig(); // clear all flags and inactivate INT line.
        }
        
        //place any other code here that needs to be executed periodically
        // i.e. send measurement data 
    }
    
    //go sleep
    Serial.flush();
    sleep_cpu();
    // on interrupt (RTC or any PCI) we restart from here.
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
    // Low-Limit Register expononent are set to B11. (see DS 7.4.2.3 End-of-Conversion Mode)
    // Each time a result is available a interrupt is triggered.
    opt3001.writeHighLimit(0);
    opt3001.writeLowLimit(END_OF_CONVERSION_MODE);

    OPT3001_ErrorCode errorConfig = opt3001.writeConfig(newConfig);
    if (errorConfig != NO_ERROR)
        printError("OPT3001 configuration", errorConfig);
    else {
        // clear all flags and interrupt line
        OPT3001_Config sensorConfig = opt3001.readConfig(); // clear all flags and the interrupt line
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