// Copyright 2021 Ocean (iiot2k@gmail.com) 
// All rights reserved.

#include "dev_mcp4725.h"

#define WRITEDAC        0x40
#define WRITEDACEEPROM  0x60

static uint8_t dac_pd = MCP4725_PD_OFF;
static float vref = VDD;

void dev_mcp4725_pd(uint8_t pd)
{
	dac_pd = pd;
}

static bool mcp4725_write(i2c_inst_t* i2c, uint8_t cmd, uint8_t addr, uint16_t value)
{
	if (value > 4095)
		value = 4095;
	
	uint8_t buffer[3];

	buffer[0] = cmd | (dac_pd << 1);
	buffer[1] = value >> 4;
	buffer[2] = value << 4;

	// set channel dac
    return (sys_i2c_wbuf(i2c, addr, buffer, sizeof(buffer)) == sizeof(buffer));
}

bool dev_mcp4725_set_raw(i2c_inst_t* i2c, uint8_t addr, uint16_t value)
{
	return mcp4725_write(i2c, WRITEDAC, addr, value);
}

bool dev_mcp4725_set(i2c_inst_t* i2c, uint8_t addr, float volt)
{
	uint16_t value = 0;

	value = volt / (vref/4096);  // 12 bits DAC
	if (value > 4095) { value = 4095;}  //if value outside limit, cap the limit


	return mcp4725_write(i2c, WRITEDAC, addr, value);
}

bool dev_mcp4725_save(i2c_inst_t* i2c, uint8_t addr, float volt)
{
	uint16_t value = 0;

	value = volt / (vref/4096);  // 12 bits DAC
	if (value > 4095) { value = 4095;}  //if value outside limit, cap the limit

	return mcp4725_write(i2c, WRITEDACEEPROM, addr, value);
}

float dev_mcp4725_get(i2c_inst_t* i2c, uint8_t addr)
{
	uint8_t value[5];
	uint16_t tval;
	float volt;
	
	sys_i2c_rbuf(i2c, addr, value, sizeof(value)); // Read 5 bytes
	tval = (value[1] << 4) + (value[2] >> 4);  // transform 8 bits value in 12 bits result

	volt = tval * (vref/4096);  // transform to voltage value

    return volt;
}