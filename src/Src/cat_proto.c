#include "stm32f1xx_hal.h"
#include "serial.h"
#include "utils.h"
#include "cat_proto.h"
#include <stdint.h>
#include <string.h>


static unsigned char databuf[80];

__weak void cat_event(int ev, void *data)
{
}

void cat_data_input_processing(void)
{
    static int state = WAIT_START, start_cnt = 0, databuf_cnt;
    uint8_t bt;
    uint32_t freq;

    for(;;)
    {
       if(!serial_get_buff(&bt, 1))
          break;
       
       switch(state)
       {
          case WAIT_START:
            if(start_cnt == 0)
            {
              if(bt == START_FRAME)
                   start_cnt++;
            }
            else if(start_cnt == 1)
            {
              if(bt == START_FRAME)
              {  
                 state = COLLECT_DATA;
                 databuf_cnt = 0;
              }
              start_cnt = 0;
            }
          break;  
            
            
          case COLLECT_DATA:
            if(bt == START_FRAME)
            {
               start_cnt = 1;
               state = WAIT_START;
               break;
            }
            
            if(bt == END_FRAME)
            {
              if(databuf_cnt)
              {
                  //if(databuf[0] == TRCVR_ADDRESS)
                  if(databuf[0] == THIS_MACHINE_ADDRESS)
                  {
                      if((databuf[2] == 0x25) && (databuf[3] == 0x00))
                      {  
                          databuf[9]  = 0x00;
                          databuf[10] = 0x00;
                          databuf[11] = 0x00;
                          freq = bcd_buf_to_int(*((uint64_t*)&databuf[4]));
                          cat_event(CAT_EVENT_FREQ_RCVD, (void*)freq);
                      }
                      else if((databuf[2] == ACK))
                      {
                        cat_event(CAT_EVENT_ACK, NULL);
                      }
                      else if((databuf[2] == NAK))
                      {
                        cat_event(CAT_EVENT_NAK, NULL);
                      }
                  }
              }
              
              state = WAIT_START;
              start_cnt = 0;
              break;
            }
            if(databuf_cnt < 80)
            {
              databuf[databuf_cnt++] = bt;
            }
            
          break;  
       }
    }
}

void send_freq(uint32_t freq)
{
    static uint8_t tx_buf[16];
    uint64_t freq_bcd = int32_to_packed_bcd(freq); 
    tx_buf[0] = START_FRAME;
    tx_buf[1] = START_FRAME;
    tx_buf[2] = TRCVR_ADDRESS;
    tx_buf[3] = THIS_MACHINE_ADDRESS;
    tx_buf[4] = 0x25;
    tx_buf[5] = 0x00;
    memcpy(&tx_buf[6],&freq_bcd, 5);
    tx_buf[11] = END_FRAME;
    serial_write(tx_buf, 12);  
}

void send_freq_request(void)
{
    static uint8_t tx_buf[16];
    tx_buf[0] = START_FRAME;
    tx_buf[1] = START_FRAME;
    tx_buf[2] = TRCVR_ADDRESS;
    tx_buf[3] = THIS_MACHINE_ADDRESS;
    tx_buf[4] = 0x25;
    tx_buf[5] = 0x00;
    tx_buf[6] = END_FRAME;
    serial_write(tx_buf, 7);  
}

void ptt_send(int state)
{
    static uint8_t tx_buf[16];
    tx_buf[0] = START_FRAME;
    tx_buf[1] = START_FRAME;
    tx_buf[2] = TRCVR_ADDRESS;
    tx_buf[3] = THIS_MACHINE_ADDRESS;
    tx_buf[4] = 0x1C;
    tx_buf[5] = 0x00;
    tx_buf[6] = state;
    tx_buf[7] = END_FRAME;
    serial_write(tx_buf, 8);  
}



