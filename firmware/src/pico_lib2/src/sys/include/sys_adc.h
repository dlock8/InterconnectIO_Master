/**
 * @file    sys_adc.h
 *
 * @brief   Analog to Digital Converter Functions
 * 
 * 
 * @copyright Copyright 2021 Ocean (iiot2k@gmail.com) All rights reserved
 *            Copyright (c) 2024, D.Lockhead. All rights reserved.
 * 
 * This software is licensed under the BSD 3-Clause License.
 * See the LICENSE file for more details.
 */


/**************************************************************************/
/*! 
    @file     sys_adc.h
    @author   Daniel Lockhead
    @date     2024
    
    @brief    Modifications on file 
    
    @section ADC MODIFICATIONS

    - fix documentation errors found by Doxygen
**************************************************************************/

#ifndef _SYS_ADC_H_
#define _SYS_ADC_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>

/*! $## **Constants:**
    @--
*/

/*! 
 * @def ADC_CH_0
 * ADC0 channel identifier.
 *
 * @def ADC_CH_1
 * ADC1 channel identifier.
 *
 * @def ADC_CH_2
 * ADC2 channel identifier.
 *
 * @def ADC_CH_V
 * VSYS Voltage channel identifier.
 *
 * @def ADC_CH_T
 * Internal Temperature Sensor channel identifier.
 *
 * @note The ADC channels are defined as follows:
 * - ADC_CH_0: ADC0
 * - ADC_CH_1: ADC1
 * - ADC_CH_2: ADC2
 * - ADC_CH_V: VSYS Voltage
 * - ADC_CH_T: Internal Temp. Sensor
 */
#define ADC_CH_0 0
#define ADC_CH_1 1
#define ADC_CH_2 2
#define ADC_CH_V 3
#define ADC_CH_T 4

/*! 
 * @def ADC_VREF
 * @brief ADC Reference Voltage
 *
 * The reference voltage for the ADC is set to 3.3V.
 */
#define ADC_VREF 3.3



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