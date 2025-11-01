/**
 * @file encoder_app.c
 * @brief 编码器应用层实现（MA600 高层 API、角度读取、批量收发）。
 */
#include "encoder_app.h"
#include "encoder_spi.h"
#include "EncoderReg.h"
#include "bsp_log.h"
#include "bsp_dwt.h"
#include <math.h>

static EncoderSPI *s_enc = NULL;
// 分别用于对 sin 与 cos 分量进行一阶低通

// MA600 原始角度（16-bit，满量程 0..65535）换算为“度”的系数
/** 原始 16-bit 角度换算为“度”的系数（360/65536）。 */
static const float kRawToDeg = 360.0f / 65536.0f;
// 角度换算为弧度的系数
static const float kDeg2Rad = 3.14159265358979323846f / 180.0f;
// 弧度换算为角度的系数
static const float kRad2Deg = 180.0f / 3.14159265358979323846f;

/** 初始化应用层（绑定 SPI2，默认阻塞模式）。 */
void EncoderApp_Init(void)
{
    // 默认启用阻塞传输（优先稳定性）；如需更高吞吐可切换为 SPI_DMA_MODE
    s_enc = EncoderSPI2_Init(SPI_BLOCK_MODE);
    // 应用层融合滤波器初始化：设置 alpha 并复位，使首样本直接对齐输入
    if (s_enc)
    {
        EncoderSPI_SetAngleFilterAlpha(s_enc, LOWPASS_ALPHA);
        EncoderSPI_ResetAngleFilters(s_enc);
    }
}

// ---------------- MA600 高层：寄存器 / NVM ----------------

// 读取寄存器的有效值（返回低 8 位）
// 参数：reg = 寄存器地址
// 返回：寄存器低 8 位的数值
uint8_t MA600_ReadReg(uint8_t reg)
{
    return (uint8_t)(MA600_ReadRegRaw(reg) & 0xFF);
}

// 读取寄存器原始返回值（16-bit）
// 帧格式：两帧 [0xD2|addr], [0x0000]
// 返回：第二帧接收到的 16-bit 原始值
uint16_t MA600_ReadRegRaw(uint8_t reg)
{
    if (!s_enc)
        return 0;
    const uint16_t tx[2] = { (uint16_t)(MA600_CMD_READ_REG_PREFIX | reg), MA600_CMD_READ_REG_SUFFIX };
    uint16_t rx[2] = {0};
    EncoderApp_TransceiveWords(tx, rx, 2);
    return rx[1];
}

// 写寄存器（易失性寄存器）
// 帧格式：三帧 [0xEA54], [addr|value], [0x0000]
// 返回：1=成功，0=失败（未初始化等）
uint8_t MA600_WriteReg(uint8_t reg, uint8_t value)
{
    if (!s_enc)
        return 0;
    const uint16_t tx[3] = { MA600_CMD_WRITE_REG_CMD1, (uint16_t)(((uint16_t)reg << 8) | value), MA600_CMD_WRITE_REG_CMD3 };
    uint16_t rx[3] = {0};
    EncoderApp_TransceiveWords(tx, rx, 3);
    return 1;
}

// 轮询状态寄存器，等待 NVMB 位清零（NVM 空闲）
// 参数：timeout_ms = 超时时间（毫秒）
// 返回：1=空闲就绪，0=超时未就绪
uint8_t MA600_WaitNVMReady(uint32_t timeout_ms)
{
    float start = DWT_GetTimeline_s();
    while (((DWT_GetTimeline_s() - start) * 1000.0f) < (float)timeout_ms)
    {
        uint8_t status = MA600_ReadReg(MA600_STATUS_ERR_REG);
        if ((status & MA600_NVMB_FLAG) == 0)
            return 1; // 空闲
        DWT_Delay(0.001f); // 轮询间隔 1 ms
    }
    return 0; // 超时
}

// 读取角度（单位：度）
/** 读取角度（单位：度）。 */
float MA600_ReadAngleDeg(void)
{
    float sina = 0.0f, cosa = 0.0f, arct = 0.0f;
    uint16_t raw = MA600_ReadAngleRaw();
    
    sina = sinf(((float)raw) * kRawToDeg * kDeg2Rad);
    cosa = cosf(((float)raw) * kRawToDeg * kDeg2Rad);
    // 使用实例内的独立一阶低通滤波器对 sin 和 cos 分量分别滤波
    if (s_enc)
    {
        lowpass_filter_update(&s_enc->angle_filter_sin, sina);
        sina = s_enc->angle_filter_sin.output;

        lowpass_filter_update(&s_enc->angle_filter_cos, cosa);
        cosa = s_enc->angle_filter_cos.output;
    }
    else
    {
        // 如果实例未就绪则直接使用原值
    }

    arct = atan2f(sina, cosa) * kRad2Deg;
    if (arct < 0.0f)
        arct += 360.0f;

    if (s_enc)
    {
        s_enc->Angle = arct; // 更新实例内的角度值（度）
    }

    return arct;
}

// 读取角度（原始 16-bit）：单帧
// 帧格式：发送 [0x0000]；返回同帧 16-bit 角度
/** 读取角度原始值（单帧，返回同帧 16-bit）。 */
uint16_t MA600_ReadAngleRaw(void)
{
    if (!s_enc)
        return 0;
    const uint16_t tx = MA600_CMD_READ_ANGLE;
    uint16_t rx = 0;
    EncoderApp_TransceiveWords(&tx, &rx, 1);
    return rx;
}

// 读取角度（原始 16-bit）：双帧
// 帧格式：发送 [0x0000, 0x0000]；返回第二帧 16-bit 角度
/** 读取角度原始值（双帧，第二帧返回 16-bit）。 */
uint16_t MA600_ReadAngleRaw_2frame(void)
{
    if (!s_enc)
        return 0;
    const uint16_t tx[2] = { MA600_CMD_READ_ANGLE, 0x0000 };
    uint16_t rx[2] = {0};
    EncoderApp_TransceiveWords(tx, rx, 2);
    return rx[1];
}

// NVM 存储指定块（0 或 1）
// 帧格式：三帧 [0xEA55], [0xEA00/0xEA01], [0x0000]
// 返回：1=成功（含等待 NVMB 清零），0=失败
/** NVM 存储指定块（0 或 1）。 */
uint8_t MA600_NVM_StoreBlock(uint8_t blockIndex)
{
    if (!s_enc)
        return 0;
    uint16_t cmd2 = (blockIndex == 0) ? MA600_CMD_NVM_STORE_BLOCK0 : MA600_CMD_NVM_STORE_BLOCK1;
    const uint16_t tx[3] = { MA600_CMD_NVM_STORE_CMD1, cmd2, MA600_CMD_NVM_STORE_CMD3 };
    uint16_t rx[3] = {0};
    EncoderApp_TransceiveWords(tx, rx, 3);
    // 存储过程可能较慢，这里等待 NVMB 清零（典型数十毫秒，这里给 100 ms）
    return MA600_WaitNVMReady(100);
}

// NVM 恢复全部寄存器
// 帧格式：两帧 [0xEA56], [0x0000]
// 返回：1=成功（含等待 NVMB 清零），0=失败
/** NVM 恢复全部寄存器。 */
uint8_t MA600_NVM_RestoreAll(void)
{
    if (!s_enc)
        return 0;
    const uint16_t tx[2] = { MA600_CMD_NVM_RESTORE_CMD1, MA600_CMD_NVM_RESTORE_CMD2 };
    uint16_t rx[2] = {0};
    EncoderApp_TransceiveWords(tx, rx, 2);
    return MA600_WaitNVMReady(100);
}

// 清除错误标志
// 帧格式：两帧 [0xD700], [0x0000]
// 返回：1=成功，0=失败
/** 清除错误标志。 */
uint8_t MA600_ClearError(void)
{
    if (!s_enc)
        return 0;
    const uint16_t tx[2] = { MA600_CMD_CLEAR_ERR_CMD1, MA600_CMD_CLEAR_ERR_CMD2 };
    uint16_t rx[2] = {0};
    EncoderApp_TransceiveWords(tx, rx, 2);
    return 1;
}

// 通用 16-bit 批量收发（CS 全程保持低电平）
// 参数：tx = 待发送 16-bit 数组；rx = 接收缓冲；words = 传输 16-bit 字数
// 返回：实际收发的 16-bit 字数；0 表示失败
/** 批量 16-bit 收发封装（CS 保持低电平）。 */
uint8_t EncoderApp_TransceiveWords(const uint16_t *tx, uint16_t *rx, uint8_t words)
{
    if (!s_enc || !tx || words == 0)
        return 0;
    uint8_t max_words = (uint8_t)(sizeof(s_enc->rx_buf) / sizeof(s_enc->rx_buf[0]));
    uint8_t n = (words > max_words) ? max_words : words;
    if (!rx)
        rx = s_enc->rx_buf;
    EncoderSPI_Transceive(s_enc, rx, tx, n);
    return n;
}

