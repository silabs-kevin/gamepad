/*************************************************************************
    > File Name: button.h
    > Author: Kevin
    > Created Time: 2020-08-27
    > Description:
 ************************************************************************/

#ifndef BUTTON_H
#define BUTTON_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "em_gpio.h"

#define BTN_NUM 16

#define BSP_BUTTON0_PIN                               (6U)
#define BSP_BUTTON0_PORT                              (gpioPortF)

#define BSP_BUTTON1_PIN                               (7U)
#define BSP_BUTTON1_PORT                              (gpioPortF)

#define PB_UPDATE_EVT (1 << 0UL)

typedef struct {
  GPIO_Port_TypeDef port;
  unsigned int pin;
  uint8_t press_evt_em; /* corresponding release event id = this event id +1 */
}btn_t;

enum {
  PB0_PRESS_EM = 1,
  PB0_RELEASE_EM,
  PB1_PRESS_EM,
  PB1_RELEASE_EM,
};

void btn_init(void);
void btn_update_handler(void);
#ifdef __cplusplus
}
#endif
#endif //BUTTON_H
