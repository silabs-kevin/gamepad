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
#ifdef __cplusplus
}
#endif
#endif //UTILS_H
