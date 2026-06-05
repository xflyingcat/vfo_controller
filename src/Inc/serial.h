#ifndef __SERIAL_H__
#define __SERIAL_H__

#define SIMPLE_FIFO_SIZE 128

typedef struct
{
  int   push_indx;
  int   pop_indx;
  unsigned char buff[SIMPLE_FIFO_SIZE];
  unsigned char dummy;
} SIMPLE_FIFO;

void serial_write(uint8_t *buf, int size);
int serial_get_buff(unsigned char* buff, int buffsize);

#endif


