# Easy_iot

> developped with esp8266, MQTT, Influxdb, Grafana, Prometheus, Cadvisor

## Introduction of easy iot

- Data collected from nodes(ESP8266) would be published to MQTT
- A broker written in Python would subscribe that topic and write to InfluxDB via HTTP
- Data would be visualized in Grafana
- All deployment would be monitored by Prometheus

## Feature

- All applications have been containerization by using Docker
- Easy to deploy and realize
## Demo
[<image src="assets/link.jpeg" width="5%">](http://47.100.114.83)

> Username: **demo**
> 
> Password: **publicdemo**

## Quick Start

### Server

``` shell
git clone https://github.com/GolemHerry/Easy_iot
cd Easy_iot/broker
vim conf.ini
# set up InfluxDB and MQTT infomation
docker build -t mqtt_broker .
cd .. && docker-compose up -d
```

### Node

> Download required libraries in Arduino IDE
>
> Set up wifi and MQTT server
>
> Upload program to ESP8266

## Screenshot

<details>
<summary>Weather monitor</summary>
<pre><image src="assets/dorm.png" width="100%"></pre>
</details>
<details>
<summary>Power monitor</summary>
<pre><image src="assets/poweMonitor.png" width="100%"></pre>
</details> 
<details>
<summary>Container monitor</summary>
<pre><image src="assets/containers.png" width="100%"></pre>
</details>
