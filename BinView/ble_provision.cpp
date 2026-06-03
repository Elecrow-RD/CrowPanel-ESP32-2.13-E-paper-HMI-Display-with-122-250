#include "ble_provision.h"
#include "config.h"
#include <NimBLEDevice.h>

// Targets the NimBLE-Arduino 2.x API (callbacks take a NimBLEConnInfo&).

static ProvisionCallback g_callback = nullptr;
static String g_rxBuffer;     // accumulates chunked writes
static int g_braceDepth = 0;  // tracks JSON object nesting across chunks
static bool g_sawOpen = false;

static void resetBuffer() {
  g_rxBuffer = "";
  g_braceDepth = 0;
  g_sawOpen = false;
}

// Feeds received bytes into the reassembly buffer and fires the callback once a
// complete, brace-balanced JSON object has arrived.
static void feedChunk(const std::string &chunk) {
  for (char c : chunk) {
    g_rxBuffer += c;
    if (c == '{') {
      g_braceDepth++;
      g_sawOpen = true;
    } else if (c == '}') {
      if (g_braceDepth > 0) g_braceDepth--;
      if (g_sawOpen && g_braceDepth == 0) {
        // Complete object received.
        String payload = g_rxBuffer;
        resetBuffer();
        if (g_callback) g_callback(payload);
        return;
      }
    }
  }

  // Guard against runaway buffers from malformed input.
  if (g_rxBuffer.length() > 512) {
    resetBuffer();
  }
}

class ProvisionCharacteristicCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *pCharacteristic,
               NimBLEConnInfo &connInfo) override {
    NimBLEAttValue v = pCharacteristic->getValue();
    feedChunk(std::string(v.c_str(), v.length()));
  }
};

class ProvisionServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override {
    Serial.println("[BLE] client connected");
    resetBuffer();
  }
  void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo,
                    int reason) override {
    Serial.println("[BLE] client disconnected, re-advertising");
    resetBuffer();
    NimBLEDevice::startAdvertising();
  }
};

void startBLEProvisioning(const String &deviceName, ProvisionCallback cb) {
  g_callback = cb;
  resetBuffer();

  NimBLEDevice::init(deviceName.c_str());

  NimBLEServer *pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ProvisionServerCallbacks());

  NimBLEService *pService = pServer->createService(BLE_SERVICE_UUID);
  NimBLECharacteristic *pCharacteristic = pService->createCharacteristic(
      BLE_CHARACTERISTIC_UUID,
      NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
  pCharacteristic->setCallbacks(new ProvisionCharacteristicCallbacks());
  pService->start();

  // Advertisement packet carries flags + the 128-bit Service UUID so the
  // Android app can filter by service. A 128-bit UUID and the full device name
  // do not both fit in one 31-byte advertisement, so the name is broadcast in
  // the scan response instead.
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();

  NimBLEAdvertisementData advData;
  advData.setFlags(BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP);
  advData.addServiceUUID(BLE_SERVICE_UUID);
  pAdvertising->setAdvertisementData(advData);

  NimBLEAdvertisementData scanData;
  scanData.setName(std::string(deviceName.c_str()));
  pAdvertising->setScanResponseData(scanData);

  pAdvertising->enableScanResponse(true);
  pAdvertising->start();

  Serial.print("[BLE] advertising as ");
  Serial.println(deviceName);
}

void stopBLE() {
  NimBLEDevice::deinit(true);
}
