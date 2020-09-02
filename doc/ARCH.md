## Requirements

### TBD

- 是否要传音频

  - [] Yes
  - [] No

- Report Rate 要求，关系到延时

### Peripherals

1. 4 ADC - 左摇杆 x,y 和右摇杆 x,y。
2. 12 GPIO - 上下左右，L1, L2, R1, R2, 右边四个图形按键

## HID Report

|                  | Bit 0    | Bit 1    | Bit 2        | Bit 3       | Bit 4      | Bit 5        | Bit 6       | Bit 7    |
| ---------------- | -------- | -------- | ------------ | ----------- | ---------- | ------------ | ----------- | -------- |
| Byte 0 (Buttons) | A (96)   | B (97)   | C (98)       | X (99)      | Y (100)    | Z (101)      | L1 (102)    | R1 (103) |
| Byte 1 (Buttons) | L2 (104) | R2 (105) | Select (109) | Start (108) | Mode (110) | THUMBL (106) | THUMBR(107) | 0        |
| Byte 2 (X)       |          |          |              |             |            |              |             |          |
| Byte 3 (Y)       |          |          |              |             |            |              |             |          |
| Byte 4 (Z)       |          |          |              |             |            |              |             |          |
| Byte 5 (Rz)      |          |          |              |             |            |              |             |          |  |

Axis X and Y are for the left joystick. Axis Z and Rz are for the left joystick.
Check Android's header file [input.h](https://android.googlesource.com/platform/frameworks/native/+/master/include/android/input.h)
for details.

## NOTES

- Connecting with iOS device, if iOS updates the connection interval to be 12
  actively, which means the device is regconized as a HID device.

- If the firmware reversion doesn't specified in the isc file, it seems the iOS
  won't regconize the device as a HID device.

- Connection parameters vs platforms

|                     | iOS (iPad Air 2) 13.5.1 | MacOS | Android |
| ------------------- | ----------------------- | ----- | ------- |
| Connection interval | 12                      | 35    | 30      |
| Latency             | 4                       | 0     | 0       |
| Timeout             | 100                     | 200   | 500     |
| MTU                 | 185                     | 247   | 247     |
| Tx Size             | 27                      | 251   | 251     |
| Secure Level        | 1                       | 1     | 1       |
