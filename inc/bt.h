/*************************************************************************
    > File Name: bt.h
    > Author: Kevin
    > Created Time: 2020-08-26
    > Description:
 ************************************************************************/

#ifndef BT_H
#define BT_H
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>
#include "sl_bluetooth.h"

#define SE_CALL(x)                                                 \
  do {                                                             \
    uint16_t se_call_exclusive_returned_value = (x)->result;       \
    ASSERT_MSG(se_call_exclusive_returned_value == bg_err_success, \
               "BGAPI Call Failed, Error Code: 0x%04x\n",          \
               se_call_exclusive_returned_value);                  \
  }while(0)

#define SE_CALL3(x)                                              \
  do {                                                           \
    sl_status_t se_call_exclusive_returned_value = (x);          \
    ASSERT_MSG(se_call_exclusive_returned_value == SL_STATUS_OK, \
               "BGAPI Call Failed, Error Code: 0x%04x\n",        \
               se_call_exclusive_returned_value);                \
  }while(0)

#define INVALID_HANDLE  0xff

#define ADV_INT_MIN_DFT (160)
#define ADV_INT_MAX_DFT (160)

#define CONN_INTERVAL_MS(x) ((x) * 1.25F)
#define SUPERVISION_TIMEOUT_MS(x) ((x) * 10)

#define BONDING_INFO_PSKEY  0x4000

#ifndef TEST_LOGGING
#define TEST_LOGGING  1
#endif

/* All the soft timer ID */
enum {
#if defined(TEST_LOGGING) && (TEST_LOGGING == 1)
  PERIODICAL_LOGGING_TMID = 133,
#endif
};

typedef struct {
  bool enabled;
  uint8_t handle;
  uint16_t interval_min;
  uint16_t interval_max;
  uint16_t duration;
  uint8_t disc_mode;
}adv_t;

typedef struct {
  uint16_t interval;
  uint16_t latency;
  uint16_t timeout;
  uint8_t secure_level;
  uint16_t tx_size; /* Maximum Data Channel PDU Payload size that the controller can send in an air packet */
}conn_param_t;

typedef struct {
  bool bonded;
  uint8_t bonding_handle;
  uint16_t failed_reason;
}sm_t;

typedef struct {
  uint8_t report_ccc;
}gatts_t;

typedef struct {
  conn_param_t conn_param;
  sm_t sm;
  gatts_t gatts;
}conn_t;

typedef struct {
  uint8_t conn_handle;
  union {
    adv_t adv;
    conn_t conn;
  };
}adv_conn_t;

typedef struct {
  struct {
    uint8_t flags;
    uint8_t io_cap;
  }sm;
}bt_conf_t;

typedef struct {
  uint8_t bonding_info; /* any bonded devices in the bonding list */
  adv_conn_t adv_conn;
  bt_conf_t conf;
}bt_t;

void bt_system_evt_handler(const sl_bt_msg_t* e);
void bt_conn_evt_handler(const sl_bt_msg_t* e);
void bt_sm_evt_handler(const sl_bt_msg_t* e);
void bt_gatt_server_evt_handler(const sl_bt_msg_t* e);
void bt_hardware_evt_handler(const sl_bt_msg_t* e);
void bt_advertiser_evt_handler(const sl_bt_msg_t* e);

bt_t *get_bt_ptr(void);
void start_adv(void);
#ifdef __cplusplus
}
#endif
#endif //BT_H
