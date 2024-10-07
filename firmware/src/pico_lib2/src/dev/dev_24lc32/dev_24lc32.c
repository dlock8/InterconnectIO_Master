/*
 * Copyright (c) 2022, Mezael Docoy
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "dev_24lc32.h"
#include "stdio.h"

void at24cx_i2c_device_register(at24cx_dev_t* dev, uint16_t _dev_chip, uint8_t _i2c_addres)
{
  dev->dev_chip = _dev_chip;
  dev->byte_size = (128 * _dev_chip) - 1;
  dev->i2c_addres = _i2c_addres;
  dev->status = 0;
  switch (_dev_chip)
  {
    case 512:
      dev->page_write_size = 128;
      break;
    case 256:
      dev->page_write_size = 64;
      break;
    case 128:
      dev->page_write_size = 64;
      break;
    case 32:
      dev->page_write_size = 32;
      break;
  }
  uint8_t rxdata;
  int ret;
  ret = sys_i2c_rbyte(i2c0, dev->i2c_addres, &rxdata);
  if (ret == 1)
  {
    dev->status = 1;
  }
  fprintf(stdout, "Device registered. Status: %s, Chip: AT24C%d, Address: 0x%02X, Size: %d\n", dev->status ? "Active" : "Inactive", dev->dev_chip,
          dev->i2c_addres, dev->byte_size);
}

static at24cx_err_t at24cx_i2c_error_check(at24cx_dev_t* dev, at24cx_writedata_t* dt)
{
  if (!dev->status)
    return AT24CX_NOT_DETECTED;
  else if (dt->address > dev->byte_size)
    return AT24CX_INVALID_ADDRESS;
  else
    return AT24CX_OK;
}

at24cx_err_t at24cx_i2c_byte_write(at24cx_dev_t dev, at24cx_writedata_t dt)
{
  at24cx_err_t err;
  uint8_t valid;
  uint8_t data[3];
  uint8_t eead[2];

  data[0] = dt.address >> 8;
  data[1] = dt.address & 0xFF;
  data[2] = dt.data;

  err = at24cx_i2c_error_check(&dev, &dt);
  if (err != AT24CX_OK) return err;

  if (sys_i2c_wbuf(i2c0, dev.i2c_addres, data, sizeof(data)) == sizeof(data))
  {
    sleep_ms(AT24CX_WRITE_CYCLE_DELAY);
    return AT24CX_OK;
  }

  return err;
}

at24cx_err_t at24cx_i2c_page_write(at24cx_dev_t dev, at24cx_writedata_t dt)
{
  at24cx_err_t err;
  uint8_t data[130];
  data[0] = dt.address >> 8;
  data[1] = dt.address & 0xFF;

  for (int i = 0; i < dev.page_write_size; i++) data[2 + i] = dt.data_multi[i];

  err = at24cx_i2c_error_check(&dev, &dt);
  if (err != AT24CX_OK) return err;

  if (((dt.address + dev.page_write_size - 1) > dev.byte_size) || (dt.address) % dev.page_write_size)
  {
    sleep_ms(AT24CX_WRITE_CYCLE_DELAY);
    return AT24CX_INVALID_PAGEWRITE_ADDRESS;
  }

  if (sys_i2c_wbuf(i2c0, dev.i2c_addres, data, dev.page_write_size + 2) == (dev.page_write_size + 2))
  {
    sleep_ms(AT24CX_WRITE_CYCLE_DELAY);
    return AT24CX_OK;
  }

  return err;
}

at24cx_err_t at24cx_i2c_byte_read(at24cx_dev_t dev, at24cx_writedata_t* dt)
{
  at24cx_err_t err;
  uint8_t reg[2];
  reg[0] = dt->address >> 8;
  reg[1] = dt->address & 0xFF;
  uint8_t ird;

  err = at24cx_i2c_error_check(&dev, dt);
  if (err != AT24CX_OK) return err;

  if (sys_i2c_rbyte_eeprom(i2c0, dev.i2c_addres, reg, &ird, 1) == 1)
  {
    dt->data = ird;
    return AT24CX_OK;
  }

  return err;
}

// Not working properly, return strange data.
// Need to debug but i have no use to it
at24cx_err_t at24cx_i2c_current_address_read(at24cx_dev_t dev, at24cx_writedata_t* dt)
{
  at24cx_err_t err;
  uint8_t data;

  err = sys_i2c_rbyte(i2c0, dev.i2c_addres, &data);
  dt->data = data;

  return err;
}
