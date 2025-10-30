#pragma once

#include <stdint.h>
#include "bsp_spi.h"
#include "spi.h"
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RX_BUF_SIZE 32  // 接收缓冲区大小（单位：16bit word）
/*
 * - MA600 编码器 SPI 通信模块
 * - 片选：PB12 (SPI2_CS_Pin)
 * - 总线：SPI2，已在 CubeMX/HAL 中配置为 16bit、主机、软片选
 * - 读取数据存放在 rx_buf 中
 */

typedef struct
{
    SPIInstance *spi;            // 绑定到 SPI2 的 BSP SPI 实例

    // 最近一次接收/收发得到的数据缓冲（单位：16bit word）
    uint16_t     rx_buf[RX_BUF_SIZE];
    uint8_t      rx_len;         // 实际接收的 16bit word 数

    float Angle;                  // 最近读取的角度值（单位：度）
} EncoderSPI;

/*
 * 初始化 SPI2 对应的编码器通信实例。
 * mode: 传输模式（阻塞/IT/DMA）。如无特别需求，推荐阻塞模式 SPI_BLOCK_MODE。
 */
EncoderSPI *EncoderSPI2_Init(SPI_TXRX_MODE_e mode);

/*
 * 获取编码器 SPI 实例句柄
 */
EncoderSPI *EncoderAPP_GetHandle(void);

/*
 * 发送 words 个 16bit 数据。
 * 注意：SPI2 配置为 16bit 数据宽度，len 以“16 位字”为单位，不是字节。
 */
void EncoderSPI_Transmit(EncoderSPI *enc, const uint16_t *tx, uint8_t words);

/*
 * 接收 words 个 16bit 数据（发送全 0）。
 */
void EncoderSPI_Receive(EncoderSPI *enc, uint16_t *rx, uint8_t words);

/*
 * 同时收发 words 个 16bit 数据。
 */
void EncoderSPI_Transceive(EncoderSPI *enc, uint16_t *rx, const uint16_t *tx, uint8_t words);

#ifdef __cplusplus
}
#endif
