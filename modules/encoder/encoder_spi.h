/**
 * @file encoder_spi.h
 * @brief 编码器 SPI 传输层与实例定义（SPI2 + 手动片选）。
 */
#pragma once

#include <stdint.h>
#include "bsp_spi.h"
#include "spi.h"
#include "main.h"
#include "fliter.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 接收缓冲区大小（单位：16-bit word） */
#define RX_BUF_SIZE 32
/** 编码器角度一阶低通滤波默认系数（0<alpha<1） */
#define LOWPASS_ALPHA 0.2f

/*
 * - MA600 编码器 SPI 通信模块
 * - 片选：PB12 (SPI2_CS_Pin)
 * - 总线：SPI2，已在 CubeMX/HAL 中配置为 16bit、主机、软片选
 * - 读取数据存放在 rx_buf 中
 */

typedef struct
{
    SPIInstance *spi;               /**< 绑定到 SPI2 的 BSP SPI 实例 */
    uint16_t     rx_buf[RX_BUF_SIZE]; /**< 最近一次接收/收发的数据（16-bit word） */
    uint8_t      rx_len;            /**< 实际接收的 16-bit word 数 */
    float        Angle;             /**< 最近读取/滤波后的角度（单位：度） */
    LowPassFilter angle_filter_sin; /**< sin 分量一阶低通滤波器 */
    LowPassFilter angle_filter_cos; /**< cos 分量一阶低通滤波器 */
} EncoderSPI;

/**
 * @brief 初始化 SPI2 对应的编码器通信实例。
 * @param mode 传输模式（阻塞/IT/DMA）。如无特别需求，推荐阻塞模式 SPI_BLOCK_MODE。
 * @return EncoderSPI* 实例指针
 */
EncoderSPI *EncoderSPI2_Init(SPI_TXRX_MODE_e mode);

/** 获取编码器 SPI 实例句柄（若未初始化则默认初始化一次）。 */
EncoderSPI *EncoderAPP_GetHandle(void);

/**
 * @brief 发送若干 16bit（word）数据。
 * @param enc 实例
 * @param tx 待发送数据指针
 * @param words 发送的 16bit word 数
 */
void EncoderSPI_Transmit(EncoderSPI *enc, const uint16_t *tx, uint8_t words);

/**
 * @brief 接收若干 16bit（word）数据（发送全 0 作为占位）。
 * @param enc 实例
 * @param rx 接收缓冲区
 * @param words 接收的 16bit word 数
 */
void EncoderSPI_Receive(EncoderSPI *enc, uint16_t *rx, uint8_t words);

/**
 * @brief 同时收发若干 16bit（word）数据。
 * @param enc 实例
 * @param rx 接收缓冲区
 * @param tx 发送缓冲区
 * @param words 收发的 16bit word 数
 */
void EncoderSPI_Transceive(EncoderSPI *enc, uint16_t *rx, const uint16_t *tx, uint8_t words);

/** 获取实例内的 sin 分量一阶低通滤波器指针（用于外部诊断/配置）。 */
LowPassFilter *EncoderSPI_GetAngleFilterSin(EncoderSPI *enc);
/** 获取实例内的 cos 分量一阶低通滤波器指针（用于外部诊断/配置）。 */
LowPassFilter *EncoderSPI_GetAngleFilterCos(EncoderSPI *enc);

/**
 * @brief 同时调整 sin/cos 两个角度滤波器的 alpha（不重置输出）。
 * @param enc 实例
 * @param alpha 0<alpha<1，越大响应越快
 */
void EncoderSPI_SetAngleFilterAlpha(EncoderSPI *enc, float alpha);

/**
 * @brief 复位 sin/cos 两个角度滤波器，使下次更新直接对齐当前输入。
 *        不改变 alpha，仅将 initialized 置 0。
 */
void EncoderSPI_ResetAngleFilters(EncoderSPI *enc);

#ifdef __cplusplus
}
#endif
