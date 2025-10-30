#include "encoder_app.h"
#include "encoder_spi.h"
#include "EncoderReg.h"
#include "bsp_log.h"
#include "bsp_dwt.h"

static EncoderSPI *s_enc = 0;
static float s_last_log_time = 0.0f;

void EncoderApp_Init(void)
{
    // 使用阻塞模式，稳定易用；如需高吞吐可改为 SPI_DMA_MODE
    s_enc = EncoderSPI2_Init(SPI_BLOCK_MODE);
}

// ---------------- MA600 高层：寄存器读写与NVM ----------------

// 读寄存器值：返回低8位
uint8_t MA600_ReadReg(uint8_t reg)
{
    return (uint8_t)(MA600_ReadRegRaw(reg) & 0xFF);
}

// 读寄存器：两帧 0xD2|addr, 0x0000；返回第二帧原始16bit
uint16_t MA600_ReadRegRaw(uint8_t reg)
{
    if (!s_enc)
        return 0;
    const uint16_t tx[2] = { (uint16_t)(MA600_CMD_READ_REG_PREFIX | reg), MA600_CMD_READ_REG_SUFFIX };
    uint16_t rx[2] = {0};
    EncoderApp_TransceiveWords(tx, rx, 2);
    return rx[1];
}

// 写寄存器：三帧 0xEA54, addr|value, 0x0000
uint8_t MA600_WriteReg(uint8_t reg, uint8_t value)
{
    if (!s_enc)
        return 0;
    const uint16_t tx[3] = { MA600_CMD_WRITE_REG_CMD1, (uint16_t)(((uint16_t)reg << 8) | value), MA600_CMD_WRITE_REG_CMD3 };
    uint16_t rx[3] = {0};
    EncoderApp_TransceiveWords(tx, rx, 3);
    return 1;
}

// 读取状态寄存器并等待 NVMB 清零
uint8_t MA600_WaitNVMReady(uint32_t timeout_ms)
{
    float start = DWT_GetTimeline_s();
    while (((DWT_GetTimeline_s() - start) * 1000.0f) < (float)timeout_ms)
    {
        uint8_t status = MA600_ReadReg(MA600_STATUS_ERR_REG);
        if ((status & MA600_NVMB_FLAG) == 0)
            return 1; // 空闲
        DWT_Delay(0.001f); // 1ms 间隔
    }
    return 0; // 超时
}

// 角度获取函数，主要调用对象
float MA600_ReadAngleDeg(void)
{
    uint16_t raw = MA600_ReadAngleRaw();
    return ((float)raw) * (360.0f / 65536.0f);
}

// 读取角度：单帧（发送 0x0000），返回同帧的 16bit 原始角度
uint16_t MA600_ReadAngleRaw(void)
{
    if (!s_enc)
        return 0;
    const uint16_t tx = MA600_CMD_READ_ANGLE;
    uint16_t rx = 0;
    EncoderApp_TransceiveWords(&tx, &rx, 1);
    return rx;
}

// 读取角度：双帧（发送 0x0000 + 0x0000），返回第二帧的 16bit 原始角度
uint16_t MA600_ReadAngleRaw_2frame(void)
{
    if (!s_enc)
        return 0;
    const uint16_t tx[2] = { MA600_CMD_READ_ANGLE, 0x0000 };
    uint16_t rx[2] = {0};
    EncoderApp_TransceiveWords(tx, rx, 2);
    return rx[1];
}

// NVM 存储：三帧 0xEA55, 0xEA00/0xEA01, 0x0000
uint8_t MA600_NVM_StoreBlock(uint8_t blockIndex)
{
    if (!s_enc)
        return 0;
    uint16_t cmd2 = (blockIndex == 0) ? MA600_CMD_NVM_STORE_BLOCK0 : MA600_CMD_NVM_STORE_BLOCK1;
    const uint16_t tx[3] = { MA600_CMD_NVM_STORE_CMD1, cmd2, MA600_CMD_NVM_STORE_CMD3 };
    uint16_t rx[3] = {0};
    EncoderApp_TransceiveWords(tx, rx, 3);
    // 存储过程可能较慢，等待NVMB清零
    return MA600_WaitNVMReady(100); // 典型几十毫秒；这里给100ms
}

// NVM 恢复所有寄存器：两帧 0xEA56, 0x0000
uint8_t MA600_NVM_RestoreAll(void)
{
    if (!s_enc)
        return 0;
    const uint16_t tx[2] = { MA600_CMD_NVM_RESTORE_CMD1, MA600_CMD_NVM_RESTORE_CMD2 };
    uint16_t rx[2] = {0};
    EncoderApp_TransceiveWords(tx, rx, 2);
    return MA600_WaitNVMReady(100);
}

// 清除错误标志：两帧 0xD700, 0x0000
uint8_t MA600_ClearError(void)
{
    if (!s_enc)
        return 0;
    const uint16_t tx[2] = { MA600_CMD_CLEAR_ERR_CMD1, MA600_CMD_CLEAR_ERR_CMD2 };
    uint16_t rx[2] = {0};
    EncoderApp_TransceiveWords(tx, rx, 2);
    return 1;
}

// 通用 16bit 批量收发（CS 全程保持低电平）
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

// ---------------- 10ms 读取角度与产品ID并打印 ----------------
void EncoderApp_TestTick10ms(void)
{
    if (!s_enc)
        return;
    float now = DWT_GetTimeline_s();
    if (now - s_last_log_time >= 0.01f)
    {
        s_last_log_time = now;
        uint16_t raw = MA600_ReadAngleRaw();
        float deg = ((float)raw) * (360.0f / 65536.0f);
        uint8_t product_id = MA600_ReadReg(MA600_PRODUCT_ID_REG);
        LOGINFO("[ENC] angle_raw=0x%04X angle=%.2f deg, product_id=0x%02X", (unsigned)raw, deg, (unsigned)product_id);
    }
}
