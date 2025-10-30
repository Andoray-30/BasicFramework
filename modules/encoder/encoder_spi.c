#include "encoder_spi.h"
#include <string.h>

static EncoderSPI encoder_spi2_instance; // 简单场景：单实例（SPI2）

// 获取编码器 SPI 实例句柄（若未初始化则进行一次默认初始化）
EncoderSPI *EncoderAPP_GetHandle(void)
{
    if (encoder_spi2_instance.spi == NULL)
    {
        // 默认使用阻塞模式，保证兼容性
        (void)EncoderSPI2_Init(SPI_BLOCK_MODE);
    }
    return &encoder_spi2_instance;
}

EncoderSPI *EncoderSPI2_Init(SPI_TXRX_MODE_e mode)
{
    static SPI_Init_Config_s spi_conf;
    memset(&spi_conf, 0, sizeof(spi_conf));

    spi_conf.spi_handle   = &hspi2;                 // 绑定 SPI2
    spi_conf.GPIOx        = SPI2_CS_GPIO_Port;      // PB12
    spi_conf.cs_pin       = SPI2_CS_Pin;            // 片选脚
    spi_conf.spi_work_mode = mode;                  // 传输模式
    spi_conf.callback     = NULL;                   // 编码器通常阻塞/简易用法，无需回调
    spi_conf.id           = NULL;

    encoder_spi2_instance.spi = SPIRegister(&spi_conf);
    encoder_spi2_instance.rx_len = 0;
    memset(encoder_spi2_instance.rx_buf, 0, sizeof(encoder_spi2_instance.rx_buf));

    // 片选由 BSP 层在每次传输时自动拉低/拉高，这里无需额外处理
    return &encoder_spi2_instance;
}

void EncoderSPI_Transmit(EncoderSPI *enc, const uint16_t *tx, uint8_t words)
{
    if (!enc || !enc->spi || !tx || words == 0) return;
    // HAL SPI 在 16bit 数据宽度下，长度单位为“16bit word”，
    // BSP SPI 的 len 直接传递给 HAL，因此这里传 words 即可。
    SPITransmit(enc->spi, (uint8_t *)tx, words);
}

void EncoderSPI_Receive(EncoderSPI *enc, uint16_t *rx, uint8_t words)
{
    if (!enc || !enc->spi || !rx || words == 0) return;
    // 批量一次性收发，确保 CS 全程保持低电平
    uint8_t max_words = (uint8_t)(sizeof(enc->rx_buf) / sizeof(enc->rx_buf[0]));
    uint8_t n = (words > max_words) ? max_words : words;
    uint16_t zeros[RX_BUF_SIZE] = {0}; // 与 rx_buf 同长度
    SPITransRecv(enc->spi, (uint8_t *)rx, (uint8_t *)zeros, n);
    // 将结果同步保存到实例的接收缓冲中
    for (uint8_t i = 0; i < n; ++i)
    {
        enc->rx_buf[i] = rx[i];
    }
    enc->rx_len = n;
}

void EncoderSPI_Transceive(EncoderSPI *enc, uint16_t *rx, const uint16_t *tx, uint8_t words)
{
    if (!enc || !enc->spi || !rx || !tx || words == 0) return;
    // 批量一次性收发，确保 CS 全程保持低电平
    uint8_t max_words = (uint8_t)(sizeof(enc->rx_buf) / sizeof(enc->rx_buf[0]));
    uint8_t n = (words > max_words) ? max_words : words;
    SPITransRecv(enc->spi, (uint8_t *)rx, (uint8_t *)tx, n);
    // 同步保存到实例缓冲
    for (uint8_t i = 0; i < n; ++i)
    {
        enc->rx_buf[i] = rx[i];
    }
    enc->rx_len = n;
}
