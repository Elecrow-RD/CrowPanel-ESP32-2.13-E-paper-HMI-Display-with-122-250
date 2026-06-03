#include "mqtt_client.h"
#include "config.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

static WiFiClient g_wifiClient;
static PubSubClient g_mqtt(g_wifiClient);
static MessageCallback g_messageCallback = nullptr;

// PubSubClient delivers the raw topic + payload here; we parse the JSON and
// forward { current, max, alert } to the user callback.
static void onMqttMessage(char *topic, byte *payload, unsigned int length) {
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload, length);
  if (err) {
    Serial.print("[MQTT] JSON parse error: ");
    Serial.println(err.c_str());
    return;
  }

  int current = doc["current"] | 0;
  int max = doc["max"] | 0;
  bool alert = doc["alert"] | false;

  Serial.printf("[MQTT] %s -> current=%d max=%d alert=%d\n", topic, current, max,
                alert);

  if (g_messageCallback) {
    g_messageCallback(current, max, alert);
  }
}

bool connectMQTT(const String &brokerIp, const String &binId,
                 MessageCallback cb) {
  g_messageCallback = cb;

  g_mqtt.setServer(brokerIp.c_str(), MQTT_PORT);
  g_mqtt.setCallback(onMqttMessage);

  String clientId = "binview-" + binId;
  String topic = "bins/" + binId;

  Serial.printf("[MQTT] connecting to %s:%d as %s\n", brokerIp.c_str(),
                MQTT_PORT, clientId.c_str());

  if (!g_mqtt.connect(clientId.c_str())) {
    Serial.printf("[MQTT] connect failed, state=%d\n", g_mqtt.state());
    return false;
  }

  g_mqtt.subscribe(topic.c_str());
  Serial.printf("[MQTT] connected, subscribed to %s\n", topic.c_str());
  return true;
}

void mqttLoop() {
  g_mqtt.loop();
}

void disconnect() {
  g_mqtt.disconnect();
}
