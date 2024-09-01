OPT3001 Digital Ambient Light Sensor (ALS) Library 
=====================================================================================================

Arduino library for the  [Texas Instruments OPT3001](http://www.ti.com/product/OPT3001) Digital Ambient Light Sensor (ALS) Sensor.
The OPT3001 allows to measure light illuminance in Lux units in a very simple way. 
The original library from [ClosedCube](https://www.arduino.cc/reference/en/libraries/closedcube-opt3001/) has been amended by methods 
to allow single-shot measurements with or without hardware interrupt. 

The most energy efficient method is to initiate a single-shot measurement, put the MCU to sleep while the OPT3001 integrates samples, and wake up the MCU by a hardware interrupt of the OPT3001, as soon as the result is available .

Some of the single-shot examples are especially made for ATmega0 and AVR Dx series MCU's (for example ATmega4808 or AVR64DD32). Therefore the TiNo2 library is used to support pin change interrupts, RTC and sleep methods.
