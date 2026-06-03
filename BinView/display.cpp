#include "display.h"
#include "config.h"
#include "EPD.h"
#include "qrcode_rm.h" // Richard Moore's QRCode, vendored to avoid a name clash
                      // with the ESP32 core's bundled espressif__qrcode/qrcode.h
#include <string.h>

// The driver's global frame buffer, defined in EPD.cpp.
extern uint8_t ImageBW[ALLSCREEN_BYTES];

// Drawing coordinate space (USE_HORIZONTIAL == 2): x in [0, EPD_W), y in
// [0, EPD_H). EPD_W = 250, EPD_H = 122.

// -----------------------------------------------------------------------------
// Frame buffer / refresh helpers
// -----------------------------------------------------------------------------

// Clears the frame buffer to a solid background. White background -> 0x00 bytes
// (the driver transmits ~ImageBW, and white == bit 1). Black background (alert)
// -> 0xFF bytes.
static void clearBuffer(bool blackBackground) {
  memset(ImageBW, blackBackground ? 0xFF : 0x00, ALLSCREEN_BYTES);
}

// Pushes the composed frame buffer to the panel.
//
// Full refresh performs a white flash (EPD_ALL_Fill + EPD_Update) which clears
// e-ink ghosting, then writes the content with the full LUT. Partial refresh
// skips the flash and uses the fast partial LUT -- lower power and no visible
// flash, used for count updates. Both re-init the controller because the device
// deep-sleeps (and the chip resets) between updates.
static void pushFrame(bool fullRefresh) {
  if (fullRefresh) {
    EPD_Init();
    EPD_ALL_Fill(WHITE);
    EPD_Update();
    EPD_Clear_R26H();
    EPD_DisplayImage(ImageBW);
    EPD_Update();
    EPD_Sleep();
  } else {
    EPD_Init();
    EPD_Clear_R26H();
    EPD_DisplayImage(ImageBW);
    EPD_PartUpdate();
    EPD_Sleep();
  }
}

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------

void displayInit() {
  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, HIGH); // power the display rail
}

int displayQRCode(const String &data, int x, int y, int scale, uint8_t color,
                  uint8_t version) {
  // Module count = 17 + 4*version (e.g. v3 -> 29, v4 -> 33). Error correction
  // LOW to maximize data capacity.
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(version)];
  if (qrcode_initText(&qrcode, qrcodeData, version, ECC_LOW, data.c_str()) < 0) {
    // Data too large for this version, or other encode error.
    return 0;
  }

  for (uint8_t my = 0; my < qrcode.size; my++) {
    for (uint8_t mx = 0; mx < qrcode.size; mx++) {
      if (!qrcode_getModule(&qrcode, mx, my)) {
        continue; // light module -> leave as background
      }
      int px0 = x + mx * scale;
      int py0 = y + my * scale;
      for (int dy = 0; dy < scale; dy++) {
        for (int dx = 0; dx < scale; dx++) {
          int px = px0 + dx;
          int py = py0 + dy;
          if (px >= 0 && px < EPD_W && py >= 0 && py < EPD_H) {
            EPD_DrawPoint(px, py, color);
          }
        }
      }
    }
  }
  return qrcode.size * scale;
}

void displayBinView(const String &binId, int current, int max, bool alert,
                    bool fullRefresh) {
  const uint8_t fg = alert ? WHITE : BLACK;
  clearBuffer(alert);

  // --- Left: QR code of the RAW bin id (not JSON), vertically centered ---
  // The provisioned/working display encodes only the bin id string so warehouse
  // scanners read the bin id directly.
  const int qrVersion = 3;                     // 29x29 modules; ample for a bin id
  const int qrScale = 3;                        // 29 * 3 = 87 px
  const int qrSide = (17 + 4 * qrVersion) * qrScale; // 87
  const int qrX = 8;
  const int qrY = (EPD_H - qrSide) / 2;         // ~17
  displayQRCode(binId, qrX, qrY, qrScale, fg, qrVersion);

  // --- Right top: "current/max" in the large 48px font ---
  char countBuf[16];
  snprintf(countBuf, sizeof(countBuf), "%d/%d", current, max);
  const int textX = 120;
  EPD_ShowString(textX, 18, countBuf, fg, 48);

  // --- Right bottom: bin id in the 24px font ---
  EPD_ShowString(textX, 84, binId.c_str(), fg, 24);

  pushFrame(fullRefresh);
}

void displayProvisioning(const String &uuid, const String &bleName) {
  clearBuffer(false);

  // Provisioning QR encodes JSON so the Android app can read both the device
  // UUID and the exact BLE advertised name in one scan:
  //   {"uuid":"<32 hex>","ble":"BinView-XXXXXXXX"}  (~68 bytes)
  // Version 4 (33x33, ECC LOW) holds up to 78 bytes; version 3 (53 bytes) is
  // too small for this payload.
  String payload =
      String("{\"uuid\":\"") + uuid + "\",\"ble\":\"" + bleName + "\"}";

  const int qrVersion = 4;
  const int qrScale = 3;                              // 33 * 3 = 99 px
  const int qrSide = (17 + 4 * qrVersion) * qrScale;  // 99
  const int qrX = (EPD_W - qrSide) / 2;               // centered, ~75
  const int qrY = 2;
  displayQRCode(payload, qrX, qrY, qrScale, BLACK, qrVersion);

  // BLE device name centered below the QR (font size 16, char width 8).
  const int charW = 8;
  int nameX = (EPD_W - (int)bleName.length() * charW) / 2;
  if (nameX < 0) nameX = 0;
  EPD_ShowString(nameX, qrY + qrSide + 4, bleName.c_str(), BLACK, 16);

  pushFrame(true);
}

void displayStatus(const String &message) {
  clearBuffer(false);

  // Center a single line (font size 16, char width 8) on the screen.
  const int charW = 8;
  int x = (EPD_W - (int)message.length() * charW) / 2;
  if (x < 0) x = 0;
  int y = (EPD_H - 16) / 2;
  EPD_ShowString(x, y, message.c_str(), BLACK, 16);

  pushFrame(false);
}
