#ifndef __CAT_PROTO_H__
#define __CAT_PROTO_H__

enum {
   WAIT_START = 0,
   READ_HEADER,
   COLLECT_DATA
};

enum {
      CAT_EVENT_ACK,
      CAT_EVENT_NAK,  
      CAT_EVENT_FREQ_RCVD
};


#define ACK  0xFB
#define NAK  0xFA
#define START_FRAME  0xFE
#define END_FRAME    0xFD

#define TRCVR_ADDRESS 0xA4
#define THIS_MACHINE_ADDRESS 0xE0


void send_freq(uint32_t freq);
void send_freq_request(void);
void ptt_send(int state);
void cat_data_input_processing(void);
#endif