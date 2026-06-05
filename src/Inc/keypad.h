#ifndef __KEYPAD_H__
#define __KEYPAD_H__

/* Key codes definitions */
#define PTT  0
#define STEP 1
#define LOCK 2

#define KEYS_MAXIMUM  3


#define LONG_PRESSING_USING

enum {
  KEYPAD_FSM_JUST_STARTED = 0,
  KEYPAD_WAITING_FRONT,
  KEYPAD_AFTER_LONG_PRESSED
};

typedef struct {
int state;
int counter;
int signal;
int max;
int critical_timer;
int critical_time;
GPIO_TypeDef *GPIOx;
uint16_t GPIO_Pin;
} KEYS_DESCRIPTION;

void keypad_processing(void);

#endif