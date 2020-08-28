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
#define BSP_BUTTON0_PIN                               (6U)
#define BSP_BUTTON0_PORT                              (gpioPortF)

#define BSP_BUTTON1_PIN                               (7U)
#define BSP_BUTTON1_PORT                              (gpioPortF)

#define PB_UPDATE_EVT (1 << 0UL)

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
