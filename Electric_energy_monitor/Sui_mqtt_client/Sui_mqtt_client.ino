/*
 * @version: 2.0
 * @Date: 2019-09-28 10:41:10
 * @LastEditors: Golem
 * @Github: Golemherry
 * @LastEditTime: 2019-10-07 13:58:24
 */
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define MQTT_TOPIC_IRMS "tz/sui101a/Irms"
#define MQTT_TOPIC_VRMS "tz/sui101a/Vrms"
#define MQTT_TOPIC_FREQUENCY "tz/sui101a/Frequency"
#define MQTT_TOPIC_POWERFACTOR "tz/sui101a/PowerFactor"
#define MQTT_TOPIC_PACTIVE "tz/sui101a/PActive"
#define MQTT_TOPIC_W_KWH "tz/sui101a/W_KWH"
#define MQTT_TOPIC_STATE "tz/sui101a/status"
#define MQTT_TOPIC_SWITCH "tz/sui101a/switch"
#define MQTT_CLIENT_ID "PowerMonitorStation"
#define ON (u8)1
#define OFF (u8)0

const char *WIFI_SSID = "Golem";
const char *WIFI_PASSWORD = "12345678!";
const char *MQTT_SERVER = "47.100.114.83";
const char *MQTT_USER = "";     // NULL for no authentication
const char *MQTT_PASSWORD = ""; // NULL for no authentication

WiFiClient espClient;
PubSubClient mqttClient(espClient);

#define SWITCH 14
#define DEBUG_CMD ">>GetVal"
#define DELAY_MS 1000

u8 on_off = ON;
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
    setupWifi();
    pinMode(SWITCH, OUTPUT);
    mqttClient.setServer(MQTT_SERVER, 1883);
    mqttClient.setCallback(callback);
    mqttClient.subscribe(MQTT_TOPIC_SWITCH);
}

void loop()
{
    if (!mqttClient.connected())
    {
        mqttReconnect();
    }
    mqttClient.loop();

    long now = millis();
    if (now - lastMsgTime > DELAY_MS)
    {
        if (on_off)
        {
            digitalWrite(SWITCH, HIGH);
        }
        else
        {
            digitalWrite(SWITCH, LOW);
        }
        lastMsgTime = now;
        SUI_101A_Get(1);
        mqttPublish(MQTT_TOPIC_IRMS, Irms);
        mqttPublish(MQTT_TOPIC_VRMS, Vrms);
        mqttPublish(MQTT_TOPIC_FREQUENCY, Frequency);
        mqttPublish(MQTT_TOPIC_POWERFACTOR, PowerFactor);
        mqttPublish(MQTT_TOPIC_PACTIVE, PActive);
        mqttPublish(MQTT_TOPIC_W_KWH, W_KWH);
    }
}
void callback(char *topic, byte *payload, unsigned int length)
{
    if ((char)payload[0] == '1')
    {
        on_off = ON;
    }
    else
    {
        on_off = OFF;
    }
}
void setupWifi()
{
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void mqttReconnect()
{
    while (!mqttClient.connected())
    {
        Serial.print("Attempting MQTT connection...");

        // Attempt to connect
        if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, MQTT_TOPIC_STATE, 1, true, "disconnected", false))
        {
            Serial.println("connected");

            // Once connected, publish an announcement...
            mqttClient.publish(MQTT_TOPIC_STATE, "connected", true);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}
void mqttPublish(char *topic, float payload)
{
    Serial.print(topic);
    Serial.print(": ");
    Serial.println(payload);

    mqttClient.publish(topic, String(payload).c_str(), true);
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
    // Serial.printf(" | V:%10.05f | I:%10.05f | P:%10.05f | PF:%10.05f | F:%10.05f | W:%10.05f |\r\n", Vrms, Irms, PActive, PowerFactor, Frequency, W_KWH);
    return 0;
}