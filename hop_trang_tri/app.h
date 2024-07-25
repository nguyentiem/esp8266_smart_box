#pragma one
#include <stdint.h>
#include <inttypes.h>
typedef enum
{
    SHOW_TIME,
    SHOW_WEATHER,
    SHOW_IMAGE,
    SHOW_ADDR, 
} State_t;

typedef enum{ 
  LED_OFF, 
  LED_CHANGE,
  LED_EFFECT,
  
  }led_t; 

typedef struct action
{
    uint8_t state;
    uint8_t type;
} action;
