#include "bluetooth.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_sleep.h"

#include <esp_nimble_hci.h>
#include <nimble/nimble_port.h>
#include <nimble/nimble_port_freertos.h>
#include <host/ble_hs.h>

#include <freertos/event_groups.h>

#define EVENT_BIT_SYNC (0x01)
#define EVENT_BIT_ADV (0x02)

static uint8_t ble_payload[PAYLOAD_LENGTH];
static EventGroupHandle_t ble_eventgroup = NULL;

static void crash() {
	esp_restart();
}

static void bleprph_advertise(void) {
	struct ble_gap_adv_params adv_params;
	int rc;

	rc = ble_gap_adv_set_data(ble_payload, PAYLOAD_LENGTH);
	if (rc != 0) {
		printf("error setting advertisement data; rc=%d\n", rc);
		crash();
	}

	/* Begin advertising. */
	memset(&adv_params, 0, sizeof adv_params);
	adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
	adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
	rc = ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER,
			&adv_params, NULL, NULL);
	if (rc != 0) {
		printf("error enabling advertisement; rc=%d\n", rc);
		crash();
	}
}

static void on_reset(int reason) {
	printf("Resetting state; reason=%d\n", reason);
}

static void on_sync() {
	xEventGroupSetBits(ble_eventgroup, EVENT_BIT_SYNC);
}

static void createBlePayload() {
	// Fill BLE payload as specified here: https://covid19-static.cdn-apple.com/applications/covid19/current/static/contact-tracing/pdf/ExposureNotification-BluetoothSpecificationv1.2.pdf

	// Flags field
	ble_payload[0] = 0x02; // Length of field (2 bytes)
	ble_payload[1] = 0x01; // Type of field: Flags
	ble_payload[2] = 0x02 | 0x04;  // discoverable, no BR/EDR

	// Service UUID field
	ble_payload[3] = 0x03; // Length of field (3 bytes)
	ble_payload[4] = 0x03; // Type of field: Complete List of 16-bit Service Class UUID
	ble_payload[5] = (uint8_t) (BLE_SERVICE_UUID >> 8); // First byte of service uuid
	ble_payload[6] = (uint8_t) (BLE_SERVICE_UUID >> 0); // Second byte of service uuid

	// Service Data field
	ble_payload[7] = 0x17; // Length of field (23 bytes)
	ble_payload[8] = 0x16; // Type of field: Service Data - 16-bit UUID
	ble_payload[9] = (uint8_t) (BLE_SERVICE_UUID >> 8); // First byte of service uuid
	ble_payload[10] = (uint8_t) (BLE_SERVICE_UUID >> 0); // Second byte of service uuid

	// 16 bytes RPI (rolling proximity identifier)

	ble_payload[27] = 0b01000000; // Versioning: 2 bits major version, 2 bits minor version, 4 bits Reserved for future use
	ble_payload[28] = (uint8_t) ESP32_TX_POWER; // Transmit power level
	ble_payload[29] = 0; // Reserved for future use
	ble_payload[30] = 0; // Reserved for future use
}

void bluetooth_host_task(void *param) {
	printf("BLE Host Task Started\n");
	/* This function will return only when nimble_port_stop() is executed */
	nimble_port_run();

	nimble_port_freertos_deinit();
}

void setupBluetooth() {
	ESP_ERROR_CHECK(esp_nimble_hci_and_controller_init());

	nimble_port_init();

	/* Initialize the NimBLE host configuration. */
	ble_hs_cfg.reset_cb = on_reset;
	ble_hs_cfg.sync_cb = on_sync;
	ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

	ble_eventgroup = xEventGroupCreate();

	nimble_port_freertos_init(bluetooth_host_task);

	createBlePayload();

	// wait for nimBLE to sync
	if(!(xEventGroupWaitBits(ble_eventgroup,
			EVENT_BIT_SYNC,
			pdFALSE,                   // don't clear the sync bit
			pdFALSE,                   // only relevant if multiple bits are waited for
			1000 / portTICK_PERIOD_MS) // timeout after 1000ms
			& EVENT_BIT_SYNC)) {
		printf("Waiting for nimBLE sync timed out!\n");
		crash();
	}
}

void setRPI(uint8_t *rpi) {
	memcpy(&ble_payload[11], rpi, 16);

	for (unsigned int i = 0; i < 31; i++) {
		printf("%x ", ble_payload[i]);
	}
	printf("\n");

	bleprph_advertise();
}
