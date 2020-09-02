/*************************************************************************
    > File Name: hid_desc.c
    > Author: Kevin
    > Created Time: 2020-08-26
    > Description:
 ************************************************************************/

/* Includes *********************************************************** */

/* Defines  *********************************************************** */

/* Global Variables *************************************************** */

/* Static Variables *************************************************** */

/* Static Functions Declaractions ************************************* */
/* 0x05010905a1010901a10005091901291015002501951075018102050109300931093209351581257f750895048102c0c0   */
const char joystick_report_desc[48] = {
  0x05, 0x01,                      // USAGE_PAGE (Generic Desktop)
  0x09, 0x05,                      // USAGE (Game Pad)
  0xa1, 0x01,                      // COLLECTION (Application)
  0x09, 0x01,                      //   Usage (Pointer)
  0xa1, 0x00,                      //   COLLECTION (Physical)
  0x05, 0x09,                      //     USAGE_PAGE (Button)
  0x19, 0x01,                      //     USAGE_MINIMUM (Button 1)
  0x29, 0x10,                      //     USAGE_MAXIMUM (Button 16)
  0x15, 0x00,                      //     LOGICAL_MINIMUM (0)
  0x25, 0x01,                      //     LOGICAL_MAXIMUM (1)
  0x95, 0x10,                      //     REPORT_COUNT (16)
  0x75, 0x01,                      //     REPORT_SIZE (1)
  0x81, 0x02,                      //     INPUT (Data,Var,Abs)
  0x05, 0x01,                      //     USAGE_PAGE (Generic Desktop)
  0x09, 0x30,                      //     USAGE (X)
  0x09, 0x31,                      //     USAGE (Y)
  0x09, 0x32,                      //     USAGE (Z)
  0x09, 0x35,                      //     USAGE (Rx)
  0x15, 0x81,                      //     LOGICAL_MINIMUM (-127)
  0x25, 0x7f,                      //     LOGICAL_MAXIMUM (127)
  0x75, 0x08,                      //     REPORT_SIZE (8)
  0x95, 0x04,                      //     REPORT_COUNT (4)
  0x81, 0x02,                      //     INPUT (Data,Var,Abs)
  0xc0,                            //     END_COLLECTION
  0xc0                             // END_COLLECTION
};

