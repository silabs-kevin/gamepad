/*************************************************************************
    > File Name: joystick.h
    > Author: Kevin
    > Created Time: 2020-09-02
    > Description:
 ************************************************************************/

#ifndef JOYSTICK_H
#define JOYSTICK_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>

#include "sl_bluetooth.h"
#include "em_gpio.h"
#include "em_adc.h"
#include "inc/utils.h"

#define JSTK_NUM  2
#define ADC_CHANNEL_NUM (JSTK_NUM * 2)
// Init to max ADC clock for Series 1 with AUXHFRCO
#define ADC_FREQ        4000000
// Desired letimer interrupt frequency (in Hz)
#define LETIMERDESIRED  1000

#define REF_MILLI_VOLT  2500
#define ADC_RESOLUTION  4096

#define LDMA_CHANNEL    0
#define PRS_CHANNEL     0

#define JSTK_VAL_MAX  (127)
#define JSTK_VAL_MIN  (-127)

#define JOYSTICK_UPDATE_EVT (1 << 1UL)

enum {
  JSTK_LEFT,
  JSTK_RIGHT,
};

enum {
  AXIS_X,
  AXIS_Y,
  AXIS_Z,
  AXIS_RZ
};

typedef struct {
  GPIO_Port_TypeDef port;
  unsigned int pin;
  ADC_PosSel_TypeDef pos;
}gpio_t;

typedef struct {
  int8_t val;
  const gpio_t *gpio;
}axis_t;

typedef void (*jstk_init_fn)(uint16_t freq,
                             uint8_t enable);
typedef void (*jstk_start_fn)(void);
typedef void (*jstk_stop_fn)(void);

enum {
  nil,
  init_ed,
  enabled,
};

typedef struct {
  uint16_t freq;
}jstk_timer_t;

typedef struct {
  uint8_t ldma_channel;
}jstk_peri_config_t;

/*
 * Joystick bitmap operations
 */
#define TIMER_BITMASKS  (0xC0)
#define TIMER_STATE(x)  (((x) & TIMER_BITMASKS) >> 6)
#define TIMER_TO_NIL(x) ((x) = (x) & ~TIMER_BITMASKS)
#define TIMER_TO_INITED(x) BIT_SET((x), 6); BIT_CLR((x), 7)
#define TIMER_TO_STARTED(x) BIT_SET((x), 6); BIT_SET((x), 7)

#define ADC_BITMASKS  (0x30)
#define ADC_STATE(x)  (((x) & ADC_BITMASKS) >> 4)
#define ADC_TO_NIL(x) ((x) = (x) & ~ADC_BITMASKS)
#define ADC_TO_INITED(x) BIT_SET((x), 4); BIT_CLR((x), 5)
#define ADC_TO_STARTED(x) BIT_SET((x), 4); BIT_SET((x), 5)

#define CONVERT_RESULT_BITMASK  (1 << 3)
#define CONVERT_RESULT_VALID(x) ((x) & CONVERT_RESULT_BITMASK)
#define SET_CONVERT_VALID(x)  BIT_SET((x), CONVERT_RESULT_BITMASK)
#define CLR_CONVERT_VALID(x)  BIT_CLR((x), CONVERT_RESULT_BITMASK)

#define ADC_RESULT_BITMASK  (1 << 2)
#define ADC_RESULT_VALID(x) ((x) & ADC_RESULT_BITMASK)
#define SET_ADC_VALID(x)  BIT_SET((x), ADC_RESULT_BITMASK)
#define CLR_ADC_VALID(x)  BIT_CLR((x), ADC_RESULT_BITMASK)

typedef struct {
  uint8_t states;
  axis_t  axis[ADC_CHANNEL_NUM];
  jstk_timer_t timer;

  jstk_init_fn init;
  jstk_start_fn start;
  jstk_stop_fn stop;
}joystick_t;

void jstk_init(void);
void jstk_convert(void);
#ifdef __cplusplus
}
#endif
#endif //JOYSTICK_H
