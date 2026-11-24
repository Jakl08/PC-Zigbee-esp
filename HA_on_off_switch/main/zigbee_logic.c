// author: Jakob Scheuermeyer
// date: 25.09.2026

// This file is part of the "PC Power Control" project.
// It is used for
// - handling incoming Zigbee commands from Home Assistant

#include "zigbee_logic.h"
#include "zcl_utility.h"
#include "hardware_driver.h"
#include "esp_log.h"

static const char *TAG = "ZIGBEE_LOGIC";

// Konfiguration als Zigbee Router (ZR)
#define ESP_ZB_ZR_CONFIG()                                          \
    {                                                               \
        .esp_zb_role = ESP_ZB_DEVICE_TYPE_ROUTER,                   \
        .install_code_policy = false,                               \
        .nwk_cfg.zczr_cfg = {                                       \
            .max_children = 10,                                     \
        },                                                          \
    }

// Kompatibler Action-Handler für IDF v5.1+
static esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message) {
    esp_err_t ret = ESP_OK;
    
    if (callback_id == ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID) {
        const esp_zb_zcl_set_attr_value_message_t *attr_msg = (const esp_zb_zcl_set_attr_value_message_t *)message;
        
        if (attr_msg->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_ON_OFF) {
            bool request_on = *(bool *)attr_msg->attribute.data.value;
            bool current_state = get_pc_status_phy();
            ESP_LOGI(TAG, "On/Off-Befehl: Ziel=%s, PC=%s",
                     request_on ? "AN" : "AUS", current_state ? "AN" : "AUS");

            if (request_on != current_state) {
                pulse_power_button(request_on);
            } else {
                ESP_LOGI(TAG, "Relais bleibt aus: PC ist bereits im Zielzustand");
            }
        }
    }
    return ret;
}

static void update_zigbee_status_in_stack(bool is_on) {
    esp_zb_zcl_status_t state = esp_zb_zcl_set_attribute_val(
        HA_ONOFF_SWITCH_ENDPOINT, 
        ESP_ZB_ZCL_CLUSTER_ID_ON_OFF, 
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, 
        ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID, 
        &is_on, 
        false
    );

    if (state != ESP_ZB_ZCL_STATUS_SUCCESS) {
        ESP_LOGW(TAG, "On/Off-Attribut konnte nicht aktualisiert werden: %d", state);
        return;
    }

    esp_zb_zcl_report_attr_cmd_t report_cmd = {
        .zcl_basic_cmd.dst_addr_u.addr_short = 0x0000,
        .zcl_basic_cmd.dst_endpoint = 1,
        .zcl_basic_cmd.src_endpoint = HA_ONOFF_SWITCH_ENDPOINT,
        .address_mode = ESP_ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
        .clusterID = ESP_ZB_ZCL_CLUSTER_ID_ON_OFF,
        .direction = 1,
        .attributeID = ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID,
    };
    esp_err_t report_status = esp_zb_zcl_report_attr_cmd_req(&report_cmd);
    if (report_status == ESP_OK) {
        ESP_LOGI(TAG, "Hardware-Status (%s) an Zigbee2MQTT gemeldet", is_on ? "AN" : "AUS");
    } else {
        ESP_LOGW(TAG, "On/Off-Statusreport fehlgeschlagen: %s", esp_err_to_name(report_status));
    }
}

void update_zigbee_status(bool is_on) {
    if (!esp_zb_is_started() || !esp_zb_lock_acquire(portMAX_DELAY)) {
        return;
    }

    update_zigbee_status_in_stack(is_on);
    esp_zb_lock_release();
}

void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct) {
    uint32_t *p_sg_p = signal_struct->p_app_signal;
    esp_err_t err_status = signal_struct->esp_err_status;
    esp_zb_app_signal_type_t sig_type = *p_sg_p;

    switch (sig_type) {
        case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:
            esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_INITIALIZATION);
            break;
            
        case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
        case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
            if (err_status == ESP_OK) {
                ESP_LOGI(TAG, "Geraet initialisiert. Suche nach Zigbee2MQTT (Steering)...");
                // Diesen Befehl haben wir vermisst: Er startet den echten Anlern-Prozess!
                esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
            }
            break;
            
        case ESP_ZB_BDB_SIGNAL_STEERING:
            if (err_status == ESP_OK) {
                ESP_LOGI(TAG, "Erfolgreich mit Zigbee2MQTT gepairt!");
                update_zigbee_status_in_stack(get_pc_status_phy());
            } else {
                ESP_LOGW(TAG, "Netzwerk nicht gefunden. Versuche es in 3 Sekunden erneut...");
                vTaskDelay(pdMS_TO_TICKS(3000));
                esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
            }
            break;
            
        default:
            break;
    }
}

void esp_zb_task(void *pvParameters) {
    esp_zb_cfg_t zb_nwk_cfg = ESP_ZB_ZR_CONFIG(); 
    esp_zb_init(&zb_nwk_cfg);

    esp_zb_on_off_light_cfg_t light_cfg = ESP_ZB_DEFAULT_ON_OFF_LIGHT_CONFIG();
    esp_zb_ep_list_t *ep_list = esp_zb_on_off_light_ep_create(HA_ONOFF_SWITCH_ENDPOINT, &light_cfg);
    
    zcl_basic_manufacturer_info_t info = {
        .manufacturer_name = "\x0D""CUSTOM_DEVICE",
        .model_identifier = "\x0B""PC_ESP32_C6",
    };
    esp_zcl_utility_add_ep_basic_manufacturer_info(ep_list, HA_ONOFF_SWITCH_ENDPOINT, &info);
    
    esp_zb_device_register(ep_list);

    esp_zb_core_action_handler_register(zb_action_handler);

    esp_zb_set_primary_network_channel_set(ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK);
    esp_zb_start(false);
    
    esp_zb_stack_main_loop(); 
}