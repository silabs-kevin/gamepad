/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "em_common.h"
#include "sl_app_assert.h"
#include "sl_bluetooth.h"
#include "gatt_db.h"
#include "app.h"

#include "logging/logging.h"
#include "inc/bt.h"

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
SL_WEAK void app_init(void)
{
  INIT_LOG(0xff);
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
SL_WEAK void app_process_action(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                              //
  // This is called infinitely.                                              //
  // Do not call blocking functions from here!                               //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  uint32_t evt_id = SL_BT_MSG_ID(evt->header);
  switch (evt_id) {
    case sl_bt_evt_system_boot_id:
    case sl_bt_evt_system_external_signal_id:
    case sl_bt_evt_system_soft_timer_id:
      bt_system_evt_handler(evt);
      break;

    case sl_bt_evt_connection_opened_id:
    case sl_bt_evt_connection_closed_id:
    case sl_bt_evt_connection_parameters_id:
      bt_conn_evt_handler(evt);
      break;

    case sl_bt_evt_sm_bonded_id:
    case sl_bt_evt_sm_bonding_failed_id:
    case sl_bt_evt_sm_confirm_bonding_id:
    case sl_bt_evt_sm_confirm_passkey_id:
      bt_sm_evt_handler(evt);
      break;

    case sl_bt_evt_gatt_server_characteristic_status_id:
    case sl_bt_evt_gatt_server_attribute_value_id:
    case sl_bt_evt_gatt_server_user_write_request_id:
      bt_gatt_server_evt_handler(evt);
      break;

    case sl_bt_evt_advertiser_timeout_id:
      bt_advertiser_evt_handler(evt);
      break;

    case sl_bt_evt_gatt_mtu_exchanged_id:
      LOGH("MTU:%u\n", evt->data.evt_gatt_mtu_exchanged.mtu);
      break;
    /* Add additional event handlers as your application requires */
    /*
     * for events not to be handled, list here.
     */
    case sl_bt_evt_connection_phy_status_id:
      break;

    default:
      LOGW("Unhandled event(0x%08x). Message class: 0x%02x, Message ID: 0x%02x\n",
           evt_id,
           (uint8_t)(evt_id >> 16),
           (uint8_t)(evt_id >> 24));
      break;
  }
}
