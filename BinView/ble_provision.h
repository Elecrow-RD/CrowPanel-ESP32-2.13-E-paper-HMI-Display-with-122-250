#ifndef BINVIEW_BLE_PROVISION_H
#define BINVIEW_BLE_PROVISION_H

#include <Arduino.h>
#include <functional>

// Called once a complete JSON provisioning payload has been received over BLE.
// The argument is the full JSON string reassembled from BLE write chunks.
typedef std::function<void(const String &json)> ProvisionCallback;

// Starts a NimBLE GATT server advertising BLE_SERVICE_UUID with a single
// writable characteristic (BLE_CHARACTERISTIC_UUID). Incoming writes are
// buffered (the default 20-byte MTU splits the JSON into chunks) until a
// complete JSON object is seen, at which point `cb` is invoked.
void startBLEProvisioning(const String &deviceName, ProvisionCallback cb);

// Tears down the BLE stack.
void stopBLE();

#endif // BINVIEW_BLE_PROVISION_H
