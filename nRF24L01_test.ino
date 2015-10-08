  /* CS/SS -  pin 10
  * MOSI - pin 11
  * MISO - pin 12
  * SCK - pin 13
*/
#include <SPI.h>
#include  "nRF24L01_test.h"

const int slaveSelectPin = 9;//SPI片选
const int chipEnablePin = 10;//CE 芯片开关

#define TX_ADR_WIDTH 5//发射地址位长度

byte TX_ADDRESS[TX_ADR_WIDTH]  = //发射地址
{
  0x34,0x43,0x10,0x10,0x01
};
#define TX_PAYLOAD_WIDTH 32//发射数据长度
#define RX_PAYLOAD_WIDTH 32//接收数据长度
byte txBuffer[TX_PAYLOAD_WIDTH] = {0};//发射数据
byte rxBuffer[RX_PAYLOAD_WIDTH] = {0};//接收数据

void setup() {
  pinMode(chipEnablePin,OUTPUT);
  pinMode(slaveSelectPin,OUTPUT);
  digitalWrite(slaveSelectPin,HIGH);//关闭SPI
  SPI.begin();
  Serial.begin(115200);
  Serial.setTimeout(2);

  rxMode();
  Serial.println("Booted");
}

void loop() {
  int k = 0;
  while(true)
  {    
    byte sstatus = readRegister(STATUS);//读取状态

   //判断是否发送成功
//   if(sstatus & MASK_TX_DS)
//    {
//        writeRegister(FLUSH_TX,0);//清空发送区
//        while(!Serial.available())
//        {
//        }
//        int len = Serial.readBytes(txBuffer,TX_PAYLOAD_WIDTH);  
//         
//         writeRegister(W_TX_PAYLOAD,txBuffer,len);//写入发射数据
//         Serial.println("Sended");
//    }
    
   //判断是否接收成功
    if(sstatus & MASK_RX_DR)
    {
        byte len = readRegister(R_RX_PL_WID);
        if(len < 0x21)//32位和以下为正确数据
        {
           readRegister(W_RX_PAYLOAD,rxBuffer,len);//读取接收数据
          Serial.write(rxBuffer,len);
        }
       
       writeRegister(FLUSH_RX,0);//清空接收区
    }
    
    if(sstatus > 0x0F)
    {
      writeConfigRegister(STATUS,sstatus);//清除RX_DR TX_DS MAX_RT中断
    }
  }
}

void txMode()
{
  digitalWrite(chipEnablePin,LOW);//关闭芯片
  writeConfigRegister(TX_ADDR,TX_ADDRESS,TX_ADR_WIDTH);//设置发送地址
  writeConfigRegister(RX_ADDR_P0,TX_ADDRESS,TX_ADR_WIDTH);//通道0的接收地址与发送地址相同
  writeConfigRegister(EN_AA,0x01);//启动通道0的ACK
  writeConfigRegister(EN_RXADDR,0x01);//启动接收通道0
  writeConfigRegister(SETUP_RETR,0x1a);//重发间隔500us 重发次数10次
  writeConfigRegister(RF_CH,40);//频段40 2.440GHz
  writeConfigRegister(RF_SETUP,0x0E);//0dBm  2Mbps
  writeConfigRegister(FEATURE,0x04);//启用动态数据
  writeConfigRegister(DYNPD,0x01);//启动通道0的动态数据
  writeConfigRegister(CONFIG,0x0e);/*发送模式 启动电源 2位CRC 启动CRC 
  启用发送完毕 接受完毕 达到重试次数中断*/
  writeRegister(W_TX_PAYLOAD,txBuffer,TX_PAYLOAD_WIDTH);//写入发射数据

  byte enaa = readRegister(EN_AA);
  Serial.println(enaa,BIN);
  digitalWrite(chipEnablePin,HIGH);//开启芯片
}
void rxMode()
{
  digitalWrite(chipEnablePin,LOW);//关闭芯片
  writeConfigRegister(RX_ADDR_P0,TX_ADDRESS,TX_ADR_WIDTH);//通道0的接收地址与发送地址相同
  writeConfigRegister(EN_AA,0x01);//启动通道0的ACK
  writeConfigRegister(EN_RXADDR,0x01);//启动接收通道0
  writeConfigRegister(SETUP_RETR,0x1a);//重发间隔500us 重发次数10次
  writeConfigRegister(RF_CH,40);//频段40 2.440GHz
  writeConfigRegister(RF_SETUP,0x0E);//0dBm  2Mbps
  writeConfigRegister(RX_PW_P0,RX_PAYLOAD_WIDTH);//通道0的接收长度为32位
  writeConfigRegister(FEATURE,0x04);//启用动态数据 貌似不启用也能用
  writeConfigRegister(DYNPD,0x01);//启动通道0的动态数据

  writeConfigRegister(CONFIG,0x0f);/*接收模式 启动电源 2位CRC 启动CRC 
  启用发送完毕 接受完毕 达到重试次数中断*/
  writeRegister(W_RX_PAYLOAD,rxBuffer,RX_PAYLOAD_WIDTH);//清空接收数据

  byte enaa = readRegister(EN_AA);
  Serial.println(enaa,BIN);
  digitalWrite(chipEnablePin,HIGH);//开启芯片
}

byte writeConfigRegister(byte registerAddr,byte* datas,int len)
{
  return writeRegister(W_REGISTER + registerAddr,datas,len);
}

byte writeConfigRegister(byte registerAddr,byte data)
{
   return writeConfigRegister(registerAddr,&data,1);
}

byte writeRegister(byte registerAddr,byte* datas,int len)
{
   digitalWrite(slaveSelectPin,LOW);//选中芯片
   byte sstatus = SPI.transfer(registerAddr);//选择寄存器地址
   for(int i=0;i<len;i++)
   {
      SPI.transfer(*datas ++);//写入
    }
   digitalWrite(slaveSelectPin,HIGH);//取消选中
   return sstatus;
 }

byte writeRegister(byte registerAddr,byte data)
{
   return writeRegister(registerAddr,&data,1);
}

byte readRegister(byte registerAddr)
{
    byte data = 0;
    readRegister(registerAddr,&data,1);
    return data;
}

byte readRegister(byte registerAddr,byte* datas,byte len)
{
   digitalWrite(slaveSelectPin,LOW);//选中芯片
   byte sstatus = SPI.transfer(registerAddr);//寄存器地址 R_REGISTER为0 加不加都行 
   for(int i=0;i<len;i++)
   {
      datas[i] = SPI.transfer(0);//读取
    }
   digitalWrite(slaveSelectPin,HIGH);//取消选中
   return sstatus;
}
