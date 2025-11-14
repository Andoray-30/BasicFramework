# 磁编码器模块 (MA600)

本模块提供对 **MA600 磁编码器** 的 SPI 通信与应用层支持，基于 `bsp_spi` 实现了对 SPI2 的封装，并提供了针对 MA600 的高层 API。

## 模块结构

- **`encoder_spi` (传输层)**
  - `encoder_spi.h` / `encoder_spi.c`
  - **功能**：
    - 封装 `bsp_spi`，提供面向 16-bit 数据的 SPI2 通信接口。
    - 管理 SPI2 实例与软片选 (PB12)。
    - 内置角度滤波：对角度的 `sin` 和 `cos` 分量分别进行一阶低通滤波，以获得平滑的角度值。
  - **主要接口**：
    - `EncoderSPI2_Init()`: 初始化 SPI2 实例。
    - `EncoderSPI_Transceive()`: 核心收发函数，支持 16-bit 批量读写。

- **`encoder_app` (应用层)**
  - `encoder_app.h` / `encoder_app.c`
  - **功能**：
    - 基于 `encoder_spi` 实现对 MA600 编码器的具体操作。
    - 提供读取角度、读写寄存器、操作 NVM 等高层 API。
  - **主要接口**：
    - `EncoderApp_Init()`: 初始化应用层。
    - `MA600_ReadAngleDeg()`: 读取经滤波后的角度值（单位：度）。
    - `MA600_ReadReg()` / `MA600_WriteReg()`: 读写寄存器。
    - `MA600_NVM_StoreBlock()` / `MA600_NVM_RestoreAll()`: 存储/恢复配置到非易失性存储器。

- **`EncoderReg.h` (寄存器定义)**
  - **功能**：
    - 宏定义 MA600 的所有寄存器地址、位域、标志位和 SPI 命令帧。
    - 方便应用层以可读性高的方式访问硬件功能。

## 使用方法

### 1. 初始化

在应用入口（如 `robot.c` 的 `RobotInit()`）调用初始化函数：

```c
#include "encoder_app.h"

void RobotInit()
{
    // ... 其他初始化 ...
    EncoderApp_Init(); // 初始化编码器模块
    // ...
}
```

### 2. 读取角度

在需要获取角度的任务中（如 `gimbal.c` 的 `GimbalTask()`），调用 `MA600_ReadAngleDeg()`。

```c
#include "encoder_app.h"

void GimbalTask()
{
    // ...
    float current_angle = MA600_ReadAngleDeg();
    // 使用 current_angle 进行闭环控制或其他逻辑
    // ...
}

```

该函数返回的角度经过了 **sin/cos 分解后的一阶低通滤波**，相比直接读取原始角度，具有更好的平滑性和抗抖动能力。

### 3. 读写寄存器 (高级用法)

如果需要修改编码器配置（如零点、旋转方向、滤波器窗口等），可以使用寄存器读写接口。

#### 示例：设置新的零点位置

1. **计算零点寄存器值**:
   假设要将当前位置设为新的零点，首先读取当前角度原始值。

   ```c

   uint16_t current_raw_angle = MA600_ReadAngleRaw();

   ```

2. **写入零点寄存器**:
   将该值写入 `MA600_ZERO_LOW_REG` 和 `MA600_ZERO_HIGH_REG`。

   ```c

   uint8_t low_byte = (uint8_t)(current_raw_angle & 0xFF);
   uint8_t high_byte = (uint8_t)((current_raw_angle >> 8) & 0xFF);

   MA600_WriteReg(MA600_ZERO_LOW_REG, low_byte);
   MA600_WriteReg(MA600_ZERO_HIGH_REG, high_byte);
   ```

3. **保存到 NVM (可选)**:
   如果希望断电后配置依然生效，需要将寄存器块存入 NVM。零点寄存器位于块 0。
  
   ```c

   if (MA600_NVM_StoreBlock(0)) {
       // 存储成功
   } else {
       // 存储失败 (可能超时)
   }

   ```

   **注意**: NVM 写入有次数限制，请避免频繁操作。

## 角度滤波机制

为了提供稳定、平滑的角度数据，本模块采用 **基于单位圆的 EMA (Exponential Moving Average) 滤波**，具体步骤如下：

1. **读取原始角度**: 获取编码器的 16-bit 原始角度值 `raw_angle`。
2. **分解到单位圆**: 将 `raw_angle` 转换为 `sin` 和 `cos` 两个分量。
   - `sin_val = sin(raw_angle * k)`
   - `cos_val = cos(raw_angle * k)`
3. **独立滤波**: 对 `sin` 和 `cos` 分量分别应用一阶低通滤波 (EMA)。
   - `filtered_sin = alpha * sin_val + (1 - alpha) * prev_filtered_sin`
   - `filtered_cos = alpha * cos_val + (1 - alpha) * prev_filtered_cos`
4. **合成角度**: 使用 `atan2(filtered_sin, filtered_cos)` 将滤波后的分量重新合成为角度值。

**优点**:

- **避免跳变**: 解决了在 0/360 度临界点附近直接对角度值滤波导致的跳变问题。
- **平滑稳定**: 有效抑制高频噪声和机械抖动。
- **可调性**: 滤波系数 `LOWPASS_ALPHA` (在 `encoder_spi.h` 中定义) 可根据需求调整，`alpha` 越大，响应越快，但平滑效果越弱。

## SPI 通信说明

- **总线**: `SPI2`
- **片选 (CS)**: `PB12` (软件片选)
- **数据位宽**: 16-bit
- **模式**: 阻塞/DMA/IT 可选，默认为阻塞模式 (`SPI_BLOCK_MODE`)。

所有 SPI 操作均通过 `bsp_spi` 模块进行，实现了硬件无关性。

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
