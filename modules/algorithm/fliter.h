#ifndef FLITER_H
#define FLITER_H

#include "stdint.h"
#include "main.h"
#include "cmsis_os.h"
#include "stm32f407xx.h"
#include "arm_math.h"
#include "fliter.h"


//=================================================================================================
// 滑动平均滤波器结构体

#define MOVING_AVG_SIZE 8 // 滑动平均滤波器窗口大小,建议为2的整数次幂,如8 16 32等

typedef struct {
    float buffer[MOVING_AVG_SIZE];
    uint8_t index;
    uint8_t count;
    float sum;
} MovingAverageFilter;

void moving_avg_filter_init(MovingAverageFilter *filter);
float moving_avg_filter_update(MovingAverageFilter *filter, float new_value);

//=================================================================================================
// 一阶低通滤波器结构体
typedef struct {
    float alpha; // 0 < alpha < 1
    float output;
    uint8_t initialized; // 0 = not initialized, 1 = initialized
} LowPassFilter;

void lowpass_filter_init(LowPassFilter *filter, float alpha);
void lowpass_filter_update(LowPassFilter *filter, float new_value);

#endif // FLITER_H