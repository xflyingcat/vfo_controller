#include "includes.h"

extern TIM_HandleTypeDef htim1;

int ptt_flag = 0;
int lock_flag = 0;
uint32_t freq = 7012340;
int step_value = 10; // default step 10Hz

int32_t get_delta(void);

void setup(void)
{
  HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
}

void loop(void)
{
  static uint32_t _50ms,_10ms, startup_flag = 0, to_timer = 0;
  uint32_t delta;
  if(startup_flag == 0)
  {  
     // Executed once during startup
     send_freq_request(); // Request the current transceiver frequency
                          //  for sychronization  
    _50ms = HAL_GetTick() + 50; // Initialize 50ms interval
    _10ms = HAL_GetTick() + 10; // Initialize 10ms interval
     startup_flag = 1;
  }   
    /* Task #1 Receiving data from transceiver using CAT protocol */
    cat_data_input_processing(); // CAT protocol FSM (see FSM callback:
                                 // cat_event())   
  

    /* Task #2 Keys short/Long pressing detecting, debounce */
    if(HAL_GetTick() > _10ms) // We've had the 10ms interval triggered again
    {
      _10ms = HAL_GetTick() + 10; // maintaining an interval of 10 mS   
    
       keypad_processing(); // Keypad FSM (see FSM callbacks:
                            // key_just_pressed_event(),
                            // key_just_released_event(),
                            // key_long_pressing_event())
    }
    
    
    /* Task #3 Encoder processing, send data to transceiver */
    if(HAL_GetTick() > _50ms) // We've had the 50ms interval triggered again
    {
      _50ms = HAL_GetTick() + 50; // maintaining an interval of 50 mS 

      // We read how many encoder pulses the timer 
      // has counted during a 50 ms interval.
      delta = get_delta();
      if(delta)
      {
        to_timer = 0; // restart time out timer
        freq += delta*step_value; // updated frequency value
        send_freq(freq); // update transceiver frequency by CAT
      }
      else
      { // no encoder rotation - no delta
        if(++to_timer >= 10) // idle timer for 500ms
        {  // timed_out
           to_timer = 0;
           // request to get the current transceiver
           // frequency for synchronization
           send_freq_request();  
        }
      }
    }
}


void cat_event(int ev, void *data)
{
   switch(ev)
   {
     case CAT_EVENT_ACK:
     break;
     
     case CAT_EVENT_NAK:
     break;
     
     case CAT_EVENT_FREQ_RCVD:
       freq = (uint32_t)data;
     break;
   }
}

void key_just_pressed_event(int i)
{
  switch(i)
  {
     case PTT:
       ptt_flag = 1;
       // PTT LED turn on
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
       ptt_send(1);
       break;

    case STEP:

       
       break;

    case LOCK:

         
       break;
  }
}

void key_just_released_event(int i)
{
  switch(i)
  {
     case PTT:
       ptt_flag = 0;
       ptt_send(0);
       // PTT LED turn off
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
       break;

    case STEP:
       break;

    case LOCK:
         lock_flag ^= 1;
         // Lock LED 
         HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, (GPIO_PinState)!(lock_flag == 1));    
       break;
  }
}

void key_long_pressed_event(int i)
{
  switch(i)
  {
     case PTT:
       ptt_flag = 0;
       ptt_send(0);
       // PTT LED turn off
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);       
       break;

    case STEP:
       break;

    case LOCK:
       if(step_value == 10)
       {
         step_value = 1;
         // 1 Hz LED  turn on
         HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
       }
       else if(step_value == 1)
       {
         step_value = 10;
         freq = (freq / 10) * 10;
         // 1 Hz LED  turn off
         HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
       }        
       break;
  }
}

int32_t get_delta(void)
{
    static int32_t prev_cntr = 0;
    int16_t curr_cntr = __HAL_TIM_GET_COUNTER(&htim1);
    if(curr_cntr != prev_cntr)
    {
        int32_t delta = prev_cntr - curr_cntr;
        prev_cntr = curr_cntr;

        if(ptt_flag)
           return 0;
        
        if(lock_flag)
           return 0;
        

        if((delta > -1000) && (delta < 1000))
        {
            return delta;
        }
    }
    return 0;
}

