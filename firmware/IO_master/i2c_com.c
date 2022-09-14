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




bool send_master(uint8_t i2c_add,uint8_t cmd, uint16_t wdata, uint8_t *rback)  {

    // Writing to A register
    int count;
    uint8_t buf[3];
    int buflgth;   // contains size of the buffer

    buflgth = 2; 
    buf[0] = cmd;       // command      
    buf[1] = wdata;    // gpio


    printf("on sendmaster cmd: 0x%02d: add 0x%02x\r\n", cmd,i2c_add);
    count = i2c_write_blocking(i2c1, i2c_add, buf, buflgth, false);
    if (count < 0) {
        puts("Couldn't write Register to slave");
        printf("MAS: ERROR Write at register 0x%02d: %02d\n", buf[0], buf[1]);
        return false;
    }
    printf("MAS: Write at register 0x%02d: %02d\n", buf[0], buf[1]);


    uint8_t ird[2];
    i2c_write_blocking(i2c1, i2c_add, buf, 1, false);
    i2c_read_blocking(i2c1, i2c_add, ird, buflgth-1, false);

        
    printf("MAS:Read Register 0x%02d = %d \r\n", cmd,ird[0]);
    *rback = (uint8_t) ird[0];  // save readback value
    return true;     
}

/*  sub relay_execute
    From a list of relay (list) and the action to perform
    the sub will perform the action (close, open or read) for each relay on the list
*/

bool  relay_execute(uint16_t *list,uint8_t action, uint16_t *answer) {
    size_t i = 0;
    volatile uint8_t i2c_add,gpio,ser;
    volatile uint16_t relay;
    bool rfd, exf,se;
    bool seflg[4] = {false,false,false,false}; // SE flag to close or open the SE relay
    bool sestate[4] = {false,false,false,false}; // actual value of se relay state
    volatile int gpior[4][16]= RBK;  // table of gpio corresponding to relay
    uint8_t rdata = 0xff;

    printf("On relay execute begin \r\n");

    do {
            relay = list[i];
            rfd = false;    // set flag for relay found
            printf("Channel: %d \r\n,", list[i]);
            if (relay >= 100 && relay <= 115 || relay >= 10 && relay <= 17 ) {
                i2c_add = PICO_RELAY1_ADDRESS;   // assign card to send command
                if (relay >= 100) {
                    gpio = gpior[0][relay-100]; // remove offset and get gpio reference
                    if (relay > 107) { // if number require Se relay to be close
                        se = true;   // Close relay SE
                    } else { se = false;}  // open relay SE
                } else {
                    gpio = gpior[0][relay-10]; // remove offset and get gpio reference
                    se = false; // Open Relay SE
                }
                 // Check if Se relay need to be actuated
                if (seflg[0] == true) {   // if not the first actuation of SE relay
                    if (sestate[0] != se) {  // check if state changed
                        ser = SE_BK1;  // assign gpio for Single ended relay
                        sestate[0] = se; // update value of relay
                    } else { ser = 0;}  // value not changed, no need to toggle SE relay
                } else {
                    ser = SE_BK1;  // assign gpio for Single ended relay
                    sestate[0] = se; // update value of relay
                }
                seflg[0] = true;  // set flasg for se relay upodated
                rfd =true;  // set flag true (relay found)
            }

            if (relay >= 200 && relay <= 215 || relay >= 20 && relay <= 27 ) {
                i2c_add = PICO_RELAY1_ADDRESS;   // assign card to send command       
                if (relay >= 200) {
                    gpio = gpior[1][relay-200]; // remove offset and get gpio reference
                    // condition to toggle SE relay only one time by call
                    if (relay > 207 ) { // if number require Se relay to be close
                        se = true;   // Close relay SE
                    } else { se = false;}  // open relay SE
                } else {
                    gpio = gpior[1][relay-20]; // remove offset and get gpio reference
                    se = false;   // open relay SE
                }
                // Check if Se relay need to be actuated
                if (seflg[1] == true) {   // if not the first actuation of SE relay
                    if (sestate[1] != se) {  // check if state changed
                        ser = SE_BK2;  // assign gpio for Single ended relay
                        sestate[1] = se; // update value of relay
                    } else { ser = 0;}  // value not changed, no need to toggle SE relay
                } else {
                    ser = SE_BK2;  // assign gpio for Single ended relay
                    sestate[1] = se; // update value of relay
                }
                seflg[1] = true;  // set flasg for se relay upodated
                rfd =true;  // set flag true (relay found)
            }
            if (relay >= 300 && relay <= 315 || relay >= 30 && relay <= 37 ) {
                i2c_add = PICO_RELAY2_ADDRESS;   // assign card to send command
                if (relay >= 300) {
                    gpio = gpior[2][relay-300]; // remove offset and get gpio reference
                    if (relay > 307) { // if number require Se relay to be close
                        se = true;   // Close relay SE
                    } else { se = false;}  // open relay SE
                } else {
                    gpio = gpior[2][relay-30]; // remove offset and get gpio reference
                    se = false; // Open Relay SE
                }
                 // Check if Se relay need to be actuated
                if (seflg[3] == true) {   // if not the first actuation of SE relay
                    if (sestate[2] != se) {  // check if state changed
                        ser = SE_BK3;  // assign gpio for Single ended relay
                        sestate[2] = se; // update value of relay
                    } else { ser = 0;}  // value not changed, no need to toggle SE relay
                } else {
                    ser = SE_BK3;  // assign gpio for Single ended relay
                    sestate[2] = se; // update value of relay
                }
                seflg[2] = true;  // set flasg for se relay upodated
                rfd =true;  // set flag true (relay found)
            }
            if (relay >= 400 && relay <= 415 || relay >= 40 && relay <= 47 ) {
                i2c_add = PICO_RELAY2_ADDRESS;   // assign card to send command
                if (relay >= 400) {
                    gpio = gpior[3][relay-400]; // remove offset and get gpio reference
                    if (relay > 407) { // if number require Se relay to be close
                        se = true;   // Close relay SE
                    } else { se = false;}  // open relay SE
                } else {
                    gpio = gpior[3][relay-40]; // remove offset and get gpio reference
                    se = false; // Open Relay SE
                }
                 // Check if Se relay need to be actuated
                if (seflg[3] == true) {   // if not the first actuation of SE relay
                    if (sestate[3] != se) {  // check if state changed
                        ser = SE_BK4;  // assign gpio for Single ended relay
                        sestate[3] = se; // update value of relay
                    } else { ser = 0;}  // value not changed, no need to toggle SE relay
                } else {
                    ser = SE_BK4;  // assign gpio for Single ended relay
                    sestate[3] = se; // update value of relay
                }
                seflg[3] = true;  // set flasg for se relay upodated
                rfd =true;  // set flag true (relay found)
            }
           
            if (rfd == true) { // if relay is valid

              switch (action)
              {
                case RCLEX:
                case RCLOSE:
                   if (action == RCLEX) { // Open relay bank on exclusive command
                      send_master(i2c_add, OPEN_RELAY_BANK,gpio,&rdata);
                   }
                   send_master(i2c_add, CLOSE_RELAY, gpio,&rdata); // close required relay
                   if (ser > 0) { //if valid SE number
                        if (se) { // close or open the SE relay
                            send_master(i2c_add, CLOSE_RELAY, ser,&rdata);
                        } else {
                            send_master(i2c_add, OPEN_RELAY, ser,&rdata);
                        }
                    }

                break;

                case ROPEN:
                    send_master(i2c_add, OPEN_RELAY, gpio,&rdata); // open relay bank
                    if (ser > 0) { //if valid SE number
                        if (se) { // close or open the SE relay
                            send_master(i2c_add, CLOSE_RELAY, ser,&rdata);
                        } else {
                            send_master(i2c_add, OPEN_RELAY, ser,&rdata);
                        }
                    }

                break;

                case ROPALL:
                    send_master(i2c_add, OPEN_RELAY_BANK, gpio,&rdata); // open relay bank
                    if (ser > 0) { //if valid SE number
                        if (se) { // close or open the SE relay
                            send_master(i2c_add, CLOSE_RELAY, ser,&rdata);
                        } else {
                            send_master(i2c_add, OPEN_RELAY, ser,&rdata);
                        }
                    }
                break;

                case RSTATE:
                     send_master(i2c_add, STATE_RELAY, gpio,&rdata); // read required relay
                     answer[i] = rdata;

                case BSTATE:
                     send_master(i2c_add, STATE_BANK, gpio,&rdata); // read required bank
                     answer[i] = rdata;
                
                case SESTATE:
                     send_master(i2c_add, STATE_RELAY, ser,&rdata); // read required SE relay
                     answer[i] = rdata;

              }


            } else {
                printf ("Error relay execute \r\n");
                return false;
                // relay is not fund on list
                
            }

            i++;
        } while (list[i] > 0); // Loop for all relay on the list
        //answer[i] = 0;  // add flag to terminate the answer array

     
     puts("On relay execute end\r\n");

     return true;


}
