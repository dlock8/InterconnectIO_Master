#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"


#include "hardware/i2c.h"
#include "include/i2c_com.h"





void setup_master() {
    gpio_init(I2C_MASTER_SDA_PIN);
    gpio_set_function(I2C_MASTER_SDA_PIN, GPIO_FUNC_I2C);
    // pull-ups are already active on slave side, this is just a fail-safe in case the wiring is faulty
    gpio_pull_up(I2C_MASTER_SDA_PIN);

    gpio_init(I2C_MASTER_SCL_PIN);
    gpio_set_function(I2C_MASTER_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_MASTER_SCL_PIN);

    i2c_init(i2c1, I2C_BAUDRATE);
}




void send_master(uint8_t i2c_add,uint8_t cmd, uint16_t wdata)  {

    // Writing to A register
    int count;
    uint8_t buf[3];
    int buflgth;   // contains size of the buffer

    buflgth = 2; 
    buf[0] = cmd;       // command      
    buf[1] = wdata;    // gpio



    count = i2c_write_blocking(i2c1, i2c_add, buf, buflgth, false);
    if (count < 0) {
        puts("Couldn't write Register to slave");
        return;
    }
    printf("MAS: Write at register 0x%02d: %02d\n", buf[0], buf[1]);


    uint8_t ird[2];
    i2c_write_blocking(i2c1, i2c_add, buf, 1, false);
    i2c_read_blocking(i2c1, i2c_add, ird, buflgth-1, false);

        
    printf("MAS:Read Register 0x%02d = %d \r\n", cmd,ird[0]);
        
          
}

