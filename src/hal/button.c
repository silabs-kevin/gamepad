/*************************************************************************
    > File Name: button.c
    > Author: Kevin
    > Created Time: 2020-08-27
    > Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include "inc/hal/button.h"
#include "em_gpio.h"
#include "gpiointerrupt.h"
#include "logging/logging.h"
#include "inc/hid/hid_gamepad.h"
#include "inc/utils.h"
#include "inc/bt.h"
#include "sl_bt_api.h"
/* Defines  *********************************************************** */

/* Global Variables *************************************************** */
extern gamepad_t gamepad;

/* Static Variables *************************************************** */
#define BTN_EVT_CACHE_SIZE  8
typedef struct {
  uint8_t cache[BTN_EVT_CACHE_SIZE];
  uint8_t num;
}btn_evt_t;

static btn_evt_t btn_evt = { 0 };

/* Static Functions Declaractions ************************************* */

static void btn_push_evt(uint8_t evt)
{
  ASSERT(btn_evt.num != BTN_EVT_CACHE_SIZE);
  if (btn_evt.num && btn_evt.cache[btn_evt.num - 1] == evt) {
    /* not to push the same event for multiple times */
    return;
  }
  btn_evt.cache[btn_evt.num++] = evt;
}

static void button_init(void)
{
  // configure pushbutton PB0 and PB1 as inputs, with pull-up enabled
  GPIO_PinModeSet(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN, gpioModeInputPullFilter, 1);
  GPIO_PinModeSet(BSP_BUTTON1_PORT, BSP_BUTTON1_PIN, gpioModeInputPullFilter, 1);
}

/***************************************************************************//**
 * This is a callback function that is invoked each time a GPIO interrupt
 * in one of the pushbutton inputs occurs. Pin number is passed as parameter.
 *
 * @param[in] pin  Pin number where interrupt occurs
 *
 * @note This function is called from ISR context and therefore it is
 *       not possible to call any BGAPI functions directly. The button state
 *       change is signaled to the application using gecko_external_signal()
 *       that will generate an event gecko_evt_system_external_signal_id
 *       which is then handled in the main loop.
 ******************************************************************************/
static void button_interrupt(uint8_t pin)
{
  switch (pin) {
    case BSP_BUTTON0_PIN:
      if (GPIO_PinInGet(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN) == 0) {
        /* btn_push_evt(PB0_PRESS_EM); */
        return;
      } else {
        btn_push_evt(PB0_RELEASE_EM);
      }
      break;
    case BSP_BUTTON1_PIN:
      if (GPIO_PinInGet(BSP_BUTTON1_PORT, BSP_BUTTON1_PIN) == 0) {
        /* btn_push_evt(PB1_PRESS_EM); */
        return;
      } else {
        btn_push_evt(PB1_RELEASE_EM);
      }
      break;
    default:
      return;
  }
  sl_bt_external_signal(PB_UPDATE_EVT);
}

/*******************************************************************************
 * Enable button interrupts for PB0, PB1. Both GPIOs are configured to trigger
 * an interrupt on the rising edge (button released).
 ******************************************************************************/
static void enable_button_interrupts(void)
{
  GPIOINT_Init();

  /* configure interrupt for PB0 and PB1, both falling and rising edges */
  GPIO_ExtIntConfig(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN, BSP_BUTTON0_PIN,
                    true, true, true);
  GPIO_ExtIntConfig(BSP_BUTTON1_PORT, BSP_BUTTON1_PIN, BSP_BUTTON1_PIN,
                    true, true, true);

  /* register the callback function that is invoked when interrupt occurs */
  GPIOINT_CallbackRegister(BSP_BUTTON0_PIN, button_interrupt);
  GPIOINT_CallbackRegister(BSP_BUTTON1_PIN, button_interrupt);
}

void btn_init(void)
{
  button_init();
  enable_button_interrupts();
}

void btn_update_handler(void)
{
  LOGD("btn handler\n");
  if (get_bt_ptr()->adv_conn.conn_handle == INVALID_HANDLE) {
    LOGD("Try starting adv\n");
    start_adv();
  } else {
#if 0
    for (uint8_t i = 0; i < btn_evt.num; i++) {
      if (gamepad.btn_update) {
        gamepad.btn_update((btn_evt.cache[i] - 1) / 2, btn_evt.cache[i] % 2);
      }
    }
#else
#if 0
    static uint8_t key = 1;
    LOGD("simulated key = %u\n", key);
    if (gamepad.btn_update) {
      gamepad.btn_update((key - 1) / 2, key % 2);
    }

    if (key++ == 32) {
      key = 1;
    }
    if (btn_evt.cache[btn_evt.num - 1] == PB1_RELEASE_EM && key > 2) {
      key -= 2;
    }
#endif
    static uint8_t stick = 0;
    static int value = 50;

    if (btn_evt.cache[btn_evt.num - 1] == PB1_RELEASE_EM) {
      WRAP_INCREMENT(stick, 4);
      value = 50;
    } else {
      LOGD("Report stick [%d] value: %d\n", stick, value);
      gamepad.stick_update(stick, value);
      value = (value == 50) ? 0 : value == 0 ? -50 : 50;
    }

    /* WRAP_INCREMENT(stick, 4); */
#endif
    memset(&btn_evt, 0, sizeof(btn_evt_t));
  }
}
