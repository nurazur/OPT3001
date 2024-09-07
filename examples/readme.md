## Arduino examples for OPT3001 Ambient Light Sensor
- **opt3001demo**:                    continuous acquisition
- **opt3001single-shot_polling**:     singleshot; Configuration Register polling; blocking function
- **opt3001single-shot_polling_2**:   singleshot; Configuration Register polling; non-blocking function used in loop()
- **opt3001single-shot_interrupt_1**: singleshot; instead of polling the Configuration Register
- **opt3001single-shot_interrupt_2**: singleshot; waiting for the interupt issued by the OPT3001 after completion of a data acquisition.
  Performance optimized, the MCU sleeps during the data acquisition. This sample works for MegaCoreX and DxCore only.
