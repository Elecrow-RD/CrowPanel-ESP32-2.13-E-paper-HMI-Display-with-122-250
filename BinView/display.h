#ifndef BINVIEW_DISPLAY_H
#define BINVIEW_DISPLAY_H

#include <Arduino.h>

// All rendering for BinView. Uses Elecrow's custom EPD driver (EPD.h /
// EPD_Init.h) exclusively. Drawing coordinate space is 250 (x) by 122 (y)
// (landscape, USE_HORIZONTIAL == 2).

// Powers the display rail (PWR_PIN) and initialises GPIO. Call once in setup()
// before any other display function.
void displayInit();

// Renders the main bin view: QR code of binId on the left, "current/max" in a
// large font top-right, binId below it. When alert is true the whole display is
// inverted (black background, white foreground). Pass fullRefresh=true for the
// initial boot frame or periodic de-ghosting; otherwise a partial refresh is
// used to save power.
void displayBinView(const String &binId, int current, int max, bool alert,
                    bool fullRefresh);

// Provisioning screen: QR code of the raw device UUID centered, with the BLE
// device name printed below. Always a full refresh.
void displayProvisioning(const String &uuid, const String &bleName);

// Single centered status line (e.g. "WiFi...", "Connecting...", "WiFi Error").
// Uses a partial refresh.
void displayStatus(const String &message);

// Draws a QR code encoding `data` with its top-left module at (x, y), each
// module rendered as a scale x scale pixel block, in the given color
// (BLACK or WHITE). `version` selects the QR version (1-40); module count is
// 17 + 4*version. Returns the rendered side length in pixels (0 on encode
// failure, e.g. data too large for the chosen version).
int displayQRCode(const String &data, int x, int y, int scale, uint8_t color,
                  uint8_t version);

#endif // BINVIEW_DISPLAY_H
