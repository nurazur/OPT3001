## Arduino examples for OPT3001 Ambient Light Sensor
- **opt3001demo**:                    continuous acquisition
- **opt3001single-shot_polling**:     singleshot; Configuration Register polling; blocking function
- **opt3001single-shot_polling_2**:   singleshot; Configuration Register polling; non-blocking function used in loop()
- **opt3001single-shot_interrupt_1**: singleshot; instead of polling the Configuration Register, wait for the interupt issued by the OPT3001 after completion of data acquisition. On Arduino devices this example requires the [PinChangeInterrupt](https://github.com/NicoHood/PinChangeInterrupt) library. On MegaCoreX devices only a subset of GPIO's can be used for the interrupt line (see source code).
- **opt3001single-shot_interrupt_2**: singleshot; wait for the interupt issued by the OPT3001 after completion of data acquisition.
  Performance optimized, the MCU sleeps during the data acquisition. This exsample works for MegaCoreX and DxCore only and requires the [TiNo2](https://github.com/nurazur/TiNo2) library.
