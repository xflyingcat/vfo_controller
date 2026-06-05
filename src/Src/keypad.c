#include "stm32f1xx_hal.h"
#include <stdint.h>
#include "keypad.h"


KEYS_DESCRIPTION keys[KEYS_MAXIMUM] = {
  // PTT
  { 0, 0, 0, 10/*100ms debounce */, 0, 100 * 120 /*2 minutes protection*/, GPIOB, GPIO_PIN_0  },
  // STEP
  { 0, 0, 0, 10/*100ms debounce */, 0, 100 /* 1 second long pressing*/, GPIOB, GPIO_PIN_1  },
  // LOCK
  { 0, 0, 0, 10/*100ms debounce */, 0, 100 /* 1 second long pressing*/, GPIOB, GPIO_PIN_10 }
};

__weak void key_just_pressed_event(int i)
{

}

__weak void key_just_released_event(int i)
{

}

__weak void key_long_pressed_event(int i)
{

}


void keypad_processing(void)
{

  int i,bit;
    for(i=0; i<KEYS_MAXIMUM; i++)
    {
#ifdef LONG_PRESSING_USING
      if(keys[i].critical_timer > 0)
      {
          if(--keys[i].critical_timer==0)
          {
              key_long_pressed_event(i);
              keys[i].state = KEYPAD_AFTER_LONG_PRESSED;
          }
      }
#endif

      bit = !((keys[i].GPIOx->IDR & keys[i].GPIO_Pin) && 1);
      if(bit)
      {
          if(keys[i].counter < keys[i].max || keys[i].max == 0)
          {
              if(++(keys[i].counter) >= keys[i].max || keys[i].max == 0)
              {
                  if(keys[i].signal == 0)
                  {
#ifdef LONG_PRESSING_USING
                    if(keys[i].critical_time)
                      keys[i].critical_timer = keys[i].critical_time;
#endif
                    key_just_pressed_event(i);
                  }
                  keys[i].signal = 1;
              }
          }
      }
      else
      {
          if(keys[i].counter > 0 || keys[i].max == 0)
          {
              if(--(keys[i].counter) <= 0)
              {
                  if(keys[i].signal)
                  {
#ifdef LONG_PRESSING_USING
                    if(keys[i].state == KEYPAD_AFTER_LONG_PRESSED)
                    {
                      keys[i].state = KEYPAD_WAITING_FRONT;
                    }
                    else
#endif
                    key_just_released_event(i);

                    keys[i].critical_timer = 0;
                  }
                  keys[i].signal = 0;
              }
          }
      }
    }

}


