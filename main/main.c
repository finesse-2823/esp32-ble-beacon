#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_bt_main.h"

#define TAG "BLE_BEACON"

static uint8_t adv_data[30] = {
    0x02, 0x01, 0x06,                                                      // flags
    0x03, 0x03, 0x6F, 0xFD,                                                // 16-bit UUID (0xFD6F)
    0x0D, 0x09, 'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!' // message
};

void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        esp_ble_gap_start_advertising(&adv_data);
        break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(TAG, "Advertising start failed: %s", esp_err_to_name(param->adv_start_cmpl.status));
        }
        break;
    default:
        break;
    }
}

void ble_beacon_task(void *arg)
{
    // initialize Bluetooth controller and stack
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bt_controller_init(&bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_BLE);

    // configure advertising parameters
    esp_ble_adv_data_t adv_data = {
        .set_scan_rsp = false,
        .include_name = false,
        .include_txpower = false,
        .min_interval = 0x20,
        .max_interval = 0x40,
        .appearance = 0x00,
        .manufacturer_len = 0,
        .p_manufacturer_data = NULL,
        .service_data_len = 0,
        .p_service_data = NULL,
        .service_uuid_len = 0,
        .p_service_uuid = NULL,
        .flag = 0x04,
        .include_len = 13,
        .p_include = (uint8_t *)"\x02\x01\x06\x03\x03\xF0\xFF\x12\x18\xF5\xFE\x6D\x00"};

    // start advertising
    esp_ble_gap_start_advertising(&adv_data);

    // register GAP callback function
    esp_ble_gap_register_callback(gap_event_handler);

    while (1)
    {
        // wait for 10 seconds
        vTaskDelay(10000 / portTICK_PERIOD_MS);

        // update advertisement data with new message
        uint8_t new_adv_data[24] = {
            0x02, 0x01, 0x06, 0x03, 0x03, 0xF0, 0xFF, 0x12, 0x18, 0xF5, 0xFE, 0x6D, 0x00,
            'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!'};

        // set new advertisement data
        esp_ble_gap_config_adv_data_raw(new_adv_data, sizeof(new_adv_data));
    }
}

void app_main()
{
    xTaskCreate(ble_beacon_task, "ble_beacon_task", 4096, NULL, 5, NULL);
}
