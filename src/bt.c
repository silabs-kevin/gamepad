/*************************************************************************
    > File Name: /Users/zhfu/work/projs/joystick/src/bt.c
    > Author: Kevin
    > Created Time: 2020-08-26
    > Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include "gatt_db.h"

#include "inc/bt.h"
#include "inc/utils.h"
#include "logging/logging.h"
#include "inc/hal/button.h"
#include "inc/hal/joystick.h"
#include "inc/hid/hid_gamepad.h"
#include "sl_bt_api.h"
#include "sl_bt_types.h"
/* Defines  *********************************************************** */

#ifndef DUMP_LTK
#define DUMP_LTK  1
#endif

/* #define DISABLE_SLEEP */

/* Global Variables *************************************************** */

/* Static Variables *************************************************** */
static bt_t bt = { 0 };

/* Static Functions Declaractions ************************************* */
static void test(void);

#ifdef DISABLE_SLEEP
bool app_is_ok_to_sleep(void)
{
  return false;
}
#endif
#if defined(TEST_LOGGING) && (TEST_LOGGING == 1)
#define LOGGING_TEST_INTERVAL (32768 * 2)
static uint8_t lvl = 0;
static void _start_logging_test(void)
{
  logging_demo(0xff);

  SE_CALL3(sl_bt_system_set_soft_timer(LOGGING_TEST_INTERVAL,
                                       PERIODICAL_LOGGING_TMID,
                                       0));
}
#endif
#if defined(DUMP_LTK) && (DUMP_LTK == 1)
static void dump_ltk(void)
{
  sl_status_t sls;
  size_t len;
  uint8_t buf[16];

  /* 0x8003 as the first key, 0x8103 as the second one if applicable */
  sls = sl_bt_nvm_load(0x8003,
                       16,
                       &len,
                       buf);

  if (sls == SL_STATUS_OK) {
    LOGH("LTK:\n");
    HEX_DUMP_16(buf, len);
  } else {
    LOGW("Dump LTK failed - error code: 0x%04x\n",
         sls);
  }
}
#endif

static inline void __on_sm_passkey_confirm(const sl_bt_msg_t *evt)
{
  const sl_bt_evt_sm_confirm_passkey_t *e;
  ASSERT(evt && bt.adv_conn.conn_handle != INVALID_HANDLE);
  e = &evt->data.evt_sm_confirm_passkey;

  LOGI("Passkey - %06d\n", e->passkey);

  /* TODO: dirty confirm anyway */
  SE_CALL3(sl_bt_sm_passkey_confirm(e->connection, 1));
}

static void delete_bonding(void)
{
  SE_CALL3(sl_bt_sm_delete_bondings());

  bt.bonding_info = 0;
  SE_CALL3(sl_bt_nvm_save(BONDING_INFO_PSKEY,
                          1,
                          (uint8_t *)&bt.bonding_info));
}

static inline void __on_sm_bonding_confirm(const sl_bt_msg_t *evt)
{
  const sl_bt_evt_sm_confirm_bonding_t *e;
  ASSERT(evt && bt.adv_conn.conn_handle != INVALID_HANDLE);
  e = &evt->data.evt_sm_confirm_bonding;
  SE_CALL3(sl_bt_sm_bonding_confirm(e->connection, 1));
}

static inline void __on_sm_bond_failed(const sl_bt_msg_t *evt)
{
  const sl_bt_evt_sm_bonding_failed_t *e;
  ASSERT(evt && bt.adv_conn.conn_handle != INVALID_HANDLE);
  e = &evt->data.evt_sm_bonding_failed;
  LOGE("Bonding FAILED, connection handle: %u, reason: 0x%04x\n",
       e->connection,
       e->reason);
  if (e->reason == SL_STATUS_BT_CTRL_PIN_OR_KEY_MISSING) {
    delete_bonding();
  }
  bt.adv_conn.conn.sm.bonded = 0;
  bt.adv_conn.conn.sm.failed_reason = e->reason;
}

static inline void __on_sm_bonded(const sl_bt_msg_t *evt)
{
  const sl_bt_evt_sm_bonded_t *e;
  ASSERT(evt && bt.adv_conn.conn_handle != INVALID_HANDLE);
  e = &evt->data.evt_sm_bonded;
  LOGH("Bonded, connection handle: %u, bonding handle: %u\n",
       e->connection,
       e->bonding);

  bt.adv_conn.conn.sm.bonded = e->bonding == INVALID_HANDLE ? 0 : 1;
  bt.adv_conn.conn.sm.bonding_handle = e->bonding;

  if (bt.adv_conn.conn.sm.bonded) {
    bt.bonding_info = 1;
    SE_CALL3(sl_bt_nvm_save(BONDING_INFO_PSKEY,
                            1,
                            (uint8_t *)&bt.bonding_info));
  }
#if defined(DUMP_LTK) && (DUMP_LTK == 1)
  dump_ltk();
#endif
}

static inline void __on_conn_param_updated(const sl_bt_msg_t *evt)
{
  const sl_bt_evt_connection_parameters_t *e;
  ASSERT(evt);
  e = &evt->data.evt_connection_parameters;
  LOGD("Connection Parameter Updated\n"
       "                           - Interval: %u\n"
       "                           - Latency: %u\n"
       "                           - Timeout: %u\n"
       "                           - Secure Level: %u\n"
       "                           - Tx Size: %u\n",
       e->interval,
       e->latency,
       e->timeout,
       e->security_mode,
       e->txsize);

  bt.adv_conn.conn.conn_param.interval = e->interval;
  bt.adv_conn.conn.conn_param.latency = e->latency;
  bt.adv_conn.conn.conn_param.timeout = e->timeout;
  bt.adv_conn.conn.conn_param.secure_level = e->security_mode;
  bt.adv_conn.conn.conn_param.tx_size = e->txsize;
}

static inline void __on_connection_opened(const sl_bt_msg_t *evt)
{
  const sl_bt_evt_connection_opened_t *e;
  ASSERT(evt);
  e = &evt->data.evt_connection_opened;
  LOGI("Connection Opened\n");

  memset(&bt.adv_conn, 0, sizeof(adv_conn_t));

  bt.adv_conn.conn_handle = e->connection;
  if (e->bonding != 0xff) {
    bt.adv_conn.conn.sm.bonded = true;
    bt.adv_conn.conn.sm.bonding_handle = e->bonding;
  }
}

void start_adv(void)
{
  ASSERT(bt.adv_conn.conn_handle == INVALID_HANDLE);
  if (bt.adv_conn.adv.enabled) {
    return;
  }

  /* Configure the adv timing according to HOGP */
  if (bt.bonding_info) {
    /* Already have bondings - follow table 5.2*/
    bt.adv_conn.adv.duration = 30 * 100; /* 30 seconds in unit 10ms */
    bt.adv_conn.adv.disc_mode = advertiser_limited_discoverable;
    bt.adv_conn.adv.interval_min = 32; /* 20ms in unit 0.625ms */
    bt.adv_conn.adv.interval_max = 48; /* 30ms in unit 0.625ms */
  } else {
    /* No bondings, initial connection - follow table 5.1 */
    bt.adv_conn.adv.duration = 180 * 100; /* 180 seconds in unit 10ms */
    bt.adv_conn.adv.disc_mode = advertiser_limited_discoverable;
    bt.adv_conn.adv.interval_min = 48; /* 30ms in unit 0.625ms */
    bt.adv_conn.adv.interval_max = 80; /* 50ms in unit 0.625ms */
  }

  SE_CALL3(sl_bt_advertiser_set_timing(bt.adv_conn.adv.handle,
                                       bt.adv_conn.adv.interval_min,
                                       bt.adv_conn.adv.interval_max,
                                       bt.adv_conn.adv.duration,
                                       0));
  SE_CALL3(sl_bt_advertiser_start(bt.adv_conn.adv.handle,
                                  bt.adv_conn.adv.disc_mode,
                                  advertiser_connectable_scannable));
  bt.adv_conn.adv.enabled = true;
  LOGH("%s Adv Started\n", bt.bonding_info ? "FAST" : "Normal");
}

static inline void __on_connection_closed(const sl_bt_msg_t *evt)
{
  const sl_bt_evt_connection_closed_t *e;
  ASSERT(evt);
  e = &evt->data.evt_connection_closed;
  LOGW("Connection Closed, reason: 0x%04x\n", e->reason);

  memset(&bt.adv_conn, 0, sizeof(adv_conn_t));
  bt.adv_conn.conn_handle = INVALID_HANDLE;
  /* TODO: OTA handler */
  start_adv();
}

static void bonding_info_load(void)
{
  sl_status_t sls;
  size_t len;
  uint8_t buf[1];

  sls = sl_bt_nvm_load(BONDING_INFO_PSKEY,
                       1,
                       &len,
                       buf);

  if (sls == SL_STATUS_BT_PS_KEY_NOT_FOUND) {
    uint8_t empty = 0;
    SE_CALL3(sl_bt_sm_list_all_bondings());
    while (1) {
      sl_bt_msg_t e;
      sl_status_t status = sl_bt_pop_event(&e);
      if (status != SL_STATUS_OK) {
        continue;
      }
      uint32_t evt_id = SL_BT_MSG_ID(e.header);
      if (evt_id == sl_bt_evt_sm_list_bonding_entry_id) {
        if (!empty) {
          empty = 1;
        }
      } else if (evt_id == sl_bt_evt_sm_list_all_bondings_complete_id) {
        SE_CALL3(sl_bt_nvm_save(BONDING_INFO_PSKEY,
                                1,
                                &empty));
        LOGI("Bonding list %s empty\n", empty ? "" : "NOT");
        break;
      }
    }
  }

  sls = sl_bt_nvm_load(BONDING_INFO_PSKEY,
                       1,
                       &len,
                       buf);
  if (sls == SL_STATUS_BT_PS_KEY_NOT_FOUND) {
  } else if (sls == SL_STATUS_OK) {
    bt.bonding_info = buf[0];
    LOGI("Bonding list %s empty\n",
         bt.bonding_info ? "NOT" : "");
#if defined(DUMP_LTK) && (DUMP_LTK == 1)
    if (bt.bonding_info) {
      dump_ltk();
    }
#endif
  } else {
    ASSERT(0);
  }
}

static inline void __on_system_boot(const sl_bt_msg_t* evt)
{
  const sl_bt_evt_system_boot_t *e;
  bd_addr address;
  uint8_t address_type;
  uint8_t system_id[8];

  ASSERT(evt);
  e = &evt->data.evt_system_boot;

  LOGI("System Booted, stack version: %u.%u.%u\n", e->major, e->minor, e->patch);

  SE_CALL3(sl_bt_system_get_identity_address(&address, &address_type));

  // Pad and reverse unique ID to get System ID.
  system_id[0] = address.addr[5];
  system_id[1] = address.addr[4];
  system_id[2] = address.addr[3];
  system_id[3] = 0xFF;
  system_id[4] = 0xFE;
  system_id[5] = address.addr[2];
  system_id[6] = address.addr[1];
  system_id[7] = address.addr[0];

  SE_CALL3(sl_bt_gatt_server_write_attribute_value(gattdb_system_id,
                                                   0,
                                                   sizeof(system_id),
                                                   system_id));

  bonding_info_load();

  bt.adv_conn.conn_handle = INVALID_HANDLE;

  /* Create the advertising set */
  SE_CALL3(sl_bt_advertiser_create_set(&bt.adv_conn.adv.handle));

  /* Configure Security Manager */
  // BIT_SET(bt.conf.sm.flags, 0); /* require MITM */
  // BIT_SET(bt.conf.sm.flags, 1); /* Encryption requires bonding */
  BIT_SET(bt.conf.sm.flags, 3); /* bonding needs confirmation */

  /* bt.conf.sm.io_cap = sm_io_capability_keyboardonly; */
  bt.conf.sm.io_cap = sm_io_capability_noinputnooutput;

  SE_CALL3(sl_bt_sm_configure(bt.conf.sm.flags,
                              bt.conf.sm.io_cap));
  SE_CALL3(sl_bt_sm_set_bondable_mode(1));
  SE_CALL3(sl_bt_sm_store_bonding_configuration(1, 2));

  gamepad_init();

  start_adv();
}

static inline void __on_external_signals(const sl_bt_msg_t* evt)
{
  const sl_bt_evt_system_external_signal_t *e;
  ASSERT(evt);
  e = &evt->data.evt_system_external_signal;
  /* e->extsignals is bitmap */
  if (e->extsignals & PB_UPDATE_EVT) {
    btn_update_handler();
  }

  if (e->extsignals & JOYSTICK_UPDATE_EVT) {
    jstk_convert();
  }
}

static inline void __on_gatt_server_att(const sl_bt_msg_t* evt)
{
  const sl_bt_evt_gatt_server_attribute_value_t*e;
  ASSERT(evt);
  e = &evt->data.evt_gatt_server_attribute_value;

  LOGD("GATTS Attribute value changed.\n");
  LOGD("Handle: %d, att opcode: %d, offset: %d, value len: %u\n",
       e->attribute,
       e->att_opcode,
       e->offset,
       e->value.len);
  HEX_DUMP_16(e->value.data, e->value.len);
}

static inline void __on_gatt_server_char_status(const sl_bt_msg_t* evt)
{
  const sl_bt_evt_gatt_server_characteristic_status_t *e;
  ASSERT(evt);
  e = &evt->data.evt_gatt_server_characteristic_status;
  LOGD("Char: 0x%04x, CCC: %u\n", e->characteristic, e->status_flags);
  if (e->characteristic == gattdb_report
      && e->status_flags == gatt_server_client_config) {
    bt.adv_conn.conn.gatts.report_ccc = e->client_config_flags;

#if TEST
    sl_bt_cmd_hardware_set_soft_timer(32768, TEST_TMID, 0);
#endif
  }
}

static inline void __on_softtimer(const sl_bt_msg_t* evt)
{
  const  sl_bt_evt_system_soft_timer_t *e;
  ASSERT(evt);
  e = &evt->data.evt_system_soft_timer;

  switch (e->handle) {
#if defined(TEST_LOGGING) && (TEST_LOGGING == 1)
    case PERIODICAL_LOGGING_TMID:
      logging_demo(lvl);
      lvl = (lvl + 1) % (LOGGING_VERBOSE + 1);
      break;
#endif
    case TEST_TMID:
      test();
      break;
  }
}

static inline void __on_adv_timeout(const sl_bt_msg_t* evt)
{
  ASSERT(evt);
  bt.adv_conn.adv.enabled = 0;
  LOGI("Adv Stopped due to Timeout\n");
}

void bt_conn_evt_handler(const sl_bt_msg_t* e)
{
  switch (SL_BT_MSG_ID(e->header)) {
    case sl_bt_evt_connection_opened_id:
      __on_connection_opened(e);
      break;
    case sl_bt_evt_connection_closed_id:
      __on_connection_closed(e);
      break;
    case sl_bt_evt_connection_parameters_id:
      __on_conn_param_updated(e);
      break;
  }
}

void bt_sm_evt_handler(const sl_bt_msg_t* e)
{
  switch (SL_BT_MSG_ID(e->header)) {
    case sl_bt_evt_sm_bonded_id:
      __on_sm_bonded(e);
      break;
    case sl_bt_evt_sm_bonding_failed_id:
      __on_sm_bond_failed(e);
      break;
    case sl_bt_evt_sm_confirm_bonding_id:
      __on_sm_bonding_confirm(e);
      break;
    case sl_bt_evt_sm_confirm_passkey_id:
      __on_sm_passkey_confirm(e);
      break;
  }
}

void bt_gatt_server_evt_handler(const sl_bt_msg_t* e)
{
  switch (SL_BT_MSG_ID(e->header)) {
    case sl_bt_evt_gatt_server_characteristic_status_id:
      __on_gatt_server_char_status(e);
      break;
    case sl_bt_evt_gatt_server_attribute_value_id:
      __on_gatt_server_att(e);
      break;
    case sl_bt_evt_gatt_server_user_write_request_id:
      ASSERT_MSG(0, "Need to handle\n");
      break;
  }
}

void bt_system_evt_handler(const sl_bt_msg_t* e)
{
  switch (SL_BT_MSG_ID(e->header)) {
    case sl_bt_evt_system_boot_id:
#if defined(TEST_LOGGING) && (TEST_LOGGING == 1)
      _start_logging_test();
#else
      __on_system_boot(e);
#endif
      break;
    case sl_bt_evt_system_external_signal_id:
      __on_external_signals(e);
      break;
    case sl_bt_evt_system_soft_timer_id:
      __on_softtimer(e);
      break;
  }
}

void bt_hardware_evt_handler(const sl_bt_msg_t* e)
{
  switch (SL_BT_MSG_ID(e->header)) {
  }
}

void bt_advertiser_evt_handler(const sl_bt_msg_t* e)
{
  switch (SL_BT_MSG_ID(e->header)) {
    case sl_bt_evt_advertiser_timeout_id:
      __on_adv_timeout(e);
      break;
  }
}

static void test(void)
{
  static int key = 0;
  uint16_t sent_len;

  char buf[6] = { 0 };

  BIT_SET(buf[key / 8], key % 8);
  WRAP_INCREMENT(key, 24);

  buf[2] = key;
  buf[3] = key;
  buf[4] = key;
  buf[5] = key;

  SE_CALL3(sl_bt_gatt_server_send_characteristic_notification(bt.adv_conn.conn_handle,
                                                              gattdb_report,
                                                              6,
                                                              (uint8_t *)buf,
                                                              &sent_len));
  LOGD("notify: %u\n", key);
}

bt_t *get_bt_ptr(void)
{
  return &bt;
}
