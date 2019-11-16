/*
 * @version: 
 * @Date: 2019-09-26 15:43:39
 * @LastEditors: Golem
 * @Github: Golemherry
 * @LastEditTime: 2019-09-26 19:20:52
 */
/**
    Required libraries:
      - DHT sensor library by Adafruit
      - Adafruit Unified Sensor
      - PubSubClient
**/

#include <DHT.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define DHTPIN D4
#define DHTTYPE DHT22

#define MQTT_TOPIC_HUMIDITY "topic/dht22/Humidity_2"
#define MQTT_TOPIC_TEMPERATURE "topic/dht22/Temperature_2"
#define MQTT_TOPIC_STATE "topic/dht22/status"
#define MQTT_PUBLISH_DELAY 3000
#define MQTT_CLIENT_ID "esp8266dht"

const char *WIFI_SSID = "wifissid";
const char *WIFI_PASSWORD = "wifipassword";

const char *MQTT_SERVER = "hostname";
const char *MQTT_USER = "";     // NULL for no authentication
const char *MQTT_PASSWORD = ""; // NULL for no authentication

float humidity;
float temperature;
long lastMsgTime = 0;

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;

  setupWifi();
  mqttClient.setServer(MQTT_SERVER, 1883);
  dht.begin();
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

    // Reading DHT22 sensor data
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    if (isnan(humidity) || isnan(temperature))
    {
      Serial.println("DHT22 sensor is not ready yet");
      return;
    }

    // Publishing sensor data
    mqttPublish(MQTT_TOPIC_TEMPERATURE, temperature);
    mqttPublish(MQTT_TOPIC_HUMIDITY, humidity);
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
