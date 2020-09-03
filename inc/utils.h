/*************************************************************************
    > File Name: utils.h
    > Author: Kevin
    > Created Time: 2020-08-27
    > Description:
 ************************************************************************/

#ifndef UTILS_H
#define UTILS_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
#include "logging/logging.h"

#ifndef MAX
#define MAX(a, b)                                   ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b)                                   ((a) < (b) ? (a) : (b))
#endif

#define BIT_SET(v, bit)                             ((v) |= (1 << (bit)))
#define BIT_CLR(v, bit)                             ((v) &= ~(1 << (bit)))
#define IS_BIT_SET(v, bit)                          (((v) & (1 << (bit))) ? 1 : 0)

#ifndef WRAP_INCREMENT
#define WRAP_INCREMENT(i, range)  (i) = ((i) + 1) % (range)
#endif

static inline uint16_t build_uint16(const uint8_t data[], char endian)
{
  ASSERT(endian == 'l' || endian == 'b');
  return endian == 'l'
         ? ((((uint16_t)data[1] & 0xff) << 8) | (data[0]))
         : ((((uint16_t)data[0] & 0xff) << 8) | (data[1]));
}

static inline uint32_t build_uint24(const uint8_t data[], char endian)
{
  ASSERT(endian == 'l' || endian == 'b');
  return endian == 'l'
         ?  (( (((uint32_t)data[2] & 0xff) << 16)
               | (((uint32_t)data[1] & 0xff) << 8)
               | ((uint32_t)data[0] & 0xff)) & 0x00ffffff)
         : (((((uint32_t)data[0] & 0xff) << 16)
             | (((uint32_t)data[1] & 0xff) << 8)
             | ((uint32_t)data[2] & 0xff)) & 0x00ffffff);
}

static inline uint32_t build_uint32(const uint8_t data[], char endian)
{
  ASSERT(endian == 'l' || endian == 'b');
  return endian == 'l'
         ?  ((((uint32_t)data[3] & 0xff) << 24)
             | (((uint32_t)data[2] & 0xff) << 16)
             | (((uint32_t)data[1] & 0xff) << 8)
             | ((uint32_t)data[0] & 0xff))
         : ((((uint32_t)data[0] & 0xff) << 24)
            | (((uint32_t)data[1] & 0xff) << 16)
            | (((uint32_t)data[2] & 0xff) << 8)
            | ((uint32_t)data[3] & 0xff));
}

static inline void disassemble_uint16(uint16_t val, uint8_t buf[], char endian)
{
  ASSERT(endian == 'l' || endian == 'b');
  if (endian == 'l') {
    buf[1] = ((val >> 8) & 0xff);
    buf[0] = (val & 0xff);
  } else {
    buf[0] = ((val >> 8) & 0xff);
    buf[1] = (val & 0xff);
  }
}

static inline void disassemble_uint24(uint32_t val, uint8_t buf[], char endian)
{
  ASSERT(endian == 'l' || endian == 'b');
  if (endian == 'l') {
    buf[2] = ((val >> 16) & 0xff);
    buf[1] = ((val >> 8) & 0xff);
    buf[0] = (val & 0xff);
  } else {
    buf[0] = ((val >> 16) & 0xff);
    buf[1] = ((val >> 8) & 0xff);
    buf[2] = (val & 0xff);
  }
}

static inline void disassemble_uint32(uint32_t val, uint8_t buf[], char endian)
{
  ASSERT(endian == 'l' || endian == 'b');
  if (endian == 'l') {
    buf[3] = ((val >> 24) & 0xff);
    buf[2] = ((val >> 16) & 0xff);
    buf[1] = ((val >> 8) & 0xff);
    buf[0] = (val & 0xff);
  } else {
    buf[0] = ((val >> 24) & 0xff);
    buf[1] = ((val >> 16) & 0xff);
    buf[2] = ((val >> 8) & 0xff);
    buf[3] = (val & 0xff);
  }
}
#ifdef __cplusplus
}
#endif
#endif //UTILS_H
