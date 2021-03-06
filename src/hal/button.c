/*************************************************************************
    > File Name: button.c
    > Author: Kevin
    > Created Time: 2020-08-27
    > Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include "inc/hal/button.h"
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
static const btn_t up_btn = {
  .port = gpioPortF,
  .pin = 6,
  .ext_int_num = 6,
  .press_evt_em = PB0_PRESS_EM
};

static const btn_t down_btn = {
  .port = gpioPortF,
  .pin = 7,
  .ext_int_num = 7,
  .press_evt_em = PB1_PRESS_EM
};

/* This should align with the sequence 16-bit in the report*/
static const btn_t *const btns[] = {
  &up_btn,
  &down_btn,
};

static const size_t btn_num = sizeof(btns) / sizeof(btn_t *);

static void btn_push_evt(uint8_t evt)
{
  ASSERT(btn_evt.num != BTN_EVT_CACHE_SIZE);
  /*
   * Event valid only when iterating the queue backwards, the last event is the oppsite event or no
   *  related event is found.
   */
  if (btn_evt.num) {
    for (uint8_t i = btn_evt.num - 1; i != 0xff; i--) {
      uint8_t exp_pre_evt = evt % 2 ? evt - 1 : evt + 1;
      if (btn_evt.cache[i] == exp_pre_evt) {
        break;
      } else if (btn_evt.cache[i] == evt) {
        /* Event not valid since the last same event hasn't been sent */
        return;
      }
    }
  }

  btn_evt.cache[btn_evt.num++] = evt;
}

static void button_init(void)
{
  for (uint8_t i = 0; i < btn_num; i++) {
    GPIO_PinModeSet(btns[i]->port,
                    btns[i]->pin,
                    gpioModeInputPullFilter,
                    1);
  }
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
  for (uint8_t i = 0; i < btn_num; i++) {
    if (btns[i]->ext_int_num == pin) {
      if (GPIO_PinInGet(btns[i]->port, btns[i]->pin)) {
        btn_push_evt(btns[i]->press_evt_em);
      } else {
        btn_push_evt(btns[i]->press_evt_em + 1);
      }
    }
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

  for (uint8_t i = 0; i < btn_num; i++) {
    /* configure interrupt for PB0 and PB1, both falling and rising edges */
    GPIO_ExtIntConfig(btns[i]->port,
                      btns[i]->pin,
                      btns[i]->ext_int_num,
                      true,
                      true,
                      true);
    GPIOINT_CallbackRegister(btns[i]->ext_int_num,
                             button_interrupt);
  }
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
      if (stick == 0) {
        gamepad.stick_update(value, 0, 0, 0);
      } else if (stick == 1) {
        gamepad.stick_update(0, value, 0, 0);
      } else if (stick == 2) {
        gamepad.stick_update(0, 0, value, 0);
      } else if (stick == 3) {
        gamepad.stick_update(0, 0, 0, value);
      } else {
        return;
      }
      value = (value == 50) ? 0 : value == 0 ? -50 : 50;
    }

    /* WRAP_INCREMENT(stick, 4); */
#endif
    memset(&btn_evt, 0, sizeof(btn_evt_t));
  }
}
