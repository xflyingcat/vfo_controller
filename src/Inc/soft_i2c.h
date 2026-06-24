#ifndef __I2C_H__
#define __I2C_H__

void soft_i2c_init(void);
void soft_i2c_deinit(void);
void soft_i2c_start(void);
void soft_i2c_stop(void);
int soft_i2c_write(unsigned char bt);
unsigned char soft_i2c_read(unsigned char ack);


#endif

