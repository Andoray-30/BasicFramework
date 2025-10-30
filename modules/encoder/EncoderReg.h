#ifndef ENCODERREG
#define ENCODERREG


// -------------------------- 基础信息寄存器 --------------------------
// 产品ID寄存器（只读）- 产品版本标识
#define MA600_PRODUCT_ID_REG       0x1F
#define MA600_PRODUCT_ID_VALUE     0x3C  // 十进制60，对应规格书Table 10

// 后缀ID寄存器（只读）- 配置代码标识（"-xxxx"）
#define MA600_SUFFIX_ID_REG        0x1E
#define MA600_SUFFIX_ID_DEFAULT    0x00  // 工厂默认配置代码"-0000"

// 状态与错误寄存器（只读）- 含NVM忙标志与错误标志
#define MA600_STATUS_ERR_REG       0x1A
#define MA600_NVMB_SHIFT           7     // NVM忙标志位（1=忙，0=空闲）
#define MA600_NVMB_FLAG            (1 << MA600_NVMB_SHIFT)
#define MA600_ERRCRC_SHIFT         2     // NVM恢复CRC错误标志
#define MA600_ERRCRC_FLAG          (1 << MA600_ERRCRC_SHIFT)
#define MA600_ERRMEM_SHIFT         1     // NVM忙时操作错误标志
#define MA600_ERRMEM_FLAG          (1 << MA600_ERRMEM_SHIFT)
#define MA600_ERRPAR_SHIFT         0     // SPI奇偶校验错误标志
#define MA600_ERRPAR_FLAG          (1 << MA600_ERRPAR_SHIFT)


// -------------------------- 零位与旋转配置寄存器 --------------------------
// 零位配置寄存器（读写）- 零位位置低位（Z[7:0]）
#define MA600_ZERO_LOW_REG         0x00
// 零位配置寄存器（读写）- 零位位置高位（Z[15:8]）
#define MA600_ZERO_HIGH_REG        0x01
// 零位计算：a0(deg) = Z[15:0] * 360 / 65536 ≈ Z[15:0] * 0.005493°
#define MA600_ZERO_20DEG_VALUE     0x0E39  // 20°对应Z值（20*65536/360≈3640=0x0E39）

// 旋转方向寄存器（读写）- 磁场旋转正方向配置
#define MA600_ROT_DIR_REG          0x09
#define MA600_RD_SHIFT             7     // 旋转方向控制位
#define MA600_ROT_CW               (0 << MA600_RD_SHIFT)  // 顺时针（默认）
#define MA600_ROT_CCW              (1 << MA600_RD_SHIFT)  // 逆时针


// -------------------------- 数字滤波器配置寄存器 --------------------------
// 滤波器窗口寄存器（读写）- 控制分辨率、延迟与截止频率
#define MA600_FILTER_WIN_REG       0x0D
#define MA600_FW_SHIFT             0     // 滤波器窗口控制位（4bit）
#define MA600_FW_0                 (0x0 << MA600_FW_SHIFT)  // 无滤波，12.3bit，17kHz截止
#define MA600_FW_5_DEFAULT         (0x5 << MA600_FW_SHIFT)  // 默认，12.5bit，12kHz截止
#define MA600_FW_6                 (0x6 << MA600_FW_SHIFT)  // 13bit，5.8kHz截止
#define MA600_FW_7                 (0x7 << MA600_FW_SHIFT)  // 13.5bit，2.7kHz截止
#define MA600_FW_8                 (0x8 << MA600_FW_SHIFT)  // 14bit，1.3kHz截止
#define MA600_FW_9                 (0x9 << MA600_FW_SHIFT)  // 14.3bit，0.63kHz截止
#define MA600_FW_10                (0xA << MA600_FW_SHIFT)  // 14.6bit，0.31kHz截止
#define MA600_FW_11                (0xB << MA600_FW_SHIFT)  // 14.8bit，0.15kHz截止
#define MA600_FW_12                (0xC << MA600_FW_SHIFT)  // 15bit，0.075kHz截止


// -------------------------- ABZ增量编码器配置寄存器 --------------------------
// ABZ脉冲数配置寄存器（读写）- PPT[11:3]（高位）
#define MA600_ABZ_PPT_HIGH_REG     0x05
// ABZ脉冲数配置寄存器（读写）- PPT[2:0]（低位）+ ILIP[3:0]
#define MA600_ABZ_PPT_ILIP_REG     0x04
#define MA600_PPT_HIGH_SHIFT       0     // PPT[11:3]移位（9bit）
#define MA600_PPT_LOW_SHIFT        4     // PPT[2:0]移位（3bit）
#define MA600_ILIP_SHIFT           0     // ABZ索引脉冲配置（4bit）
// PPT计算：脉冲数/圈 = PPT[11:0] + 1（范围1~4096）
#define MA600_ABZ_512PPT_VALUE     0x1FF // 512脉冲/圈（默认，0x1FF+1=512）
#define MA600_ABZ_120PPT_VALUE     0x77  // 120脉冲/圈（0x77+1=120）

// ABZ滞回配置寄存器（读写）- 避免误触发
#define MA600_ABZ_HYS_REG          0x0C
#define MA600_HYS_SHIFT            0     // 滞回控制（8bit）
// 滞回计算：H(deg) = (HYS[7:0] * 2.8) / 256
#define MA600_HYS_FW5_DEFAULT      0x10  // FW=5默认值（16），H≈0.18°


// -------------------------- UVW增量编码器配置寄存器 --------------------------
// UVW极对数配置寄存器（读写）- 控制UVW换相步数
#define MA600_UVW_NPP_REG          0x07
#define MA600_NPP_SHIFT            0     // 极对数控制（3bit）
#define MA600_UVW_1POLE            (0x0 << MA600_NPP_SHIFT)  // 1极对，6状态/圈，60°步宽
#define MA600_UVW_2POLE            (0x1 << MA600_NPP_SHIFT)  // 2极对，12状态/圈，30°步宽
#define MA600_UVW_3POLE            (0x2 << MA600_NPP_SHIFT)  // 3极对，18状态/圈，20°步宽
#define MA600_UVW_4POLE            (0x3 << MA600_NPP_SHIFT)  // 4极对，24状态/圈，15°步宽
#define MA600_UVW_8POLE            (0x7 << MA600_NPP_SHIFT)  // 8极对，48状态/圈，7.5°步宽


// -------------------------- PWM绝对输出配置寄存器 --------------------------
// PWM配置寄存器（读写）- 频率与错误检测
#define MA600_PWM_CONF_REG         0x08
#define MA600_PWMM_SHIFT           7     // PWM错误检测使能（1=使能，0=禁用）
#define MA600_PWMM_ENABLE          (1 << MA600_PWMM_SHIFT)
#define MA600_PWMM_DISABLE         (0 << MA600_PWMM_SHIFT)
#define MA600_PWMF_SHIFT           6     // PWM频率选择
#define MA600_PWM_250HZ            (0 << MA600_PWMF_SHIFT)  // 250Hz（默认）
#define MA600_PWM_1KHZ             (1 << MA600_PWMF_SHIFT)  // 1kHz


// -------------------------- 多圈与速度配置寄存器 --------------------------
// 多圈/速度控制寄存器（读写）- 含 parity 配置
#define MA600_MT_SPEED_REG         0x1C
#define MA600_MTSP_SHIFT           7     // 多圈/速度选择（1=速度，0=多圈）
#define MA600_MODE_SPEED           (1 << MA600_MTSP_SHIFT)
#define MA600_MODE_MULTITURN       (0 << MA600_MTSP_SHIFT)
#define MA600_PRT_SHIFT            6     // SPI奇偶校验使能（1=使能，0=禁用）
#define MA600_PRT_ENABLE           (1 << MA600_PRT_SHIFT)
#define MA600_PRT_DISABLE          (0 << MA600_PRT_SHIFT)
#define MA600_PRTS_SHIFT           5     // 奇偶校验类型（1=奇，0=偶）
#define MA600_PARITY_ODD           (1 << MA600_PRTS_SHIFT)
#define MA600_PARITY_EVEN          (0 << MA600_PRTS_SHIFT)
#define MA600_APRT_SHIFT           4     // 角度数据奇偶校验使能（1=使能）
#define MA600_APRT_ENABLE          (1 << MA600_APRT_SHIFT)

// 多圈偏移寄存器（读写）- 偏移低位（MTOFFSET[7:0]）
#define MA600_MT_OFFSET_LOW_REG    0x12
// 多圈偏移寄存器（读写）- 偏移高位（MTOFFSET[15:8]）
#define MA600_MT_OFFSET_HIGH_REG   0x13
#define MA600_MT_OFFSET_DEFAULT    0x0000 // 默认偏移0


// -------------------------- I/O引脚配置寄存器 --------------------------
// I/O模式配置寄存器（读写）- 开漏/推挽与SPI引脚状态
#define MA600_IO_MODE_REG          0x0B
#define MA600_OD615_SHIFT          5     // IO6/IO1/IO5模式（1=开漏，0=推挽）
#define MA600_IO615_OPENDRAIN      (1 << MA600_OD615_SHIFT)
#define MA600_IO615_PUSHPULL       (0 << MA600_OD615_SHIFT)
#define MA600_OD243_SHIFT          4     // IO2/IO4/IO3模式（1=开漏，0=推挽）
#define MA600_IO243_OPENDRAIN      (1 << MA600_OD243_SHIFT)
#define MA600_IO243_PUSHPULL       (0 << MA600_OD243_SHIFT)
#define MA600_SPULLIN_SHIFT        3     // SPI输入（/CS/SCLK/COPI）高阻使能
#define MA600_SPI_IN_HIZ           (1 << MA600_SPULLIN_SHIFT)
#define MA600_TRISTATE_SHIFT       2     // CIPO空闲状态（1=高阻，0=下拉）
#define MA600_CIPO_IDLE_HIZ        (1 << MA600_TRISTATE_SHIFT)
#define MA600_CIPO_IDLE_PD         (0 << MA600_TRISTATE_SHIFT)

// I/O功能映射寄存器（读写）- 选择IO1~IO6功能（ABZ/UVW/SSI/PWM）
#define MA600_IO_MAP_REG           0x0E
#define MA600_INTF_SEL_SHIFT       0     // 接口选择（3bit）
#define MA600_INTF_ABZ             (0x0 << MA600_INTF_SEL_SHIFT)  // ABZ输出
#define MA600_INTF_UVW             (0x1 << MA600_INTF_SEL_SHIFT)  // UVW输出
#define MA600_INTF_SSI             (0x2 << MA600_INTF_SEL_SHIFT)  // SSI接口
#define MA600_INTF_PWM             (0x3 << MA600_INTF_SEL_SHIFT)  // PWM输出
#define MA600_DAZ_SHIFT            4     // DAZ模式使能（1=使能，方向+A+Z）
#define MA600_DAZ_ENABLE           (1 << MA600_DAZ_SHIFT)
#define MA600_CK100_SHIFT          0     // 100kHz时钟输出使能（1=使能）
#define MA600_CK100_ENABLE         (1 << MA600_CK100_SHIFT)


// -------------------------- 特殊模式配置寄存器 --------------------------
// 菊花链/简化布线模式控制寄存器（读写）- 需UR10解锁
#define MA600_DAISY_RWM_REG        0x0A
#define MA600_DAISY_SHIFT          7     // 菊花链模式使能（1=使能）
#define MA600_DAISY_ENABLE         (1 << MA600_DAISY_SHIFT)
#define MA600_RWM_SHIFT            0     // 简化布线模式使能（1=使能）
#define MA600_RWM_ENABLE           (1 << MA600_RWM_SHIFT)

// 解锁寄存器（读写）- 解锁DAISY/RWM配置
#define MA600_UNLOCK_REG           0x84
#define MA600_UR10_SHIFT           0     // 解锁位（1=解锁，0=锁定）
#define MA600_UNLOCK_ENABLE        (1 << MA600_UR10_SHIFT)
#define MA600_UNLOCK_DISABLE       (0 << MA600_UR10_SHIFT)


// -------------------------- 校准寄存器（32点用户校准表） --------------------------
// 校准表寄存器（读写）- CORR0~CORR31（对应32个校准点）
#define MA600_CORR0_REG            0x20  // 第一个校准点（0°）
#define MA600_CORR1_REG            0x21  // 第二个校准点（11.25°）
#define MA600_CORR2_REG            0x22  // 第三个校准点（22.5°）
// ... 中间省略CORR3~CORR29 ...
#define MA600_CORR30_REG           0x3E  // 第三十一个校准点（337.5°）
#define MA600_CORR31_REG           0x3F  // 第三十二个校准点（348.75°）
// 校准值范围：-11.25°~+11.25°（8bit有符号数）
#define MA600_CORR_0_45DEG         0x05  // 0.45°校准值（十进制5）
#define MA600_CORR_NEG_0_07DEG     0xFF  // -0.07°校准值（十进制255）


// -------------------------- SPI命令宏定义（规格书Table 5） --------------------------
// 读角度（16bit帧）
#define MA600_CMD_READ_ANGLE       0x0000
// 读多圈/速度（32bit帧，MTSP=0读多圈，MTSP=1读速度）
#define MA600_CMD_READ_MT_SPEED    0x00000000
// 读寄存器（2帧：第一帧0xD2+地址，第二帧0x0000）
#define MA600_CMD_READ_REG_PREFIX  0xD200  // 高8bit为命令，低8bit填寄存器地址
#define MA600_CMD_READ_REG_SUFFIX  0x0000
// 写寄存器（3帧：第一帧0xEA54，第二帧地址+值，第三帧0x0000）
#define MA600_CMD_WRITE_REG_CMD1   0xEA54
#define MA600_CMD_WRITE_REG_CMD3   0x0000
// NVM存储单个寄存器块（3帧：第一帧0xEA55，第二帧0xEA00/0xEA01，第三帧0x0000）
#define MA600_CMD_NVM_STORE_CMD1   0xEA55
#define MA600_CMD_NVM_STORE_BLOCK0 0xEA00  // 存储块0（REG0~REG31）
#define MA600_CMD_NVM_STORE_BLOCK1 0xEA01  // 存储块1（REG32~REG63）
#define MA600_CMD_NVM_STORE_CMD3   0x0000
// NVM恢复所有寄存器块（2帧：第一帧0xEA56，第二帧0x0000）
#define MA600_CMD_NVM_RESTORE_CMD1 0xEA56
#define MA600_CMD_NVM_RESTORE_CMD2 0x0000
// 清除错误标志（2帧：第一帧0xD700，第二帧0x0000）
#define MA600_CMD_CLEAR_ERR_CMD1   0xD700
#define MA600_CMD_CLEAR_ERR_CMD2   0x0000


#endif // !ENCODERREG