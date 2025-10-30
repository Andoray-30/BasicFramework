#pragma once

#include <stdint.h>
#include "encoder_spi.h"
#include "EncoderReg.h"

#ifdef __cplusplus
extern "C" {
#endif

void EncoderApp_Init(void);

/* 通用多命令 16bit 批量收发（CS 全程保持低电平） */
uint8_t EncoderApp_TransceiveWords(const uint16_t *tx, uint16_t *rx, uint8_t words);

/* MA600 角度读取（基于 Table 5 帧格式） */
uint16_t MA600_ReadAngleRaw(void);        // 单帧读取：返回原始 16bit 角度
uint16_t MA600_ReadAngleRaw_2frame(void); // 双帧读取：命令+dummy，返回第二帧角度
float    MA600_ReadAngleDeg(void);        // 角度（单位：度）

/* MA600 寄存器与NVM高层API */
uint16_t MA600_ReadRegRaw(uint8_t reg);        // 读寄存器原始返回（16bit，通常低8位为寄存器值）
uint8_t  MA600_ReadReg(uint8_t reg);           // 读寄存器值（低8位）
uint8_t  MA600_WriteReg(uint8_t reg, uint8_t value); // 写寄存器（易失性寄存器）；返回1成功

uint8_t  MA600_WaitNVMReady(uint32_t timeout_ms);    // 轮询等待NVMB=0，返回1表示空闲
uint8_t  MA600_NVM_StoreBlock(uint8_t blockIndex);   // NVM存储块0或块1（会等待完成），返回1成功
uint8_t  MA600_NVM_RestoreAll(void);                 // NVM恢复全部寄存器，返回1成功
uint8_t  MA600_ClearError(void);                     // 清除错误标志

void EncoderApp_TestTick10ms(void);

#ifdef __cplusplus
}
#endif
