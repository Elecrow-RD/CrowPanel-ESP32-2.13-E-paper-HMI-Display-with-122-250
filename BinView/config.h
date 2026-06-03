#ifndef BINVIEW_CONFIG_H
#define BINVIEW_CONFIG_H

// =============================================================================
// BinView firmware configuration
// CrowPanel ESP32-S3 2.13" e-ink (122 x 250, SSD1680Z / JD79661)
// =============================================================================
//
// Display SPI / control pins are owned by spi.h (SCK 12, MOSI 11, RES 10,
// DC 13, CS 14, BUSY 9) and intentionally NOT re-#defined here to avoid macro
// redefinition warnings. They are reproduced below only as documentation.
//
//   #define SCK  12
//   #define MOSI 11
//   #define RES  10
//   #define DC   13
//   #define CS   14
//   #define BUSY 9
//
// Display power rail. Must be driven HIGH before any EPD operation. This pin is
// NOT part of spi.h, so we define it here.
#define PWR_PIN 7

// -----------------------------------------------------------------------------
// BLE provisioning
// -----------------------------------------------------------------------------
#define BLE_SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define BLE_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define BLE_DEVICE_PREFIX       "BinView-"

// -----------------------------------------------------------------------------
// MQTT
// -----------------------------------------------------------------------------
#define MQTT_PORT            1883
#define MQTT_RECONNECT_DELAY 5000   // ms between MQTT reconnect attempts

// -----------------------------------------------------------------------------
// Power / timing
// -----------------------------------------------------------------------------
#define SLEEP_DURATION_SEC   55     // deep sleep between MQTT polls (normal mode)
#define WIFI_FAIL_SLEEP_SEC  30     // deep sleep after a failed WiFi connect
#define WIFI_CONNECT_TIMEOUT_MS 15000
#define AWAKE_WINDOW_MS      10000  // stay awake this long listening for MQTT

// Full refresh on boot and every Nth refresh to clear e-ink ghosting; all other
// updates use partial refresh to save power.
#define FULL_REFRESH_EVERY   10

// -----------------------------------------------------------------------------
// Display dimensions (physical panel). Drawing coordinate space after the
// driver's USE_HORIZONTIAL==2 rotation is EPD_W=250 (x) by EPD_H=122 (y).
// -----------------------------------------------------------------------------
#define EPD_WIDTH  122
#define EPD_HEIGHT 250

// -----------------------------------------------------------------------------
// Preferences (NVS)
// -----------------------------------------------------------------------------
#define PREF_NAMESPACE "binview"

static const char *PREF_SSID    = "ssid";
static const char *PREF_PASS    = "pass";
static const char *PREF_BROKER  = "broker_ip";
static const char *PREF_BIN_ID  = "bin_id";
static const char *PREF_MAX     = "max_count";
static const char *PREF_CURRENT = "current";
static const char *PREF_UUID    = "device_uuid";

#endif // BINVIEW_CONFIG_H
