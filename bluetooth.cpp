#include "bluetooth.h"
#include <Arduino.h>

#include <esp_bt_main.h>
#include <esp_bt.h>
#include <esp_gap_ble_api.h>

static uint8_t ble_payload[PAYLOAD_LENGTH];

static void crash() {
	ESP.restart();
	delay(5000);
}

static esp_ble_scan_params_t ble_scan_params = {
	.scan_type = BLE_SCAN_TYPE_ACTIVE,
	.own_addr_type = BLE_ADDR_TYPE_PUBLIC,
	.scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
	.scan_interval = 0x50,
	.scan_window = 0x30
//	.scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE
};

static esp_ble_adv_params_t ble_adv_params = {
	.adv_int_min = 0x20,
	.adv_int_max = 0x40,
	.adv_type = ADV_TYPE_NONCONN_IND,
	.own_addr_type = BLE_ADDR_TYPE_PUBLIC,
	.peer_addr = {},
	.peer_addr_type = BLE_ADDR_TYPE_PUBLIC,
	.channel_map = ADV_CHNL_ALL,
	.adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY
};

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
	switch (event) {
		case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
			//adv_config_done &= (~ADV_CONFIG_FLAG);
			//if (adv_config_done == 0){
			esp_ble_gap_start_advertising(&ble_adv_params);
			//}
			break;
		/*case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
			adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
			if (adv_config_done == 0){
				esp_ble_gap_start_advertising(&adv_params);
			}
			break;*/
		case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
			/* advertising start complete event to indicate advertising start successfully or failed */
			if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
				Serial.println("advertising start failed");
			}else{
				Serial.println("***** advertising start successfully *****");
			}
			break;
		case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
			if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
				Serial.println("Advertising stop failed");
			}
			else {
				Serial.println("Stop adv successfully");
			}
			break;
		/*case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
			EXAMPLE_DEBUG(EXAMPLE_TAG, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
				  param->update_conn_params.status,
				  param->update_conn_params.min_int,
				  param->update_conn_params.max_int,
				  param->update_conn_params.conn_int,
				  param->update_conn_params.latency,
				  param->update_conn_params.timeout);
			break;
		case ESP_GAP_BLE_PASSKEY_REQ_EVT:						   /* passkey request event */
		/*	EXAMPLE_DEBUG(EXAMPLE_TAG, "ESP_GAP_BLE_PASSKEY_REQ_EVT");
			//esp_ble_passkey_reply(heart_rate_profile_tab[HEART_PROFILE_APP_IDX].remote_bda, true, 0x00);
			break;
		
		case ESP_GAP_BLE_NC_REQ_EVT:
			/* The app will receive this event when the IO has DisplayYesNO capability and the peer device IO also has DisplayYesNo capability.
			show the passkey number to the user to confirm it with the number displayed by peer device. */
		/*	Serial.println("ESP_GAP_BLE_NC_REQ_EVT, the passkey Notify number:%d", param->ble_security.key_notif.passkey);
			break;
		case ESP_GAP_BLE_SEC_REQ_EVT:
			/* send the positive(true) security response to the peer device to accept the security request.
			If not accept the security request, should send the security response with negative(false) accept value*/
		/*	esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
			break;
		case ESP_GAP_BLE_PASSKEY_NOTIF_EVT:  ///the app will receive this evt when the IO has Output capability and the peer device IO has Input capability.
			///show the passkey number to the user to input it in the peer device.
			Serial.println("The passkey notify number:%d", param->ble_security.key_notif.passkey);
			break;
		case ESP_GAP_BLE_KEY_EVT:
			//shows the ble key info share with peer device to the user.
			EXAMPLE_DEBUG(EXAMPLE_TAG, "key type = %s", esp_key_type_to_str(param->ble_security.ble_key.key_type));
			break;
		case ESP_GAP_BLE_AUTH_CMPL_EVT: {
			esp_bd_addr_t bd_addr;
			memcpy(bd_addr, param->ble_security.auth_cmpl.bd_addr, sizeof(esp_bd_addr_t));
			EXAMPLE_DEBUG(EXAMPLE_TAG, "remote BD_ADDR: %08x%04x",\
					(bd_addr[0] << 24) + (bd_addr[1] << 16) + (bd_addr[2] << 8) + bd_addr[3],
					(bd_addr[4] << 8) + bd_addr[5]);
			EXAMPLE_DEBUG(EXAMPLE_TAG, "address type = %d", param->ble_security.auth_cmpl.addr_type);
			if (param->ble_security.auth_cmpl.success){
				Serial.println("(1) ***** pair status = success ***** \n");
			}
			else {
				Serial.println("***** pair status = fail, reason = 0x%x *****\n", param->ble_security.auth_cmpl.fail_reason);
			}
			show_bonded_devices();
			break;
		}
		case ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT: {
			EXAMPLE_DEBUG(EXAMPLE_TAG, "ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT status = %d", param->remove_bond_dev_cmpl.status);
			#if DEBUG_ON
			esp_log_buffer_hex(EXAMPLE_TAG, (void *)param->remove_bond_dev_cmpl.bd_addr, sizeof(esp_bd_addr_t));
			#endif 
			EXAMPLE_DEBUG(EXAMPLE_TAG, "------------------------------------");
			break;
		}*/
		default:
			break;
	}
}

static void createBlePayload() {
	// Fill BLE payload as specified here: https://covid19-static.cdn-apple.com/applications/covid19/current/static/contact-tracing/pdf/ExposureNotification-BluetoothSpecificationv1.2.pdf
	
	// Flags field
	ble_payload[0] = 0x02; // Length of field (2 bytes)
	ble_payload[1] = ESP_BLE_AD_TYPE_FLAG; // Type of field: Flags
	ble_payload[2] = 0x02 | 0x04;  // discoverable, no BR/EDR

	// Service UUID field
	ble_payload[3] = 0x03; // Length of field (3 bytes)
	ble_payload[4] = ESP_BLE_AD_TYPE_16SRV_CMPL; // Type of field: Complete List of 16-bit Service Class UUID
	ble_payload[5] = (uint8_t) (BLE_SERVICE_UUID >> 8); // First byte of service uuid
	ble_payload[6] = (uint8_t) (BLE_SERVICE_UUID >> 0); // Second byte of service uuid

	// Service Data field
	ble_payload[7] = 0x17; // Length of field (23 bytes)
	ble_payload[8] = ESP_BLE_AD_TYPE_SERVICE_DATA; // Type of field: Service Data - 16-bit UUID
	ble_payload[9] = (uint8_t) (BLE_SERVICE_UUID >> 8); // First byte of service uuid
	ble_payload[10] = (uint8_t) (BLE_SERVICE_UUID >> 0); // Second byte of service uuid

	// 16 bytes RPI (rolling proximity identifier)
	
	ble_payload[27] = 0b01000000; // Versioning: 2 bits major version, 2 bits minor version, 4 bits Reserved for future use
	ble_payload[28] = (uint8_t) ESP32_TX_POWER; // Transmit power level
	ble_payload[29] = 0; // Reserved for future use
	ble_payload[30] = 0; // Reserved for future use
}

static void createAdvParams() {
	
}

void setupBluetooth() {
	esp_err_t ret;

	ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

	    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        Serial.printf("%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        crash();
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        Serial.printf("%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        crash();
    }
    
    ret = esp_bluedroid_init();
    if (ret) {
        Serial.printf("%s init bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        crash();
    }
    ret = esp_bluedroid_enable();
    if (ret) {
        Serial.printf("%s enable bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        crash();
    }

    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret){
        Serial.printf("gap register error, error code = %x", ret);
        crash();
    }

	createBlePayload();
	createAdvParams();
}


void setRPI(uint8_t* rpi) {
	memcpy(&ble_payload[11], rpi, 16);
	esp_ble_gap_config_adv_data_raw(ble_payload, PAYLOAD_LENGTH);
	for(unsigned int i = 0; i < 31; i++) {
        Serial.printf("%x ", ble_payload[i]);
	}
	Serial.println();
}
