/**
 * @file encoder_app.h
 * @brief 编码器应用层接口（MA600 高层访问、批量收发、角度读取）。
 *
 * 提供 MA600 角度/寄存器/NVM 的高层 API，以及对 BSP SPI 的批量 16-bit 收发封装。
 * 单位约定：角度以“度”为主（0~360），原始角以 16-bit 值（0..65535）表征。
 */
#pragma once

#include <stdint.h>
#include "encoder_spi.h"
#include "EncoderReg.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化编码器应用层（绑定 SPI2 实例、使用阻塞模式）。
 * @note 默认以阻塞传输模式初始化，如需更高吞吐可切换底层为 DMA 模式。
 */
void EncoderApp_Init(void);

/* 通用多命令 16bit 批量收发（CS 全程保持低电平） */
/**
 * @brief 通用 16-bit 批量收发（CS 全程保持低电平）。
 * @param[in]  tx    待发送的 16-bit 指令/数据数组。
 * @param[out] rx    接收缓冲；若为 NULL 则使用内部缓冲。
 * @param[in]  words 传输 16-bit 字数量（注意：单位是 16-bit word）。
 * @return 实际收发的 16-bit 字数；0 表示失败或未初始化。
 */
uint8_t EncoderApp_TransceiveWords(const uint16_t *tx, uint16_t *rx, uint8_t words);

/* MA600 角度读取（基于 Table 5 帧格式） */
/** @brief 读取角度原始值（单帧）。返回同帧 16-bit 原始角度。 */
uint16_t MA600_ReadAngleRaw(void);
/** @brief 读取角度原始值（双帧）。第二帧返回 16-bit 原始角度。 */
uint16_t MA600_ReadAngleRaw_2frame(void);
/** @brief 读取角度（单位：度）。内部将 16-bit 原始值换算为度。 */
float    MA600_ReadAngleDeg(void);

/* MA600 寄存器与NVM高层API */
/** @brief 读寄存器原始返回（16-bit，通常低 8 位为寄存器值）。 */
uint16_t MA600_ReadRegRaw(uint8_t reg);
/** @brief 读寄存器有效值（低 8 位）。 */
uint8_t  MA600_ReadReg(uint8_t reg);
/** @brief 写寄存器（易失性寄存器）。@return 1 成功，0 失败。 */
uint8_t  MA600_WriteReg(uint8_t reg, uint8_t value);

/** @brief 轮询等待 NVMB=0（NVM 空闲）。@return 1 空闲，0 超时。 */
uint8_t  MA600_WaitNVMReady(uint32_t timeout_ms);
/** @brief NVM 存储块 0 或 1（内部等待完成）。@return 1 成功，0 失败。 */
uint8_t  MA600_NVM_StoreBlock(uint8_t blockIndex);
/** @brief NVM 恢复全部寄存器（内部等待完成）。@return 1 成功，0 失败。 */
uint8_t  MA600_NVM_RestoreAll(void);
/** @brief 清除错误标志。@return 1 成功，0 失败。 */
uint8_t  MA600_ClearError(void);
// （已清理：历史的单位圆 EMA 接口改动已移出，这里保留当前公开 API）

#ifdef __cplusplus
}
#endif
