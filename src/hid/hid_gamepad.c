/*************************************************************************
    > File Name: hid_gamepad.c
    > Author: Kevin
    > Created Time: 2020-08-27
    > Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include <string.h>
#include "em_gpio.h"

#include "inc/hid/hid_gamepad.h"
#include "inc/hal/button.h"
#include "inc/utils.h"
#include "gatt_db.h"
#include "logging/logging.h"
#include "sl_bt_api.h"
/* Defines  *********************************************************** */

/* Global Variables *************************************************** */

/* Static Variables *************************************************** */
gamepad_t gamepad = { 0 };

/* Static Functions Declaractions ************************************* */

static void _gamepad_send_report(void)
{
  if (gamepad.bt->adv_conn.conn_handle != INVALID_HANDLE
      && gamepad.bt->adv_conn.conn.gatts.report_ccc != 0) {
    uint16_t sent_len;
    SE_CALL3(sl_bt_gatt_server_send_characteristic_notification(gamepad.bt->adv_conn.conn_handle,
                                                               gattdb_report,
                                                               REPORT_SIZE,
                                                               (uint8_t *)gamepad.report_buf,
                                                               &sent_len));
    LOGD("Report once.\n");
  }
}

static void _gamepad_on_btn_update(uint8_t which, bool press)
{
  if (press) {
    BIT_SET(gamepad.report_buf[which / 8], which % 8);
  } else {
    BIT_CLR(gamepad.report_buf[which / 8], which % 8);
  }
  _gamepad_send_report();
}

static void _gamepad_on_stick_update(uint8_t which, int8_t value)
{
  gamepad.report_buf[2 + which] = value;

  _gamepad_send_report();
}

void gamepad_init(void)
{
  memset(&gamepad, 0, sizeof(gamepad_t));

  gamepad.bt = get_bt_ptr();

  gamepad.btn_update = _gamepad_on_btn_update;
  gamepad.stick_update = _gamepad_on_stick_update;

  btn_init();

  if (GPIO_PinInGet(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN) == 0
      || GPIO_PinInGet(BSP_BUTTON1_PORT, BSP_BUTTON1_PIN) == 0) {
    LOGW("Factory Reset...\n");
    SE_CALL3(sl_bt_nvm_erase_all());
    sl_bt_system_reset(0);
  }
}
