/*
   @version:
   @Date: 2019-09-08 08:28:08
 * @LastEditors: Golem
   @Github: Golemherry
 * @LastEditTime: 2019-09-16 20:33:34
*/
// SCL--> D1   SDA --> D2
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define MQTT_TOPIC_HUMIDITY "topic/bme280/humidity"
#define MQTT_TOPIC_TEMPERATURE "topic/bme280/temperature"
#define MQTT_TOPIC_ALTITUDE "topic/bme280/altitude"
#define MQTT_TOPIC_STATE "topic/bme280/status"
#define MQTT_PUBLISH_DELAY 3000
#define MQTT_CLIENT_ID "esp8266bme280"

#define BME280_ADDRESS 0x76

const char *WIFI_SSID = "wifissid";
const char *WIFI_PASSWORD = "wifipassword";

const char *MQTT_SERVER = "hostname";
const char *MQTT_USER = "";     // NULL for no authentication
const char *MQTT_PASSWORD = ""; // NULL for no authentication

float humidity;
float temperature;
float altitude;
long lastMsgTime = 0;

Adafruit_BME280 bme;
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("Have found a BME280 sensor");

  if (!bme.begin(BME280_ADDRESS))
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring or BME-280 address!");
    while (1)
      ;
  }

  // Use force mode so that the sensor returns to sleep mode when the measurement is finished
  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1, // temperature
                  Adafruit_BME280::SAMPLING_X1, // pressure
                  Adafruit_BME280::SAMPLING_X1, // humidity
                  Adafruit_BME280::FILTER_X2);

  setupWifi();
  mqttClient.setServer(MQTT_SERVER, 1883);
}

void loop()
{
  if (!mqttClient.connected())
  {
    mqttReconnect();
  }
  mqttClient.loop();

  long now = millis();
  if (now - lastMsgTime > MQTT_PUBLISH_DELAY)
  {
    lastMsgTime = now;

    // Reading BME280 sensor data
    bme.takeForcedMeasurement(); // has no effect in normal mode
    humidity = bme.readHumidity();
    temperature = bme.readTemperature();
    altitude = bme.readAltitude(1013.25);

    if (isnan(humidity) || isnan(temperature))
    {
      Serial.println("BME280 reading issues");
      return;
    }

    // Publishing sensor data
    mqttPublish(MQTT_TOPIC_TEMPERATURE, temperature);
    mqttPublish(MQTT_TOPIC_HUMIDITY, humidity);
    mqttPublish(MQTT_TOPIC_ALTITUDE, altitude);
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
