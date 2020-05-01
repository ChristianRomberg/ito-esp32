#include <Arduino.h>

#define BLE_SERVICE_UUID ((uint16_t)0xFD6F)
#define PAYLOAD_LENGTH (31)
#define ESP32_TX_POWER (-65)



void setupBluetooth();
void setRPI(uint8_t* rpi);
