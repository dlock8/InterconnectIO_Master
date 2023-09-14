// Copyright 2021 Ocean (iiot2k@gmail.com) 
// All rights reserved.

#include "hardware/adc.h"
#include "sys_adc.h"

static bool isadc_init = false;

void sys_adc_init(uint8_t ch)
{
    // init adc
    if (!isadc_init)
    {
        adc_init();
        isadc_init = true;
    }

    switch(ch)
    {
        case ADC_CH_0:
            adc_gpio_init(26);
            break;
        case ADC_CH_1:
            adc_gpio_init(27);
            break;
        case ADC_CH_2:
            adc_gpio_init(28);
            break;
        case ADC_CH_V: // VSYS voltage
            adc_gpio_init(29);
            break;
        case ADC_CH_T: // internal temp. sensor
            adc_set_temp_sensor_enabled(true);
            break;
    }
}

uint16_t sys_adc_raw(uint8_t ch)
{
    // check if init
    if (!isadc_init)
        return 0;

    if (ch > ADC_CH_T)
        return 0;

    // Select ADC input
    adc_select_input(ch);

    // read from adc
    return adc_read();
}

double sys_adc_scale(uint8_t ch, double low, double high)
{
    return ((double)sys_adc_raw(ch)) * ((high - low) / 4095.0) + low;
}

double sys_adc_volt(uint8_t ch)
{
    return ((double)sys_adc_raw(ch)) * (ADC_VREF/4095.0);
}

double sys_adc_vsys()
{
    return sys_adc_volt(ADC_CH_V) * 3.0;
}

double sys_adc_temp_c()
{
    return 27.0 - ((sys_adc_volt(ADC_CH_T) - 0.706)/0.001721);
}

double sys_adc_temp_f()
{
    return sys_adc_temp_c() * 1.8 + 32;
}