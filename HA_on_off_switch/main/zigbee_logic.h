// author: Jakob Scheuermeyer
// date: 25.09.2026

// This file is part of the "PC Power Control" project.
// It is used for
// - handling incoming Zigbee commands from Home Assistant

#pragma once

#include "esp_zigbee_core.h"
#include "ha/esp_zigbee_ha_standard.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HA_ONOFF_SWITCH_ENDPOINT 1

void esp_zb_task(void *pvParameters);
void update_zigbee_status(bool is_on);

#ifdef __cplusplus
}
#endif