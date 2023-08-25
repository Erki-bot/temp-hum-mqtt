#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
// #include <ESP8266WiFi.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <string>
#define DHTPIN 4  // Digital pin connected to the DHT sensor
#define GAS_SENSOR_PIN 35
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment the type of sensor in use:
#define DHTTYPE DHT11  // DHT 11
// #define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

// See guide for details on sensor wiring and usage:
//   https://learn.adafruit.com/dht/overview

DHT_Unified dht(DHTPIN, DHTTYPE);

// const char* ssid = "Moov Africa_2.4G_0D360";
// const char* password = "64949961";
// const char* mqtt_server = "192.168.1.105";

const char* ssid = "AGBO1";
const char* password = "ossenou1";
const char* mqtt_server = "192.168.43.205";

// const char* ssid = "RightNetwork.";
// const char* password = "#80b42C8q@9vhqJHb5XtX1";
// const char* mqtt_server = "192.168.87.38";

// const char* ssid = "RightNetwork";
// const char* password = "#80b42C8q@9vhqJHb5XtX1";
// const char* mqtt_server = "192.168.86.130";

// const char* ssid = "theSPACE";
// const char* password = "thespaceNGO4040";
// const char* mqtt_server = "192.168.100.31";

uint32_t delayMS;
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  // Initialize device.
  dht.begin();
  Serial.println(F("DHTxx Unified Sensor Example"));
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print(F("Sensor Type: "));
  Serial.println(sensor.name);
  Serial.print(F("Driver Ver:  "));
  Serial.println(sensor.version);
  Serial.print(F("Unique ID:   "));
  Serial.println(sensor.sensor_id);
  Serial.print(F("Max Value:   "));
  Serial.print(sensor.max_value);
  Serial.println(F("°C"));
  Serial.print(F("Min Value:   "));
  Serial.print(sensor.min_value);
  Serial.println(F("°C"));
  Serial.print(F("Resolution:  "));
  Serial.print(sensor.resolution);
  Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print(F("Sensor Type: "));
  Serial.println(sensor.name);
  Serial.print(F("Driver Ver:  "));
  Serial.println(sensor.version);
  Serial.print(F("Unique ID:   "));
  Serial.println(sensor.sensor_id);
  Serial.print(F("Max Value:   "));
  Serial.print(sensor.max_value);
  Serial.println(F("%"));
  Serial.print(F("Min Value:   "));
  Serial.print(sensor.min_value);
  Serial.println(F("%"));
  Serial.print(F("Resolution:  "));
  Serial.print(sensor.resolution);
  Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;
  pinMode(GAS_SENSOR_PIN,INPUT);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off".
  // Changes the output state according to the message
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if (messageTemp == "on") {
      Serial.println("on");
      digitalWrite(2, HIGH);
    } else if (messageTemp == "off") {
      Serial.println("off");
      digitalWrite(2, LOW);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
  //  boolean connect(const char* id, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage);
    if (client.connect("project1","status/esp2",1,true,"off")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  client.publish("status/esp1","on",true);
}

void readDHT() {
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  } else {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
    publishStringMqtt("home/temp", String(event.temperature));
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  } else {
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
    publishStringMqtt("home/hum", String(event.relative_humidity));
  }
}

void readGas(){
  int val = analogRead(GAS_SENSOR_PIN);
  float gas = (val/4095.0)*100;
  Serial.print("Gas : ====> ");
  Serial.println(gas);
  publishStringMqtt("home/gas",String (gas));
}
void publishJsonMqtt(String topic, String key, String value) {
  String d;

  // d = "{\"distance\":\"";
  d = "{\"" + key + "\":\"";
  d += (String)value;
  d += "\"}";
  // d = "akdjdfdnss";
  // Serial.println(d);
  char datas[d.length() + 1];
  char tp[topic.length() + 1];
  d.toCharArray(datas, sizeof(datas));
  topic.toCharArray(tp, sizeof(tp));
  Serial.println(datas);
  if (!client.publish(tp, datas, true)) {
    Serial.println("Failled to publish");
    Serial.println("Retrying");
    if (!client.connected()) {
      reconnect();
    }
    while (!client.publish(tp, datas, true)) {
      Serial.print('.');
      client.publish(tp, datas, true);
      Serial.println("");
      delay(500);
    }
    Serial.println('Sent');
  }
}
void publishStringMqtt(String topic, String data) {
 
  char datas[data.length() + 1];
  char tp[topic.length() + 1];
  data.toCharArray(datas, sizeof(datas));
  topic.toCharArray(tp, sizeof(tp));
  Serial.println(datas);
  if (!client.publish(tp, datas, true)) {
    Serial.println("Failled to publish");
    Serial.println("Retrying");
    if (!client.connected()) {
      reconnect();
    }
    while (!client.publish(tp, datas, true)) {
      Serial.print('.');
      client.publish(tp, datas, true);
      Serial.println("");
      delay(500);
    }
    Serial.println('Sent');
  }
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  // Delay between measurements.
  delay(delayMS);
  readDHT();
  readGas();
}