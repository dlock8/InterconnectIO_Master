//
   
/*
 * Copyright (c) 2022 Daniel Lockhead <daniel.lockhead@videotron.ca>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _FUNCTADV_H_
#define _FUNCTADV_H_



//#include "scpi/scpi.h"


// define ressource used by ADC
#define ADC0    26
#define ADC1    27
#define ADC2    28
#define ADC3    29
#define ADC_REF 3.3f


void  read_int_ADC(float *adc_val);
void  setup_ADC(bool enable);
void  read_ADC(float *adc_val);


#endif //

void scan_i2c_bus();

