// Copyright 2021 Ocean (iiot2k@gmail.com) 
// All rights reserved.

/*!
    @mainpage ../README.md
    @name sys_adc - Analog to Digital Converter Functions
    @file sys_adc.h
    @lib lib2_sys
    @n
*/

/*! $### Default pin assignment for lib2 
    @image images/pico-pinout.png 
*/

#ifndef _SYS_ADC_H_
#define _SYS_ADC_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>

/*! $## **Constants:**
    @--
*/

/*! $### **ADC channel**
    @def ADC_CH_0 ADC0
    @def ADC_CH_1 ADC1
    @def ADC_CH_2 ADC2
    @def ADC_CH_V VSYS Voltage
    @def ADC_CH_T Internal Temp. Sensor
    @n
*/
#define ADC_CH_0 0
#define ADC_CH_1 1
#define ADC_CH_2 2
#define ADC_CH_V 3
#define ADC_CH_T 4

/*! $### **ADC Reference Voltage**
    @def ADC_VREF ADC Reference Voltage (3.3V)
    @n
*/
#define ADC_VREF 3.0

/*! $## **Functions:**
    @--
*/

/*! @brief - Init ADC channel
    @param ch ADC channel
*/
void sys_adc_init(uint8_t ch);

/*! @brief - Read ADC raw value
    @param ch ADC channel
    @return ADC raw value 0..4095 (0..VREF)
*/
uint16_t sys_adc_raw(uint8_t ch);

/*! @brief - Read ADC and scale raw value to low..high
    @param ch ADC channel
    @param low Low value for 0
    @param high High value for 4095 (3.3V or VREF)
    @return Value between low and high 
*/
double sys_adc_scale(uint8_t ch, double low, double high);

/*! @brief - Read ADC voltage value
    @param ch ADC channel
    @return ADC voltage value 0..3.3V (VREF)
*/
double sys_adc_volt(uint8_t ch);


/*! @brief - Read ADC VSYS voltage
    @return VSYS voltage value ~5V
*/
double sys_adc_vsys();

/*! @brief - Read ADC internal temp. sensor in celsius
    @return Temp. value in celsius
*/
double sys_adc_temp_c();


/*! @brief - Read ADC internal temp. sensor in fahrenheit
    @return Temp. value in fahrenheit
*/
double sys_adc_temp_f();


#ifdef __cplusplus
}
#endif

#endif   // _SYS_ADC_H_