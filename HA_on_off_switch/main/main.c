// author: Jakob Scheuermeyer
// date: 25.09.2026

// This is the main file part of the "PC Power Control" project.


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "hardware_driver.h"
#include "zigbee_logic.h"

void app_main(void) {
    // 1. Hardware sofort absichern (O(1) Laufzeit)[cite: 13]
    hardware_init(update_zigbee_status);

    // 2. Flash-Speicher initialisieren (Notwendig für Zigbee-Keys)[cite: 5]
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // 3. Zigbee-Logik in separatem Thread starten[cite: 5]
    xTaskCreate(esp_zb_task, "Zigbee_main_task", 10240, NULL, 5, NULL);
}