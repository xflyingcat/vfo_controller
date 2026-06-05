#include "main.h"
#include "utils.h"
#include <string.h>


uint64_t int32_to_packed_bcd(int32_t num) 
{
    uint64_t bcd = 0;
    int shift = 0;
    while (num > 0) {
        bcd |= (uint64_t)(num % 10) << shift;
        num /= 10;
        shift += 4;
    }
    return bcd;
}


uint32_t bcd_buf_to_int(uint64_t bcd)
{
   uint32_t freq = 0;
   uint32_t k = 1;
   int i;
           /*   4  5  6  7  8  9 10 11  */
           /*  00 40 07 07 00 00 00 00  */   
       for(i=0;i<10;i++) 
       {  
           freq += (bcd & 0x0F) * k;
           bcd >>= 4;
           k *= 10;
       } 
    
  return freq;
}










