/* CS/SS -  pin 10
* MOSI - pin 11
* MISO - pin 12
* SCK - pin 13
*/
#include <SPI.h>
#include  "nRF24L01_test.h"
#define MODE_TX 0
#define MODE_RX 1
#define INIT_MODE MODE_RX

const int slaveSelectPin = 9;//SPI片选
const int chipEnablePin = 10;//CE 芯片开关

#define TX_ADR_WIDTH 5//发射地址位长度

byte TX_ADDRESS[TX_ADR_WIDTH]  = //发射地址 低位先写入
{
  0xe1, 0xf0, 0xf0, 0xe8, 0xe8
};
#define TX_PAYLOAD_WIDTH 32//发射数据长度
#define RX_PAYLOAD_WIDTH 32//接收数据长度
byte txBuffer[TX_PAYLOAD_WIDTH] = {0};//发射数据
byte rxBuffer[RX_PAYLOAD_WIDTH] = {0};//接收数据
boolean txReady  = true;
boolean ackReady = true;

void setup() {
  pinMode(chipEnablePin, OUTPUT);
  pinMode(slaveSelectPin, OUTPUT);
  digitalWrite(slaveSelectPin, HIGH); //关闭SPI
  SPI.begin();
  Serial.begin(115200);
  Serial.setTimeout(2);
  commonSetting();
#if(INIT_MODE==MODE_TX)
  txMode();
#else
  rxMode();
#endif

  Serial.println(INIT_MODE == MODE_TX ? "TX Mode" : "RX Mode");
}
 byte lenTx = 0;
void loop() {
  byte sstatus = readRegister(STATUS);//读取状态
#if(INIT_MODE==MODE_TX)
  //判断是否发送成功

  if (txReady)
  {
    if (Serial.available())
    {
      byte len = Serial.readBytes(txBuffer, TX_PAYLOAD_WIDTH);
      writeRegister(FLUSH_TX, 0); //清空发送区
      writeRegister(W_TX_PAYLOAD, txBuffer, len); //写入发射数据
      txReady = false;
    }
  }

  if ((sstatus & MASK_TX_DS))
  {
    txReady = true;

  }

  if (sstatus & MASK_MAX_RT)
  {
    txReady = true;
  }

#endif

    #if(INIT_MODE==MODE_RX)
    if(ackReady)
    {
         if(Serial.available())
        {
             byte len = Serial.readBytes(txBuffer,TX_PAYLOAD_WIDTH);
              //写入ACK Payload 当更改PPP时应断电重启
            writeRegister(W_ACK_PAYLOAD  , txBuffer, len); 
            ackReady = false;
        }
    }
#endif
  //判断是否接收成功
  if (sstatus & MASK_RX_DR)
  {
#if(INIT_MODE==MODE_RX)
      ackReady = true;
#endif

    byte len = readRegister(R_RX_PL_WID);
    if (len < 0x21) //32位和以下为正确数据
    {
      readRegister(R_RX_PAYLOAD, rxBuffer, len); //读取接收数据
      Serial.write(rxBuffer, len);
    }
    //    writeRegister(FLUSH_RX, 0); //清空接收区


  }

  if (sstatus > 0x0F)
  {
    writeConfigRegister(STATUS, sstatus); //清除RX_DR TX_DS MAX_RT中断
  }
}

void txMode()
{
  digitalWrite(chipEnablePin, LOW); //关闭芯片
  writeConfigRegister(CONFIG, 0x7e);/*发送模式 启动电源 2位CRC 启动CRC
  启用发送完毕 接受完毕 达到重试次数中断*/
  digitalWrite(chipEnablePin, HIGH); //开启芯片
  delayMicroseconds(130);
}
void rxMode()
{
  digitalWrite(chipEnablePin, LOW); //关闭芯片
  writeConfigRegister(CONFIG, 0x7f);/*接收模式 启动电源 2位CRC 启动CRC
  启用发送完毕 接受完毕 达到重试次数中断*/
  digitalWrite(chipEnablePin, HIGH); //开启芯片
  delayMicroseconds(130);
}

void commonSetting()
{
   writeConfigRegister(TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH); //设置发送地址
  writeConfigRegister(RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH); //通道0的接收地址与发送地址相同
  writeConfigRegister(RX_PW_P0, RX_PAYLOAD_WIDTH); //通道0的接收长度为32位
  writeConfigRegister(EN_AA, 0x01); //启动通道0的ACK
  writeConfigRegister(EN_RXADDR, 0x01); //启动接收通道0
  writeConfigRegister(SETUP_RETR, 0xff); //重发间隔4000us 重发15次
  writeConfigRegister(RF_CH, 0x7d); //2.525GHz
  writeConfigRegister(RF_SETUP, 0x27); //0dBm(SI24R1 7dbm)  250kbps
  writeConfigRegister(FEATURE, 0x06); //启用动态数据 ACK Payload
  writeConfigRegister(DYNPD, 0x01); //启动通道0的动态数据
  writeRegister(W_TX_PAYLOAD, txBuffer, TX_PAYLOAD_WIDTH); //清空发射数据
  writeRegister(R_RX_PAYLOAD, rxBuffer, RX_PAYLOAD_WIDTH); //清空接收数据
}

byte writeConfigRegister(byte registerAddr, byte* datas, int len)
{
  return writeRegister(W_REGISTER + registerAddr, datas, len);
}

byte writeConfigRegister(byte registerAddr, byte data)
{
  return writeConfigRegister(registerAddr, &data, 1);
}

byte writeRegister(byte registerAddr, byte* datas, int len)
{
  digitalWrite(slaveSelectPin, LOW); //选中芯片
  byte sstatus = SPI.transfer(registerAddr);//选择寄存器地址
  for (int i = 0; i < len; i++)
  {
    SPI.transfer(*datas ++);//写入
  }
  digitalWrite(slaveSelectPin, HIGH); //取消选中
  return sstatus;
}

byte writeRegister(byte registerAddr, byte data)
{
  return writeRegister(registerAddr, &data, 1);
}

byte readRegister(byte registerAddr)
{
  byte data = 0;
  readRegister(registerAddr, &data, 1);
  return data;
}

byte readRegister(byte registerAddr, byte* datas, byte len)
{
  digitalWrite(slaveSelectPin, LOW); //选中芯片
  byte sstatus = SPI.transfer(registerAddr);//寄存器地址 R_REGISTER为0 加不加都行
  for (int i = 0; i < len; i++)
  {
    datas[i] = SPI.transfer(0xFF);//读取
  }
  digitalWrite(slaveSelectPin, HIGH); //取消选中
  return sstatus;
}


