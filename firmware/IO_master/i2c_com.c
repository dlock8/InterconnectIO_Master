#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"


#include "hardware/i2c.h"
#include "include/i2c_com.h"
#include "include/fts_scpi.h"





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

bool  relay_execute(int32_t *list,int8_t action) {
    size_t i = 0;
    volatile uint8_t i2c_add,gpio,relay,ser;
    bool rfd, exf;
    volatile int gpior[4][16]= RBK;  // table of gpio corresponding to relay

    fprintf(stdout, "On relay execute");


    do {
            relay = list[i];
            rfd = false;    // set flag for relay found
            fprintf(stdout, "Channel: %d,", list[i]);
            if (relay >= 100 && relay <= 115) {
                i2c_add = PICO_RELAY1_ADDRESS;   // assign card to send command
                gpio = gpior[0][relay-100]; // get gpio reference to execute  
                ser = SE_BK1;  // assign gpio for Single ended relay
                rfd =true;  // set flag true (relay found)
            }
            if (relay >= 200 && relay <= 215) {
                i2c_add = PICO_RELAY1_ADDRESS;   // assign card to send command
                gpio = gpior[1][relay-200]; // get gpio reference to execute  
                ser = SE_BK2;  // assign gpio for Single ended relay
                rfd =true;  // set flag true (relay found)  
            }
            if (relay >= 300 && relay <= 315) {
                i2c_add = PICO_RELAY2_ADDRESS;   // assign card to send command
                gpio = gpior[2][relay-300]; // get gpio reference to execute  
                ser = SE_BK3;  // assign gpio for Single ended relay
                rfd =true;  // set flag true (relay found) 
            }
            if (relay >= 400 && relay <= 415) {
                i2c_add = PICO_RELAY2_ADDRESS;   // assign card to send command
                gpio = gpior[3][relay-400]; // get gpio reference to execute
                ser = SE_BK4;  // assign gpio for Single ended relay
                rfd =true;  // set flag true (relay found)   
            }
        
            if (rfd == true) { // if relay is valid

              switch (action)
              {
                case RCLEX:
                case RCLOSE:
                   if (action == RCLEX) {
                      send_master(i2c_add, OPEN_RELAY_BANK, gpio);
                   }
                   send_master(i2c_add, CLOSE_RELAY, gpio);
                   if (relay % 2  != 0 ) { // if required close the SE relay
                      send_master(i2c_add, CLOSE_RELAY, ser);
                   }
                break;


                case ROPEN:
                    send_master(i2c_add, OPEN_RELAY, gpio);
                    if (relay % 2  != 0 ) { // if required open the SE relay
                      send_master(i2c_add, OPEN_RELAY, ser);
                   }
                break;
                
              }


            } else {
                // relay is not fund on list
            }

            i++;
        } while (list[i] > 0);
        //fprintf(stdout, "\r\n Channel List completed \r\n ");

     
     puts("On relay execute");

     return true;


}
