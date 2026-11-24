// author: Jakob Scheuermeyer
// date: 25.09.2026

// This file is part of the "PC Power Control" project.
// It is used for
// - initializing the GPIO pins for the relay and status sensor
// - simulating the physical power button press

#pragma once
#include <stdbool.h>
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RELAY_PIN GPIO_NUM_18
#define STATUS_PIN GPIO_NUM_17

// Callback-Definition für Statusänderungen
typedef void (*pc_status_callback_t)(bool is_on);

// Initialisierung erwartet nun den Callback
void hardware_init(pc_status_callback_t cb);
void pulse_power_button(bool turning_on);
bool get_pc_status_phy(void);

#ifdef __cplusplus
}
#endif