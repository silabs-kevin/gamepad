## Requirements

### TBD

- 是否要传音频

  - [] Yes
  - [] No

- Report Rate 要求，关系到延时

### Peripherals

1. 4 ADC - 左摇杆 x,y 和右摇杆 x,y。
2. 16 GPIO - 上下左右，L1, L2, R1, R2, 右边四个图形按键

#### Pin Map

| Pin  | Functionality         | WSTK        | Connected | EXP Header | WSTK Port |
| ---- | --------------------- | ----------- | --------- | ---------- | --------- |
| PA0  | USART0 Tx             | VCOM TX     | x         | 12         | 9         |
| PA1  | USART0 Rx             | VCOM RX     | x         | 14         | 11        |
| PA2  | USART0 CLK            | VCOM CTS    | x         | 3          | 0         |
| PA3  | USART0 CS#0           | VCOM RTS    | x         | 5          | 2         |
| PA4  | USART1 CS#1           |             |           |            | 14        |
| PA5  |                       | VCOM ENABLE | x         |            | 16        |
| PB11 | PTI.CLK               | PTI.CLK     | x         |            | 18        |
| PB12 | PTI.DATA              | PTI.DATA    | x         |            | 20        |
| PB13 | PTI.SYNC              | PTI.SYNC    | x         |            | 22        |
| PB14 | LFXTAL_P              |             | x         |            |           |
| PB15 | LFXTAL_N              |             | x         |            |           |
| PC6  | US1_TX#11             | x           | x         | 4          | 1         |
| PC7  | US1_RX#11             | x           | x         | 6          | 3         |
| PC8  | US1_CLK#11            | x           | x         | 8          | 5         |
| PC9  | US1_CS#11             |             |           | 10         | 7         |
| PC10 | I2C0_SCL#14           | x           | x         | 15         | 12        |
| PC11 | I2C0_SDA#16           | x           | x         | 16         | 13        |
| PD10 |                       |             |           | 7          | 4         |
| PD11 |                       |             |           | 9          | 6         |
| PD12 |                       |             |           | 11         | 8         |
| PD13 | LETIM0_OUT0#23        |             |           |            | 31        |
| PD14 | US1_CS#19             |             |           |            | 33        |
| PD15 |                       |             |           |            | 35        |
| PF0  | DBG_SWCLK#0           | x           | x         |            | 24        |
| PF1  | DBG_SWDIOTMS#0        | x           | x         |            | 26        |
| PF2  | DBG_SWO#0 / DBG_TDO#0 | x           | x         |            | 28        |
| PF3  | DBG_TDI#0             | x           | x         | 13         | 10        |
| PF4  | UIF_LED0              | x           | x         |            | 30        |
| PF5  | UIF_LED1              | x           | x         |            | 33        |
| PF6  | UIF_BUTTON0           | x           | x         |            | 34        |
| PF7  | UIF_BUTTON1           | x           | x         |            | 36        |

In total, 31 GPIOs

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
