/*
 * @version: 
 * @Date: 2019-09-28 10:41:10
 * @LastEditors: Golem
 * @Github: Golemherry
 * @LastEditTime: 2019-10-07 13:07:25
 */
#include <Wire.h>
#include <ESP8266WiFi.h>

#define DEBUG_CMD ">>GetVal"
#define DELAY_MS 1000

u8 RxBuf[1024] = {0};  //串口接收缓存
u32 RxCnt = 0;         //接收计数
float Irms = 0;        //电流有效值
float Vrms = 0;        //电压有效值
float Frequency = 0;   //频率
float PowerFactor = 1; //功率因数
float PActive = 0;     //有功功率
double W_KWH = 0;      //累积功耗

long lastMsgTime = 0;
String rx_msg = "";
void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);
  while ((!Serial) || (!Serial1))
    ;
  Serial.println("Serial is OK!");
}

void loop()
{

  long now = millis();
  if (now - lastMsgTime > DELAY_MS)
  {
    lastMsgTime = now;
    SUI_101A_Get(1);
  }
}

u8 SUI_101A_Get(u8 adder)
{
  String msg_rx;
  u8 t = 20;
  u8 rxlen = 0;
  u8 i = 0;
  u8 sum = 0;
  u8 n = 0;
  u8 CmdTxBuf[] = {0x55, 0x55, 0x01, 0x02, 0x00, 0x00, 0xAD};
  CmdTxBuf[2] = adder;
  RxCnt = 0;
  Serial1.write(CmdTxBuf, 7);
  delay(10);
  RxCnt = Serial.readBytes(RxBuf, 32);
  while (t)
  {
    t--;
    rxlen = RxCnt;
    delay(10); //等待5ms,连续超过5ms没有接收到一个数据,则认为接收结束
    if ((rxlen == RxCnt) && (rxlen != 0))
    { //接收到了数据,且接收完成了
      if (rxlen == (RxBuf[5] + 7))
      {
        //数据长度正确
      }
      else
      {
        return 3; //异常,数据长度错误
      }
      sum = 0;
      rxlen -= 1; //除去校验位的长度
      for (i = 0; i < rxlen; i++)
      {
        sum += RxBuf[i];
      }
      if (sum == RxBuf[rxlen])
      { //校验和正确
        Vrms = (double)(((u32)RxBuf[6] << 24) | ((u32)RxBuf[7] << 16) | ((u32)RxBuf[8] << 8) | ((u32)RxBuf[9] << 0)) / 1000.0;
        Irms = (double)(((u32)RxBuf[10] << 24) | ((u32)RxBuf[11] << 16) | ((u32)RxBuf[12] << 8) | ((u32)RxBuf[13] << 0)) / 1000.0;
        PActive = (double)(((u32)RxBuf[14] << 24) | ((u32)RxBuf[15] << 16) | ((u32)RxBuf[16] << 8) | ((u32)RxBuf[17] << 0)) / 1000.0;
        n = 18;
        PowerFactor = (double)(s32)(((s32)RxBuf[n++] << 24) | ((s32)RxBuf[n++] << 16) | ((s32)RxBuf[n++] << 8) | ((s32)RxBuf[n++] << 0)) / 10000.0;
        Frequency = (double)(((u32)RxBuf[n++] << 24) | ((u32)RxBuf[n++] << 16) | ((u32)RxBuf[n++] << 8) | ((u32)RxBuf[n++] << 0)) / 1000.0;
        W_KWH = (double)(((u32)RxBuf[n++] << 24) | ((u32)RxBuf[n++] << 16) | ((u32)RxBuf[n++] << 8) | ((u32)RxBuf[n++] << 0)) / 10000.0;
      }
      else
      { //数据校验错误
        return 1;
      }
      break;
    }
  }
  if (t == 0)
  {
    return 2; //超时
  }
  Serial.printf(" | V:%10.05f | I:%10.05f | P:%10.05f | PF:%10.05f | F:%10.05f | W:%10.05f |\r\n", Vrms, Irms, PActive, PowerFactor, Frequency, W_KWH);
  return 0;
}