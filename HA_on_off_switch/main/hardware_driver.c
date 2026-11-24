// author: Jakob Scheuermeyer
// date: 25.09.2026

// This file is part of the "PC Power Control" project.
// It is used for
// - initializing the GPIO pins for the relay and status sensor
// - simulating the physical power button press

#include "hardware_driver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

#define RELAY_VERIFY_EVENT UINT32_MAX

static pc_status_callback_t status_cb = NULL;
static QueueHandle_t gpio_evt_queue = NULL;
static TimerHandle_t relay_timer = NULL;
static TimerHandle_t relay_verify_timer = NULL;

static void schedule_relay_status_verification(void) {
    if (relay_verify_timer != NULL) {
        xTimerChangePeriod(relay_verify_timer, pdMS_TO_TICKS(5000), 0);
    }
}

static void relay_verify_callback(TimerHandle_t timer) {
    (void)timer;
    uint32_t event = RELAY_VERIFY_EVENT;
    if (gpio_evt_queue != NULL) {
        xQueueSend(gpio_evt_queue, &event, 0);
    }
}

static void relay_release_callback(TimerHandle_t timer) {
    (void)timer;
    gpio_set_level(RELAY_PIN, 0);
    schedule_relay_status_verification();
}

// ISR: Wird in Hardware-Echtzeit ausgeführt, wenn sich GPIO Status ändert
static void IRAM_ATTR gpio_isr_handler(void *arg) {
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

// Background-Task: Entprellt das Signal und feuert den Callback
static void status_monitor_task(void *arg) {
    uint32_t io_num;
    bool last_state = get_pc_status_phy();

    for (;;) {
        // Wartet blockierend auf ein Event aus der ISR (kein Polling-Overhead)
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            if (io_num == RELAY_VERIFY_EVENT) {
                bool current_state = get_pc_status_phy();
                last_state = current_state;
                if (status_cb != NULL) {
                    status_cb(current_state);
                }
                continue;
            }

            vTaskDelay(pdMS_TO_TICKS(50)); // 50ms Debounce
            
            bool current_state = get_pc_status_phy();
            if (current_state != last_state) {
                last_state = current_state;
                if (status_cb != NULL) {
                    status_cb(current_state); // Informiert die Zigbee-Logik
                }
            }
        }
    }
}

void hardware_init(pc_status_callback_t cb) {
    status_cb = cb;

    // Relais sichern
    gpio_set_level(RELAY_PIN, 0);
    gpio_config_t relay_conf = {
        .pin_bit_mask = (1ULL << RELAY_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&relay_conf);
    relay_timer = xTimerCreate("relay_release", pdMS_TO_TICKS(500), pdFALSE, NULL, relay_release_callback);
    relay_verify_timer = xTimerCreate("relay_verify", pdMS_TO_TICKS(5000), pdFALSE, NULL, relay_verify_callback);

    // Status-Sensor für Interrupts vorbereiten
    gpio_config_t status_conf = {
        .pin_bit_mask = (1ULL << STATUS_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_ANYEDGE // Triggert bei HIGH -> LOW und LOW -> HIGH
    };
    gpio_config(&status_conf);

    // Queue und Task für ISR-Verarbeitung erstellen
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(status_monitor_task, "status_monitor", 2048, NULL, 10, NULL);

    // ISR an den Pin binden
    gpio_install_isr_service(0);
    gpio_isr_handler_add(STATUS_PIN, gpio_isr_handler, (void *)STATUS_PIN);
}

void pulse_power_button(bool turning_on) {
    TickType_t pulse_duration = pdMS_TO_TICKS(turning_on ? 500 : 5000);
    gpio_set_level(RELAY_PIN, 1);
    if (relay_timer != NULL) {
        if (xTimerChangePeriod(relay_timer, pulse_duration, 0) != pdPASS) {
            gpio_set_level(RELAY_PIN, 0);
        }
    } else {
        vTaskDelay(pulse_duration);
        gpio_set_level(RELAY_PIN, 0);
        schedule_relay_status_verification();
    }
}

bool get_pc_status_phy(void) {
    return gpio_get_level(STATUS_PIN) == 1;
}