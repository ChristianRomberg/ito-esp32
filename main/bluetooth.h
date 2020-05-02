#include <stdint.h>

#define ESP32_TX_POWER (-65)

// Source: https://covid19-static.cdn-apple.com/applications/covid19/current/static/contact-tracing/pdf/ExposureNotification-BluetoothSpecificationv1.2.pdf
#define BLE_SERVICE_UUID ((uint16_t)0xFD6F)
#define PAYLOAD_LENGTH (31)
#define MIN_BROADCAST_INTERVAL (320) // 200ms / 0.625 N/ms
#define MAX_BROADCAST_INTERVAL (432) // 270ms / 0.625 N/ms

void setupBluetooth();
void setRPI(uint8_t* rpi);
