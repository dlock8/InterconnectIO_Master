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

    i2c_init(i2c0, I2C_BAUDRATE);
}




bool send_master(uint8_t i2c_add,uint8_t cmd, uint16_t wdata, uint16_t *rback)  {

    // Writing to A register
    int count;
    uint8_t buf[3];
    int buflgth;   // contains size of the buffer

    buflgth = 2; 
    buf[0] = cmd;       // command      
    buf[1] = wdata;    // gpio


    fprintf(stdout,"on sendmaster cmd: 0x%02d: add 0x%02x\r\n", cmd,i2c_add);
    count = i2c_write_blocking(i2c0, i2c_add, buf, buflgth, false);
    if (count < 0) {
        //puts("Couldn't write Register to slave");
        fprintf(stdout,"MAS: ERROR Write at register 0x%02d: %02d\n", buf[0], buf[1]);
        *rback = SCPI_I2C_COMMUNICATION_SLAVE;  // return error number to caller

        return false; // set flag to indicate error (error number on rback)
    }
    fprintf(stdout,"MAS: Write at register 0x%02d: %02d\n", buf[0], buf[1]);

   //read register value and return to caller on pointer rback    
    uint8_t ird[2];
    i2c_write_blocking(i2c0, i2c_add, buf, 1, false);
    i2c_read_blocking(i2c0, i2c_add, ird, buflgth-1, false);
    
    fprintf(stdout,"MAS:Read Register 0x%02d = %d \r\n", cmd,ird[0]);
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
    bool rfd,smf, exf,se;
    bool seflg[4] = {false,false,false,false}; // SE flag to close or open the SE relay
    bool sestate[4] = {false,false,false,false}; // actual value of se relay state
    volatile int gpior[4][16]= RBK;  // table of gpio corresponding to relay
    uint16_t rdata;

    fprintf(stdout,"On relay execute begin \r\n");

    do {
            relay = list[i];
            rfd = false;    // set flag for relay found
            printf("Channel: %d ,\r\n", list[i]);
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
                seflg[1] = true;  // set flasg for se relay updated
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
                seflg[3] = true;  // set flasg for se relay updated
                rfd =true;  // set flag true (relay found)
            }

            if (relay >= 500  ) {  // Power relay
                gpio = relay-500; // remove offset and get gpio reference
                if (gpio== GPIO_LPR1 || gpio== GPIO_LPR2 || gpio== GPIO_HPR1 || gpio== GPIO_SSD1) {
                  rfd =true;  // set flag true (power relay valid)
                }
            }
           
            if (rfd == true) { // if relay is valid

              switch (action)
              {
                case RCLEX:
                case RCLOSE:
                   if (action == RCLEX) { // Open relay bank on exclusive command
                      smf = send_master(i2c_add, OPEN_RELAY_BANK,gpio,&rdata);
                      if (!smf) { 
                        answer[0] = rdata;  // save error on answer
                        return false;       // return
                      }   
                   }
                   smf = send_master(i2c_add, CLOSE_RELAY, gpio,&rdata); // close required relay
                   if (!smf) { answer[0] = rdata; return false;}  // Save error and return
                   if (ser > 0) { //if valid SE number
                        if (se) { // close or open the SE relay
                            smf = send_master(i2c_add, CLOSE_RELAY, ser,&rdata);
                            if (!smf) { answer[0] = rdata; return false;}
                        } else {
                            smf = send_master(i2c_add, OPEN_RELAY, ser,&rdata);
                            if (!smf) { answer[0] = rdata; return false;}
                        }
                    }

                break;

                case ROPEN:
                    smf = send_master(i2c_add, OPEN_RELAY, gpio,&rdata); // open relay bank
                    if (!smf) { answer[0] = rdata; return false;}  // Save error and return
                    if (ser > 0) { //if valid SE number
                        if (se) { // close or open the SE relay
                            smf =send_master(i2c_add, CLOSE_RELAY, ser,&rdata);
                            if (!smf) { answer[0] = rdata; return false;}
                        } else {
                            smf= send_master(i2c_add, OPEN_RELAY, ser,&rdata);
                            if (!smf) { answer[0] = rdata; return false;}
                        }
                    }

                break;

                case ROPALL:
                    smf = send_master(i2c_add, OPEN_RELAY_BANK, gpio,&rdata); // open relay bank
                    if (!smf) { answer[0] = rdata; return false;}  // Save error and return
                    if (ser > 0) { //if valid SE number
                        if (se) { // close or open the SE relay
                            smf= send_master(i2c_add, CLOSE_RELAY, ser,&rdata);
                            if (!smf) { answer[0] = rdata; return false;}
                        } else {
                            smf= send_master(i2c_add, OPEN_RELAY, ser,&rdata);
                            if (!smf) { answer[0] = rdata; return false;}
                        }
                    }
                break;

                case RSTATE:
                     smf= send_master(i2c_add, STATE_RELAY, gpio,&rdata); // read required relay
                     if (!smf) { answer[0] = rdata; return false;}  // Save error and return
                     answer[i] = rdata;
                     break;

                case BSTATE:
                     smf= send_master(i2c_add, STATE_BANK, gpio,&rdata); // read required bank
                     if (!smf) { answer[0] = rdata; return false;}  // Save error and return
                     answer[i] = rdata;
                     break;
                
                case SESTATE:
                     smf= send_master(i2c_add, STATE_RELAY, ser,&rdata); // read required SE relay
                     if (!smf) { answer[0] = rdata; return false;}  // Save error and return
                     answer[i] = rdata;
                     break;

                
                case PWCLOSE:
                     gpio_put(gpio,1);  // Close Gpio on master device
                     fprintf(stdout,"MAS: CLOSE PWR relay using gpio: %02d\n", gpio);
                     break;   

                case PWOPEN:
                     gpio_put(gpio,0);  // Close Gpio on master device
                     fprintf(stdout,"MAS: OPEN PWR relay using gpio: %02d\n", gpio);
                     break; 

                case PWSTATE:
                     answer[i] = gpio_get(gpio);   // Read true Value
                     fprintf(stdout,"MAS: STATE PWR relay using gpio: %02d, State: %01d\n", gpio, answer[i]);
                     break;    

              


              } // end switch (action)


            } else {
                fprintf (stdout,"Error relay numbering (channel not valid)  \r\n");
                answer[0] = RELAY_NUMBERING_ERROR;
                return false;
                // relay is not fund on list
                
            }

            i++;
        } while (list[i] > 0); // Loop for all relay on the list
        //answer[i] = 0;  // add flag to terminate the answer array

     
     fprintf(stdout,"On relay execute end\r\n");

     return true;
}

bool  digital_execute(uint8_t action, uint8_t port, uint8_t bit, uint8_t value, uint16_t *answer) {
    size_t i;
    volatile int gpiod[2][8]= DIGP;
    bool smf;
    uint16_t rdata;
    uint8_t  command;
    uint8_t  gp, portd;

    fprintf(stdout,"On digital execute begin\r\n");

    switch (action)
    {
        case SDIR:
          command = DIG_DIR_MASK + (port * 10); // change command number following port selected
          smf= send_master(PICO_PORT_ADDRESS, command, value,&rdata); // send command
          if (!smf) { answer[0] = rdata; return false;}  // Save error and return
          break;

        case RDIR: // command to read port direction exist only by bit and not by port.
          command =  DIR_GP_READ;  // command to send to read direction
          portd = 0;   // register to save value read
          for (i = 0; i <= 7; i++) {  // loop to read each bit of the port
              gp = gpiod[port][i]; // get Gpio associated to bit and port
              smf= send_master(PICO_PORT_ADDRESS, command, gp,&rdata); // send command
              if (!smf) { answer[0] = rdata; return false;}  // Save error and return
              portd += (rdata << i); // save value based on bit position  
          }
          answer[0]= portd;
          break;

        case SBDIR: // read gpio direction
          gp = gpiod[port][bit]; // get Gpio associated to bit and port
          if (value) {          // set command based on direction direction 
            command = DIR_GP_OUT;  // command number for set GPIO direction OUT
          } else { 
            command = DIR_GP_IN; // command number for set GPIO direction IN
          }
          smf= send_master(PICO_PORT_ADDRESS, command, gp,&rdata); // send command
          if (!smf) { answer[0] = rdata; return false;}  // Save error and return
          break;
        
        case RBDIR:
          command =  DIR_GP_READ;  // command to send to read direction
          gp = gpiod[port][bit]; // get Gpio associated to bit and port
          smf= send_master(PICO_PORT_ADDRESS, command, gp,&rdata); // send command
          if (!smf) { answer[0] = rdata; return false;}  // Save error and return
          answer[0]= rdata; // return answer
          break;

        case SOUT:
          command = DIG_OUT + (port * 10); // change command number following port selected
          smf= send_master(PICO_PORT_ADDRESS, command, value,&rdata); // send command
          if (!smf) { answer[0] = rdata; return false;}  // Save error and return
          break;

        case SBOUT:  // set Bit 
          gp = gpiod[port][bit]; // get Gpio associated to bit and port
          if (value) {          // set command based on direction direction 
            command = DIG_GP_OUT_SET;  // command number for set GPIO at High
          } else { 
            command = DIG_GP_OUT_CLEAR; // command number for set GPIO at low
          }
          smf= send_master(PICO_PORT_ADDRESS, command, gp,&rdata); // send command
          if (!smf) { answer[0] = rdata; return false;}  // Save error and return
          break;

        case RIN:
          command = DIG_IN + (port * 10); // change command number following port selected
          smf= send_master(PICO_PORT_ADDRESS, command, value,&rdata); // send command
          if (!smf) { answer[0] = rdata; return false;}  // Save error and return
          answer[0] = rdata;  // return read value
          break;

        case RBIN:
          command = DIG_GP_IN; // command to sent for read gpio
          gp = gpiod[port][bit]; // get Gpio associated to bit and port
          smf= send_master(PICO_PORT_ADDRESS, command, gp,&rdata); // send command
          if (!smf) { answer[0] = rdata; return false;}  // Save error and return
          answer[0] = rdata;  // return read value
          break;

    }

    fprintf(stdout,"On digital execute end\r\n");
    return true;
}

bool  gpio_execute(uint8_t action, uint8_t device, uint8_t gpio, uint8_t value, uint16_t *answer) {
    bool smf;
    uint8_t  command;
    int address[4] = {PICO_MASTER_ADDRESS,PICO_PORT_ADDRESS,PICO_RELAY1_ADDRESS,PICO_RELAY2_ADDRESS};
    uint8_t slave, pval;
    uint32_t  maskvalue;
    bool rval;

    fprintf(stdout,"On gpio execute begin\r\n");


    switch (action)
    {
        case GPSDIR: // Set GPIO Direction
            slave = address[device]; /// Set I2C address
            if (value) {          // set command based on direction requested
                command = DIR_GP_OUT;  // command number for set GPIO direction OUT
            } else { 
                command = DIR_GP_IN; // command number for set GPIO direction IN
            }
          
            if (slave == PICO_MASTER_ADDRESS) {
                gpio_set_dir(gpio,1);  // send direct command to set direction = output
                fprintf(stdout,"Cmd %02d, Set Dir Out Gpio: %02d \r\n ", command, gpio);
            } else {
                smf= send_master(slave, command, gpio,answer); // send i2c command to slave
                if (!smf) { return false;}  // Error return
            }
            break;

        case GPRDIR: // Get GPIO Direction
            slave = address[device]; /// Set I2C address
            command =  DIR_GP_READ;  // command to send to read direction
            if (slave == PICO_MASTER_ADDRESS) {
                rval = gpio_get_dir(gpio);  // send direct command to read direction
                answer[0] = rval;
                fprintf(stdout,"Cmd %02d, read Direction Gpio: %02d. State: %01d \r\n ", command, gpio,rval);
            } else {
                smf= send_master(slave, command, gpio,answer); // send i2c command to slave
                if (!smf) { return false;}  // Error return
            }
            break;

        case GPOUT:// Set GPIO Output State
            slave = address[device]; /// Set I2C address
            if (value) {          // set command based on direction direction 
                command = DIG_GP_OUT_SET;  // command number for set GPIO at High
            } else { 
                command = DIG_GP_OUT_CLEAR; // command number for set GPIO at low
            }

            if (slave == PICO_MASTER_ADDRESS) {
                gpio_put(gpio,value);  // send direct command to read direction
                fprintf(stdout,"Cmd %02d, Set Output Gpio: %02d. State: %01d \r\n ", command, gpio,value);
            } else {
                smf= send_master(slave, command, gpio,answer); // send i2c command to slave
                if (!smf) { return false;}  // Error return
            }
            break;

        case GPIN: // Get GPIO Input state
            slave = address[device]; /// Set I2C address
            command =  DIG_GP_IN;  // command to send to read gpio value
            if (slave == PICO_MASTER_ADDRESS) {
                rval = gpio_get(gpio);  // send direct command to read gpio state
                answer[0] = rval;
                fprintf(stdout,"Cmd %02d, read value Gpio: %02d. State: %01d \r\n ", command, gpio,rval);
            } else {
                smf= send_master(slave, command, gpio,answer); // send i2c command to slave
                if (!smf) { return false;}  // Error return
            }
            break;

        case GPSPAD: // Set GPIO PAD State
            slave = address[device]; /// Set I2C address
            maskvalue = 0xfful; // change only the lower 8 bits
            command =  GP_PAD_SET;  // command to send to read gpio value
            if (slave == PICO_MASTER_ADDRESS) {
                hw_write_masked(&padsbank0_hw ->io[gpio],value,maskvalue); // Set Pad state
                fprintf(stdout,"Cmd %02d, Set Pad State to Gpio: %02d ,State: 0x%01x \r\n",command,  gpio, value);
            } else { // 2 commands required to set PAD value
                smf= send_master(slave, GP_PAD_VALUE, gpio,answer); // send i2c command to slave
                if (!smf) { return false;}  // Error return
                smf= send_master(slave, command, gpio,answer); // send i2c command to slave
                if (!smf) { return false;}  // Error return
            }
            break;
            
        case GPGPAD: // Read GPIO PAD State
            slave = address[device]; /// Set I2C address
            maskvalue = 0xfful; // change only the lower 8 bits
            command =  GP_PAD_READ;  // command to send to read gpio pad value
            if (slave == PICO_MASTER_ADDRESS) {
                pval =  padsbank0_hw ->io[gpio] &maskvalue; // Read gpio PAD Value
                answer[0] = pval;  // save value to be returned
                fprintf(stdout,"Cmd %02d, Gpio: %02d ,Read PAD State: 0x%01x \r\n",command,  gpio,pval);
            } else {
                smf= send_master(slave, command, gpio,answer); // send i2c command to slave
                if (!smf) { return false;}  // Error return
            }
    }


    fprintf(stdout,"On gpio execute end\r\n");
    return true;
}
