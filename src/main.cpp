#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include "U8g2lib.h"

#include "sensor.h"
#include "display.h"
#include "virtual_thread.h"

#define LED 2
#define CONNECT_TIMEOUT_MS 6000
#define PUBLISH_TIMEOUT_MS 6000
#define PING_TIMEOUT_MS 6000
#define SUBACK_TIMEOUT_MS 6000

const char DEGREE_SYMBOL[] = {0xB0, '\0'};

const uint32_t LOOP_DELAY_MS = 1000;
const uint32_t PUBLISH_DELAY_MS = 300'000; // 5 minutes
const uint32_t POWER_SAVE_DELAY_MS = 600'000; // 10 minutes

// WIFI credentials
const char *ssid = "***"; // WIFI SSID
const char *pass = "***"; // WIFI password
WiFiClient client;

// MQTT credentials
const char *mqtt_username = "***"; // MQTT username
const char *mqtt_password = "***"; // MQTT password
const int mqtt_port = 1883;
const char *mqtt_broker = "homeassistant.local"; // MQTT broker IP
const char *topic = "home";                // MQTT topic base name
PubSubClient clientMqtt(client);

struct MqttTopics
{
  struct Office
  {
    const char *temp = "home/office/temp";
    const char *humi = "home/office/humi";
  };
  struct Outside
  {
    const char *temp = "home/outside/temp";
    const char *humi = "home/outside/humi";
  };
  Office office;
  Outside outside;
};
MqttTopics mqttTopics;

// BUZZER pin
uint8_t BUZZER = D1;
// DHT pin
uint8_t DHTPin_IN = D2;
uint8_t DHTPin_OUT = D3;
// Button pin
uint8_t BUTTON = D4;
// OLED pin
uint8_t OLED_SCL = D7;
uint8_t OLED_SDA = D6;

// Initialize DHT sensor.
TempSensor<DHT> dht_in("Inside", D2, DHT22);
TempSensor<DHT> dht_out("Outside", D3, DHT21);

// OLED display
U8G2_SH1106_128X64_NONAME_F_HW_I2C oledDisplay(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /*clock =*/OLED_SCL, /* data =*/OLED_SDA);

// MQTT callback function
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (unsigned int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");
}

void setup()
{
  // prepare(Serial);
  Serial.begin(9600);
  delay(10);

  pinMode(LED, OUTPUT);
  pinMode(BUTTON, INPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(LED, HIGH);

  oledDisplay.begin();
  oledDisplay.clearBuffer();
  oledDisplay.setFont(u8g2_font_ncenR08_tr);
  oledDisplay.drawStr(0, 20, "Connecting to WiFi:");
  oledDisplay.drawStr(0, 40, ssid);
  oledDisplay.sendBuffer();

  Serial.println(" Connecting to ");
  Serial.println(ssid);

  // Connecting to WiFi
  WiFi.begin(ssid, pass);
  size_t i = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(50);
    Serial.print(".");
    oledDisplay.drawStr(i, 60, ".");
    oledDisplay.sendBuffer();
    if (i >= 110)
    {
      i = 0;
      oledDisplay.clearBuffer();
      oledDisplay.setFont(u8g2_font_ncenR08_tr);
      oledDisplay.drawStr(0, 20, "Connecting to WiFi:");
      oledDisplay.drawStr(0, 40, ssid);
      oledDisplay.sendBuffer();
    }
    i++;
  }

  Serial.println("\nWiFi connected!");
  Serial.print("NodeMCU IP address: ");
  Serial.println(WiFi.localIP());

  // Connecting to a MQTT broker
  clientMqtt.setServer(mqtt_broker, mqtt_port);
  clientMqtt.setCallback(callback);
  oledDisplay.clearBuffer();
  oledDisplay.setFont(u8g2_font_ncenR10_tr);
  oledDisplay.drawStr(0, 40, "Connecting to MQTT");
  size_t attempt = 0;
  while (!clientMqtt.connected())
  {
    oledDisplay.drawStr(attempt, 60, ".");
    oledDisplay.sendBuffer();
    String client_id = "esp8266-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public MQTT broker, ", client_id.c_str());
    if (clientMqtt.connect(client_id.c_str(), mqtt_username, mqtt_password))
    {
      Serial.println("Public MQTT broker connected");

      digitalWrite(LED, LOW);
      // oledDisplay.clearBuffer();
      // oledDisplay.setFont(u8g2_font_ncenR08_tr);
      // oledDisplay.drawStr(0, 20, "MQTT connected! \n");
      // oledDisplay.sendBuffer();
      delay(50);
      digitalWrite(LED, HIGH);
      delay(150);
    }
    else
    {
      oledDisplay.clearBuffer();
      oledDisplay.setFont(u8g2_font_ncenR08_tr);
      oledDisplay.drawStr(0, 20, "MQTT failed with");
      oledDisplay.drawStr(0, 40, "state: " + clientMqtt.state());
      oledDisplay.drawStr(0, 60, "Retrying...");
      oledDisplay.sendBuffer();
      Serial.print("Failed with state: ");
      Serial.print(clientMqtt.state());
      delay(2000);
      if (attempt > 10)
      {
        Serial.print("Failed to connect to MQTT broker for 10 times, skip...");
        break;
      }
      attempt++;
    }
  }
}

// Write sensor data to OLED display
void writeOutputs()
{
  dht_in.read();
  dht_out.read();
  auto dht_in_map = dht_in.toMap();
  auto dht_out_map = dht_out.toMap();

  oledDisplay.clearBuffer();
  oledDisplay.setFont(u8g2_font_ncenR08_tr);
  oledDisplay.drawStr(90, 10, dht_in_map["name"].c_str());
  oledDisplay.drawStr(83, 45, dht_out_map["name"].c_str());

  oledDisplay.setFont(u8g2_font_luBS19_tf);
  oledDisplay.drawStr(0, 30, (dht_in_map["temp"] + DEGREE_SYMBOL).c_str());
  oledDisplay.drawStr(0, 63, (dht_out_map["temp"] + DEGREE_SYMBOL).c_str());

  oledDisplay.setFont(u8g2_font_luBS10_tf);
  oledDisplay.drawStr(70, 30, (dht_in_map["humi"] + "%").c_str());
  oledDisplay.drawStr(70, 63, (dht_out_map["humi"] + "%").c_str());

  oledDisplay.sendBuffer();
  Serial.print(dht_in.toString());
  Serial.print(dht_out.toString());
  Serial.print("\n");
}

// Handling button for power save mode
void handleButton(SimpleVirtualThread &powerSaveThread)
{
  if (!digitalRead(BUTTON))
  {
    // Wake up display
    powerSaveThread.reset();
    digitalWrite(LED, LOW);
    digitalWrite(BUZZER, HIGH);
    oledDisplay.setPowerSave(0);
  }
  else
  {
    digitalWrite(LED, HIGH);
    digitalWrite(BUZZER, LOW);
  }
}

void mqttPublish()
{
  dht_in.read();
  dht_out.read();
  auto dht_in_map = dht_in.toMap();
  auto dht_out_map = dht_out.toMap();

  if(!clientMqtt.connected())
  {
    // TODO: refactor - move to a separate function ()
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("WiFi disconnected, reconnecting...");
      WiFi.begin(ssid, pass);
      while (WiFi.status() != WL_CONNECTED)
      {
        delay(50);
        Serial.print(".");
      }
      Serial.println("\nWiFi connected!");
      Serial.print("NodeMCU IP address: ");
      Serial.println(WiFi.localIP());
    }
    Serial.println("MQTT disconnected, reconnecting...");
    String client_id = "esp8266-client-";
    client_id += String(WiFi.macAddress());
    // TODO: refactor - create function for connecting + retrying + logging (WiFi and MQTT)
    size_t attempt = 0;
    while (!clientMqtt.connect(client_id.c_str(), mqtt_username, mqtt_password))
    {
      Serial.println("MQTT reconnect failed, retrying...");
      delay(2000);
      if (attempt > 10)
      {
        Serial.print("Failed to connect to MQTT broker for 10 times, skip...");
        break;
      }
      attempt++;
    }
    Serial.println("MQTT reconnected");

  }

  clientMqtt.loop();
  bool state = true;
  state *= clientMqtt.publish(mqttTopics.office.temp, dht_in_map["temp"].c_str());
  state *= clientMqtt.publish(mqttTopics.office.humi, dht_in_map["humi"].c_str());
  state *= clientMqtt.publish(mqttTopics.outside.temp, dht_out_map["temp"].c_str());
  state *= clientMqtt.publish(mqttTopics.outside.humi, dht_out_map["humi"].c_str());
  Serial.println("Topic published" + String(state ? " successfully" : " failed"));
}

SimpleVirtualThread outputThread(LOOP_DELAY_MS, PUBLISH_DELAY_MS, true, writeOutputs);
SimpleVirtualThread powerSaveThread(LOOP_DELAY_MS, POWER_SAVE_DELAY_MS, false, []()
                                    { oledDisplay.setPowerSave(1); });
SimpleVirtualThread mqttThread(LOOP_DELAY_MS, PUBLISH_DELAY_MS, true, mqttPublish);

size_t i = 0;
void loop()
{
  bool new_run = outputThread.run();
  powerSaveThread.run();
  mqttThread.run();
  handleButton(powerSaveThread);

  delay(LOOP_DELAY_MS);
}
