#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdint.h>

uint64_t int32_to_packed_bcd(int32_t num);
uint32_t bcd_buf_to_int(uint64_t bcd);

#endif

