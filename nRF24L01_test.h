#define  R_REGISTER      0x00 //SPI读命令
#define W_REGISTER      0x20 //SPI写命令

#define CONFIG 0x00//设置
#define  EN_AA  0x01//启用自动ACK
#define EN_RXADDR 0x02//启动接收通道
#define SETUP_RETR 0x04//设置重发
#define RF_CH 0x05//设置频段
#define RF_SETUP 0x06//设置功率
#define  STATUS        0x07//状态寄存器
#define OBSERVE_TX  0x08//观察发送状态
#define  RX_ADDR_P0      0x0A//通道0的接收地址
#define  RX_ADDR_P1      0x0B//通道1的接收地址
#define  TX_ADDR       0x10 //发送地址寄存器
#define RX_PW_P0 0x11//通道0的数据长度
#define RX_PW_P1 0x12//通道1的数据长度
#define DYNPD 0x1C//启动接收动态数据
#define FEATURE 0x1D//启用动态数据

#define  W_TX_PAYLOAD    0xA0//发射数据区
#define  R_RX_PAYLOAD    0x61//接收数据区
#define FLUSH_TX 0xE1//清空发送区
#define  FLUSH_RX 0xE2//清空接收区
#define R_RX_PL_WID    0x60//接收到的数据长度

#define MASK_TX_DS    0x20//发送完毕
#define MASK_RX_DR    0x40//接收完毕
#define MASK_MAX_RT 0x10//达到最大重发次数

#define W_ACK_PAYLOAD 0xA8//ACK Payload

#define ACTIVATE 0x50//激活



