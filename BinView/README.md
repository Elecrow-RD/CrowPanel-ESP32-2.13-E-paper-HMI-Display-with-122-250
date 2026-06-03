# BinView Firmware

Warehouse bin-tag firmware for the **CrowPanel ESP32-S3 2.13" e-ink** display
(122 × 250, SSD1680Z / JD79661). Shows the current ASIN count (`current/max`)
plus a QR code of the bin ID. Counts arrive over MQTT; WiFi/MQTT/bin credentials
are provisioned over BLE on first boot. The device deep-sleeps between polls and
relies on the e-ink panel holding its image at zero power.

Uses Elecrow's bundled EPD driver (`EPD.*`, `EPD_Init.*`, `spi.*`, `EPDfont.h`),
**not** GxEPD2.

## Required libraries (Arduino Library Manager)

| Library | Author / version | Use |
|---|---|---|
| PubSubClient | Nick O'Leary (2.8) | MQTT |
| NimBLE-Arduino | h2zero (**2.x** API) | BLE provisioning |
| ArduinoJson | Benoit Blanchon (v7) | JSON parsing |

> **QRCode** by Richard Moore is **vendored into the sketch** as `qrcode_rm.{h,c}`
> rather than installed via Library Manager. The ESP32 core (3.x) bundles its own
> `espressif__qrcode/include/qrcode.h`, so `#include <qrcode.h>` resolves to the
> wrong header. Vendoring under a unique name (`qrcode_rm.h`) avoids the clash —
> same approach used for the EPD driver.

Verified to compile with **arduino-cli 1.5.0 + esp32:esp32 3.3.8** for
`esp32:esp32:esp32s3` (exit 0, no warnings in the sketch sources).

## Arduino IDE settings

- Board: **ESP32S3 Dev Module**
- Flash Size: **8MB**
- PSRAM: **OPI PSRAM**
- Partition Scheme: **8M with spiffs**
- Upload Speed: **921600**

## Behaviour

**Provisioning mode** (no stored WiFi credentials): generates a device UUID and
starts a NimBLE GATT server. BLE advertising carries the **Service UUID**
`4fafc201-…` in the advertisement packet and the device name
`BinView-<first 8 hex of UUID>` in the **scan response** (a 128-bit UUID plus the
full name won't fit in one 31-byte advertisement). The e-ink shows a QR encoding
**JSON** so the Android app reads the UUID and exact BLE name in one scan
(QR version 4, ECC LOW):

```json
{"uuid":"<32 hex>","ble":"BinView-XXXXXXXX"}
```

The app writes the provisioning JSON to characteristic
`beb5483e-36e1-4688-b7f5-ea07361b26a8` (WRITE / WRITE_NR). Chunked writes are
reassembled until a complete JSON object arrives, then saved; the device tears
down BLE, shows "Connecting…", and reboots (no confirmation byte — the app
should treat the disconnect as success).

```json
{ "ssid": "...", "password": "...", "broker_ip": "...", "bin_id": "B190409", "max_count": 5 }
```

**Normal mode**: connects WiFi (15 s timeout → "WiFi Error" + 30 s sleep),
connects MQTT at `broker_ip:1883` as client `binview-<bin_id>`, subscribes to
`bins/<bin_id>`, renders the bin view, listens 10 s, then deep-sleeps 55 s. Each
received message resets the 10 s window.

```json
{ "current": 3, "max": 5, "alert": false }
```

The working display shows a QR encoding the **raw `bin_id` string only** (not
JSON) — version 3 — so warehouse scanners read the bin id directly. Alert
(`current >= max`) inverts the display (black background, white text). The first
render and every 10th use a full refresh (de-ghost); others use partial refresh.

## V1 scope

Buttons (MENU force-refresh, BACK flag-via-HTTP-POST) are **not** implemented —
deferred to v2 via OTA.
