# encoder

提供对光电和霍尔磁编码器的支持.

可能需要新增bsp_tim以提供period lapse和tim编码器计数的功能.

## SPI2 通信接口（新增）

本工程已在 `modules/encoder/` 下增加基于 SPI2 的通用编码器通信封装（参考 BMI088 模块的 BSP SPI 方案）：

- 文件：
  - `encoder_spi.h`：对外 API。
  - `encoder_spi.c`：实现，使用 BSP SPI 按软片选控制 PB12（`SPI2_CS_Pin`）。

- 依赖：
  - HAL 已配置 `SPI2` 为 16bit 数据宽度、主机模式、软件 NSS；
  - 片选为 `PB12`，由 BSP 在每次传输前后自动拉低/拉高；
  - SPI2 引脚：PB13 SCK、PB14 MISO、PB15 MOSI（AF5）。

- 典型用法：

```c
#include "encoder_spi.h"

static EncoderSPI *enc;

void EncoderInitExample(void)
{
  // 初始化 SPI2 的编码器通信实例，阻塞模式（也可用 SPI_DMA_MODE / SPI_IT_MODE）
  enc = EncoderSPI2_Init(SPI_BLOCK_MODE);
}

uint16_t ReadOneWord(void)
{
  uint16_t tx = 0x0000; // 依据具体器件协议设置命令，如需读操作/寄存器地址等
  uint16_t rx = 0;
  EncoderSPI_Transceive(enc, &rx, &tx, 1); // 收发 1 个 16bit word
  return rx;
}

void WriteWords(const uint16_t *buf, uint8_t words)
{
  EncoderSPI_Transmit(enc, buf, words); // 发送若干 16bit word
}
```

> 注意：HAL 在 16bit 数据宽度下，传输长度以“16 位字”为单位，非字节。

如需对接具体编码器（例如 AS5048A/MT6701 等），请在此基础上补充芯片协议部分（读命令格式、寄存器地址、奇偶校验等），并在 `encoder_spi.c` 之上实现对应的高层驱动。
