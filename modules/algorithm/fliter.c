/**
 ******************************************************************************
 * @file	fliter.c
 * @author  
 * @version 0.1
 * @date    2025/10/10
 * @brief
 ******************************************************************************
 * @attention   下面是一些常用的滤波算法
 *  
 ******************************************************************************
 */
#include "stdlib.h"
#include "memory.h"
#include "fliter.h"
#include "math.h"
#include "main.h"

/**
 * @brief 滑动平均值滤波
 *
 * @param filter 滤波器结构体指针
 * @param new_value 新的输入值,即滤波对象

 */


void moving_avg_filter_init(MovingAverageFilter *filter) {
    memset(filter->buffer, 0, sizeof(filter->buffer));
    filter->index = 0;
    filter->count = 0;
    filter->sum = 0;
}

float moving_avg_filter_update(MovingAverageFilter *filter, float new_value) {
    if(filter->count < MOVING_AVG_SIZE) {
        // 缓冲区未满，直接累加
        filter->sum += new_value;
        filter->buffer[filter->count] = new_value;
        filter->count++;
        filter->index = filter->count;
    } else {
        // 缓冲区已满，滑动更新
        filter->sum -= filter->buffer[filter->index];  // 减去最旧值
        filter->sum += new_value;                      // 加上最新值
        filter->buffer[filter->index] = new_value;     // 更新缓冲区
        filter->index = (filter->index + 1) % MOVING_AVG_SIZE;
    }
    
    return filter->sum / filter->count;
}

/**
 * @brief 一阶低通滤波：适用于信号平滑，抑制高频噪声
 *
 * @param filter 滤波器结构体指针
 * @param new_value 新的输入值,即滤波对象
 * @param alpha: 0 < alpha < 1, alpha越大,滤波器响应越快,但噪声抑制效果越差
 */

void lowpass_filter_init(LowPassFilter *filter, float alpha) {
    filter->alpha = alpha;
    filter->output = 0.0f;
    filter->initialized = 0u;
}

void lowpass_filter_update(LowPassFilter *filter, float input) {
    if (!filter->initialized) {
        // 首次调用：直接把输出设为输入，避免起始衰减
        filter->output = input;
        filter->initialized = 1u;
        return;
    }
    filter->output = filter->alpha * input + (1.0f - filter->alpha) * filter->output;
}