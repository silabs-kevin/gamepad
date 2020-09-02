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

#define JSTK_NUM  2
#define ADC_CHANNEL_NUM (JSTK_NUM * 2)
// Init to max ADC clock for Series 1 with AUXHFRCO
#define ADC_FREQ        4000000
// Desired letimer interrupt frequency (in Hz)
#define LETIMERDESIRED  1000

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
  uint8_t state;
  uint16_t freq;
}jstk_timer_t;

typedef struct {
  uint8_t ldma_channel;
}jstk_peri_config_t;

typedef struct {
  uint8_t adc_state;
  axis_t  axis[ADC_CHANNEL_NUM];
  jstk_timer_t timer;

  jstk_init_fn init;
  jstk_start_fn start;
  jstk_stop_fn stop;
}joystick_t;

void jstk_init(void);
#ifdef __cplusplus
}
#endif
#endif //JOYSTICK_H
