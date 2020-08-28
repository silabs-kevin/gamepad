/*************************************************************************
    > File Name: hid_gamepad.h
    > Author: Kevin
    > Created Time: 2020-08-27
    > Description:
 ************************************************************************/

#ifndef HID_GAMEPAD_H
#define HID_GAMEPAD_H
#ifdef __cplusplus
extern "C"
{
#endif
#include "inc/bt.h"

#define REPORT_SIZE 6

typedef void (*btn_update_fn)(uint8_t which, bool press);
typedef void (*stick_update_fn)(uint8_t which, int8_t press);

typedef struct {
  int8_t report_buf[REPORT_SIZE];
  bt_t *bt;

  btn_update_fn btn_update;
  stick_update_fn stick_update;
}gamepad_t;

void gamepad_init(void);
#ifdef __cplusplus
}
#endif
#endif //HID_GAMEPAD_H
