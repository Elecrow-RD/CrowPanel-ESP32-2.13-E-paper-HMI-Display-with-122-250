#ifndef BINVIEW_MQTT_CLIENT_H
#define BINVIEW_MQTT_CLIENT_H

#include <Arduino.h>
#include <functional>

// Invoked for each MQTT message received on the subscribed bin topic, after the
// JSON payload { current, max, alert } has been parsed.
typedef std::function<void(int current, int max, bool alert)> MessageCallback;

// Connects to the MQTT broker at brokerIp:MQTT_PORT, using client id
// "binview-<binId>", and subscribes to "bins/<binId>". Returns true on a
// successful connect + subscribe. Assumes WiFi is already connected.
bool connectMQTT(const String &brokerIp, const String &binId,
                 MessageCallback cb);

// Services the MQTT client; call repeatedly while awake.
void mqttLoop();

// Disconnects from the broker.
void disconnect();

#endif // BINVIEW_MQTT_CLIENT_H
