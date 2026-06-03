// =============================================================================
// BinView firmware -- CrowPanel ESP32-S3 2.13" e-ink (122 x 250)
//
// Warehouse bin tag: shows current/max ASIN count and a QR code of the bin id.
// Counts arrive over MQTT; WiFi/MQTT/bin credentials are provisioned over BLE
// on first boot. Deep-sleeps between polls; the e-ink panel retains its image
// at zero power.
//
// Arduino IDE settings:
//   Board: ESP32S3 Dev Module | Flash Size: 8MB | PSRAM: OPI PSRAM
//   Partition Scheme: 8M with spiffs | Upload Speed: 921600
//
// Required libraries (Library Manager):
//   PubSubClient (Nick O'Leary), NimBLE-Arduino, qrcode (Richard Moore),
//   ArduinoJson. Display driver is Elecrow's own EPD library (bundled here).
//
// V1 scope: buttons (MENU refresh / BACK flag) are intentionally NOT
// implemented -- deferred to v2 via OTA.
// =============================================================================

#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include <esp_sleep.h>
#include <esp_system.h>
#include <ArduinoJson.h>

#include "config.h"
#include "display.h"
#include "ble_provision.h"
#include "mqtt_client.h"
#include "EPD.h"

// State that must survive deep sleep (the chip resets on wake). Used to choose
// full vs partial refresh and to skip redundant redraws.
RTC_DATA_ATTR uint32_t g_refreshCount = 0;
RTC_DATA_ATTR int g_lastCurrent = -1;
RTC_DATA_ATTR int g_lastMax = -1;

Preferences prefs;

// Loaded configuration.
static String g_ssid, g_pass, g_broker, g_binId, g_uuid;
static int g_maxCount = 0;
static int g_current = 0;

// Normal-mode runtime.
static String g_runtimeBinId;
static unsigned long g_awakeDeadline = 0;

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

static void deepSleepSeconds(uint32_t seconds) {
  Serial.printf("[PWR] deep sleep %u s\n", seconds);
  Serial.flush();
  esp_sleep_enable_timer_wakeup((uint64_t)seconds * 1000000ULL);
  esp_deep_sleep_start();
}

// Generates a hex UUID-like string seeded from the hardware (efuse) MAC plus
// random bytes, so it is stable enough to identify the unit on first boot.
static String generateUUID() {
  uint64_t mac = ESP.getEfuseMac();
  uint32_t r1 = esp_random();
  uint32_t r2 = esp_random();
  char buf[33];
  snprintf(buf, sizeof(buf), "%08X%08X%08X%08X", (uint32_t)(mac >> 16),
           (uint32_t)(mac & 0xFFFF) << 16 | (r1 >> 16), r1 << 16 | (r2 >> 16),
           r2);
  return String(buf);
}

static void loadConfig() {
  g_ssid = prefs.getString(PREF_SSID, "");
  g_pass = prefs.getString(PREF_PASS, "");
  g_broker = prefs.getString(PREF_BROKER, "");
  g_binId = prefs.getString(PREF_BIN_ID, "");
  g_uuid = prefs.getString(PREF_UUID, "");
  g_maxCount = prefs.getInt(PREF_MAX, 0);
  g_current = prefs.getInt(PREF_CURRENT, 0);
}

// -----------------------------------------------------------------------------
// Provisioning mode
// -----------------------------------------------------------------------------

// Called from the BLE task when a full JSON payload arrives. Persists the
// configuration and reboots into normal mode.
static void onProvision(const String &json) {
  Serial.print("[PROV] received: ");
  Serial.println(json);

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, json);
  if (err) {
    Serial.print("[PROV] JSON parse error: ");
    Serial.println(err.c_str());
    return;
  }

  String ssid = doc["ssid"] | "";
  if (ssid.length() == 0) {
    Serial.println("[PROV] missing ssid, ignoring");
    return;
  }

  prefs.putString(PREF_SSID, ssid);
  prefs.putString(PREF_PASS, String(doc["password"] | ""));
  prefs.putString(PREF_BROKER, String(doc["broker_ip"] | ""));
  prefs.putString(PREF_BIN_ID, String(doc["bin_id"] | ""));
  prefs.putInt(PREF_MAX, (int)(doc["max_count"] | 0));
  prefs.putInt(PREF_CURRENT, 0);

  Serial.println("[PROV] saved, rebooting");
  stopBLE();
  displayStatus("Connecting...");
  delay(200);
  ESP.restart();
}

static void runProvisioningMode() {
  Serial.println("[MODE] provisioning");

  if (g_uuid.length() == 0) {
    g_uuid = generateUUID();
    prefs.putString(PREF_UUID, g_uuid);
    Serial.print("[PROV] generated uuid: ");
    Serial.println(g_uuid);
  }

  String bleName = String(BLE_DEVICE_PREFIX) + g_uuid.substring(0, 8);

  displayProvisioning(g_uuid, bleName);
  startBLEProvisioning(bleName, onProvision);
  // Stays awake; work happens in onProvision (which reboots). loop() idles.
}

// -----------------------------------------------------------------------------
// Normal mode
// -----------------------------------------------------------------------------

// Renders the bin view, choosing a full refresh on boot and every
// FULL_REFRESH_EVERY frames to clear ghosting; otherwise partial.
static void renderBinView(int current, int max) {
  bool fullRefresh =
      (g_refreshCount == 0) || (g_refreshCount % FULL_REFRESH_EVERY == 0);
  bool alert = (max > 0) && (current >= max);
  displayBinView(g_runtimeBinId, current, max, alert, fullRefresh);
  g_refreshCount++;
  g_lastCurrent = current;
  g_lastMax = max;
}

// MQTT message handler: persist the new count, redraw (unless unchanged), and
// extend the awake window so we keep listening.
static void onMessage(int current, int max, bool alert) {
  prefs.putInt(PREF_CURRENT, current);
  if (max > 0) prefs.putInt(PREF_MAX, max);

  if (current == g_lastCurrent && max == g_lastMax) {
    Serial.println("[NORM] count unchanged, skipping redraw");
  } else {
    renderBinView(current, max);
  }

  g_awakeDeadline = millis() + AWAKE_WINDOW_MS;
}

static void runNormalMode() {
  Serial.println("[MODE] normal");
  g_runtimeBinId = g_binId;

  // --- WiFi ---
  displayStatus("WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(g_ssid.c_str(), g_pass.c_str());

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED &&
         millis() - start < WIFI_CONNECT_TIMEOUT_MS) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[NORM] WiFi failed");
    displayStatus("WiFi Error");
    deepSleepSeconds(WIFI_FAIL_SLEEP_SEC);
    return; // unreachable
  }
  Serial.print("[NORM] WiFi connected, IP ");
  Serial.println(WiFi.localIP());

  // --- MQTT ---
  displayStatus("MQTT...");
  if (!connectMQTT(g_broker, g_binId, onMessage)) {
    Serial.println("[NORM] MQTT failed");
    displayStatus("MQTT Error");
    WiFi.disconnect(true);
    deepSleepSeconds(SLEEP_DURATION_SEC);
    return; // unreachable
  }

  // --- Initial render of last-known state ---
  renderBinView(g_current, g_maxCount);

  // --- Listen for updates ---
  g_awakeDeadline = millis() + AWAKE_WINDOW_MS;
  while ((long)(millis() - g_awakeDeadline) < 0) {
    mqttLoop();
    delay(10);
  }

  // --- Sleep ---
  disconnect();
  WiFi.disconnect(true);
  deepSleepSeconds(SLEEP_DURATION_SEC);
}

// -----------------------------------------------------------------------------
// Arduino entry points
// -----------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println("\n[BOOT] BinView");

  displayInit(); // power the display rail (GPIO 7)

  prefs.begin(PREF_NAMESPACE, false);
  loadConfig();

  if (g_ssid.length() == 0) {
    runProvisioningMode(); // returns; loop() idles while BLE handles the rest
  } else {
    runNormalMode(); // ends in deep sleep
  }
}

void loop() {
  // Normal mode never reaches here (setup() deep-sleeps). In provisioning mode
  // the BLE task drives everything; just idle.
  delay(1000);
}
