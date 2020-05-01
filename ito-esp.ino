//#include <WiFiManager.h>

#include <time.h>
#include <SPIFFS.h>

#include "bluetooth.h"

#define CONFIG_AP_NAME ("ito configuration")
#define NTP_SERVER ("pool.ntp.org")

static void crash() {
	ESP.restart();
	delay(5000);
}

void setup() {
	Serial.begin(115200);

	/*WiFiManager wifiManager;
	wifiManager.setTimeout(20);

	if(!wifiManager.autoConnect(CONFIG_AP_NAME)) {
		Serial.println("failed to connect and hit timeout");
		crash();
	}*/
	
	//configTime(0, 0, NTP_SERVER);

	if(!SPIFFS.begin(true)) {
		Serial.println("An Error has occurred while mounting SPIFFS");
		crash();
	}

	setupBluetooth();
}

long getTimestampSeconds() {
	struct tm timeinfo;
	if(!getLocalTime(&timeinfo)) {
		Serial.println("Failed to obtain time");
		crash();
	}
	return (long)mktime(&timeinfo);
}

void loop() {
	uint8_t rpi[16];
	for(unsigned int i = 0; i < 16; i++)
	{
		rpi[i] = (uint8_t)random(0, 255);
	}
	setRPI(rpi);
	//Serial.println(getTimestampSeconds());
	delay(5000);
}
