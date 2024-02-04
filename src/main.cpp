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

const uint32_t LOOP_DELAY_MS = 100;
const uint32_t PUBLISH_DELAY_MS = 10000;
const uint32_t POWER_SAVE_DELAY_MS = 10000; // 2 minutes

const char *ssid = "Homerun-Guest"; // WIFI SSID
const char *pass = "HomerunPass5"; // WIFI password
WiFiClient client;

// Setting up MQTT client
const char *mqtt_broker = "192.168.0.123";  // MQTT broker IP
const char *topic = "s1";                   // MQTT topic
const char *topic1_humi = *topic + "/humi"; // MQTT sub-topic
const char *mqtt_username = "***";          // MQTT username
const char *mqtt_password = "***";          // MQTT password
const int mqtt_port = 1883;
// PubSubClient clientMqtt(client);

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
  oledDisplay.setFont(u8g2_font_ncenB10_tr);
  oledDisplay.drawStr(0, 20, "Connecting to WiFi:");
  oledDisplay.drawStr(0, 40, ssid);
  oledDisplay.sendBuffer();

  Serial.println(" Connecting to ");
  Serial.println(ssid);

  // // Connecting to WiFi
  // WiFi.begin(ssid, pass);
  // size_t i = 20;
  // while (WiFi.status() != WL_CONNECTED)
  // {
  //   delay(50);
  //   Serial.print(".");
  //   oledDisplay.drawStr(i, 60, ".");
  //   oledDisplay.sendBuffer();
  //   i++;
  // }

  // Serial.println("\nWiFi connected!");
  // Serial.print("NodeMCU IP address: ");
  // Serial.println(WiFi.localIP());

  // connecting to a mqtt broker
  // clientMqtt.setServer(mqtt_broker, mqtt_port);
  // clientMqtt.setCallback(callback);
  // while (!clientMqtt.connected())
  // {
  //   String client_id = "esp8266-client-";
  //   client_id += String(WiFi.macAddress());
  //   Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
  //   if (clientMqtt.connect(client_id.c_str(), mqtt_username, mqtt_password))
  //   {
  //     Serial.println("Public emqx mqtt broker connected");
  //     for (int i = 0; i < 3; i++)
  //     {
  //       digitalWrite(LED, LOW);
  //       delay(50);
  //       digitalWrite(LED, HIGH);
  //       delay(50);
  //     }
  //   }
  //   else
  //   {
  //     Serial.print("failed with state ");
  //     Serial.print(clientMqtt.state());
  //     delay(2000);
  //   }
  // }
  // // publish and subscribe
  // clientMqtt.publish(topic, "hello sooo");
  // clientMqtt.subscribe(topic);
}

// void sendMqtt(float temp, float humi
void writeOutputs() {
  // Blink LED
  // digitalWrite(LED, LOW);
  // digitalWrite(BUZZER, HIGH);
  // delay(50);
  // digitalWrite(LED, HIGH);
  // digitalWrite(BUZZER, LOW);
  dht_in.read();
  dht_out.read();
  // TemperatureDisplay tempDisplay = TemperatureDisplay(oledDisplay, std::list<ISensor<TempSensorReadings>>{dht_in, dht_out});
  // TemperatureDisplay outsideTempDisp = TemperatureDisplay(oledDisplay, dht_out);
  // insideTempDisp.draw(0,20);
  // outsideTempDisp.draw(0,40);
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
  oledDisplay.drawStr(70, 30, (dht_in_map["humi"]).c_str());
  oledDisplay.drawStr(70, 63, (dht_out_map["humi"]).c_str());

  // oledDisplay.setFont(u8g2_font_ncenB14_tr);
  oledDisplay.sendBuffer();

  Serial.print(dht_in.toString());
  Serial.print(dht_out.toString());
  Serial.print("\n");
}

// Handling button for power save mode
void handleButton(SimpleVirtualThread& powerSaveThread) {
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


SimpleVirtualThread outputThread(LOOP_DELAY_MS, PUBLISH_DELAY_MS, true, writeOutputs);
SimpleVirtualThread powerSaveThread(LOOP_DELAY_MS, POWER_SAVE_DELAY_MS, false, [](){oledDisplay.setPowerSave(1);});

size_t i = 0;
void loop()
{
  bool new_run = outputThread.run();
  powerSaveThread.run();
  handleButton(powerSaveThread);
  // oledDisplay.drawStr(i, 60, ".");
  // oledDisplay.sendBuffer();
  // i++;
  // if (new_run) {
  //   i = 0;
  // }

  delay(LOOP_DELAY_MS);
  // if (clientMqtt.connected())
  // {
  //   clientMqtt.loop();
  //   getDHTData();
  //   delay(600e3);
  // }
  // else
  // {
  //   Serial.println("Client disconnected! Trying to reconnect.");
  //   delay(1000);
  //   setup();
  // }
}



