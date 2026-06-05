#include "includes.h"
#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef huart1;

static SIMPLE_FIFO uart1_rx_fifo;

static int fifo_pop(SIMPLE_FIFO *fifo, unsigned char *data)
{
    int indx;
    if (fifo->push_indx == fifo->pop_indx)
        return 0; // fifo is empty
    indx = fifo->pop_indx + 1;
    indx = indx % SIMPLE_FIFO_SIZE;
    *data = fifo->buff[indx];
    fifo->pop_indx = indx;
    return 1;
}

static int fifo_push(SIMPLE_FIFO *fifo, unsigned char data)
{
    int indx;
    indx = fifo->push_indx + 1;
    indx = indx % SIMPLE_FIFO_SIZE;

    if (indx == fifo->pop_indx)
        return 0; // fifo is full
    fifo->buff[indx] = data;
    fifo->push_indx = indx;
    return 1;
}


void serial_write(uint8_t *buf, int size)
{
   HAL_UART_Transmit_DMA(&huart1, buf, size); 
}

int serial_get_buff(unsigned char* buff, int buffsize)
{
  unsigned int cnt;

  UART_HandleTypeDef *huart = &huart1;

  for (cnt = 0; cnt < buffsize; ++cnt)
  {

   __disable_irq();

    if(!fifo_pop(&uart1_rx_fifo, buff + cnt))
    {
      HAL_UART_Receive_IT(huart, &(uart1_rx_fifo.dummy), 1);
      __enable_irq();
      break;
    }

    __enable_irq();
  }
  return cnt;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart == &huart1)
  {
    fifo_push(&uart1_rx_fifo,(huart->Instance->DR & 0xFF));
    HAL_UART_Receive_IT((UART_HandleTypeDef*)huart, &(uart1_rx_fifo.dummy), 1);
  }
}

