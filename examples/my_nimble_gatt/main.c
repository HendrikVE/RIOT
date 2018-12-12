/*
 * Copyright (C) 2018 Freie Universität Berlin
 *               2018 Codecoup
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       BLE peripheral example using NimBLE
 *
 * Have a more detailed look at the api here:
 * https://mynewt.apache.org/latest/tutorials/ble/bleprph/bleprph.html
 *
 * More examples (not ready to run on RIOT) can be found here:
 * https://github.com/apache/mynewt-nimble/tree/master/apps
 *
 * Test this application e.g. with Nordics "nRF Connect"-App
 * iOS: https://itunes.apple.com/us/app/nrf-connect/id1054362403
 * Android: https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Andrzej Kaczmarek <andrzej.kaczmarek@codecoup.pl>
 * @author      Hendrik van Essen <hendrik.ve@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "periph/pm.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "host/ble_gatt.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "ble_service_uuids.h"
#include "config.h"

#define CONFIG_DEVICE_ROOM "CONFIG_DEVICE_ROOM"
#define CONFIG_DEVICE_ID "CONFIG_DEVICE_ID"
#define CONFIG_OTA_HOST "CONFIG_OTA_HOST"
#define CONFIG_OTA_FILENAME "CONFIG_OTA_FILENAME"
#define CONFIG_OTA_SERVER_USERNAME "CONFIG_OTA_SERVER_USERNAME"
#define CONFIG_OTA_SERVER_PASSWORD "CONFIG_OTA_SERVER_PASSWORD"
#define CONFIG_ESP_WIFI_SSID "CONFIG_ESP_WIFI_SSID"
#define CONFIG_ESP_WIFI_PASSWORD "CONFIG_ESP_WIFI_PASSWORD"
#define CONFIG_MQTT_USER "CONFIG_MQTT_USER"
#define CONFIG_MQTT_PASSWORD "CONFIG_MQTT_PASSWORD"
#define CONFIG_MQTT_SERVER_IP "CONFIG_MQTT_SERVER_IP"
#define CONFIG_MQTT_SERVER_PORT "CONFIG_MQTT_SERVER_PORT"
#define CONFIG_SENSOR_POLL_INTERVAL_MS "CONFIG_SENSOR_POLL_INTERVAL_MS"

struct config_values config_values = {
        .device_room = CONFIG_DEVICE_ROOM,
        .device_id = CONFIG_DEVICE_ID,
        .ota_host = CONFIG_OTA_HOST,
        .ota_filename = CONFIG_OTA_FILENAME,
        .ota_server_username = CONFIG_OTA_SERVER_USERNAME,
        .ota_server_password = CONFIG_OTA_SERVER_PASSWORD,
        .wifi_ssid = CONFIG_ESP_WIFI_SSID,
        .wifi_password = CONFIG_ESP_WIFI_PASSWORD,
        .mqtt_user = CONFIG_MQTT_USER,
        .mqtt_password = CONFIG_MQTT_PASSWORD,
        .mqtt_server_ip = CONFIG_MQTT_SERVER_IP,
        .mqtt_server_port = CONFIG_MQTT_SERVER_PORT,
        .sensor_poll_interval_ms = CONFIG_SENSOR_POLL_INTERVAL_MS
};
struct config_values config_values_tmp = {};

struct config_map_key {
    char config_key[64];
};

struct config_map_value {
    uint8_t *value;
    int value_length;
};

struct config_map {
    struct config_map_key key;
    struct config_map_value value;
};

struct config_map config[] = {
        {{KEY_CONFIG_DEVICE_ROOM}, {config_values.device_room, sizeof(config_values.device_room)}},
        {{KEY_CONFIG_DEVICE_ID}, {config_values.device_id, sizeof(config_values.device_id)}},
        {{KEY_CONFIG_OTA_HOST}, {config_values.ota_host, sizeof(config_values.ota_host)}},
        {{KEY_CONFIG_OTA_FILENAME}, {config_values.ota_filename, sizeof(config_values.ota_filename)}},
        {{KEY_CONFIG_OTA_SERVER_USERNAME}, {config_values.ota_server_username, sizeof(config_values.ota_server_username)}},
        {{KEY_CONFIG_OTA_SERVER_PASSWORD}, {config_values.ota_server_password, sizeof(config_values.ota_server_password)}},
        {{KEY_CONFIG_WIFI_SSID}, {config_values.wifi_ssid, sizeof(config_values.wifi_ssid)}},
        {{KEY_CONFIG_WIFI_PASSWORD}, {config_values.wifi_password, sizeof(config_values.wifi_password)}},
        {{KEY_CONFIG_MQTT_USER}, {config_values.mqtt_user, sizeof(config_values.mqtt_user)}},
        {{KEY_CONFIG_MQTT_PASSWORD}, {config_values.mqtt_password, sizeof(config_values.mqtt_password)}},
        {{KEY_CONFIG_MQTT_SERVER_IP}, {config_values.mqtt_server_ip, sizeof(config_values.mqtt_server_ip)}},
        {{KEY_CONFIG_MQTT_SERVER_PORT}, {config_values.mqtt_server_port, sizeof(config_values.mqtt_server_port)}},
        {{KEY_CONFIG_SENSOR_POLL_INTERVAL_MS}, {config_values.sensor_poll_interval_ms, sizeof(config_values.sensor_poll_interval_ms)}},
};

static const char device_name[] = "Lord NimBLEer";
static uint8_t own_addr_type;

static int gatt_svr_chr_access_device_info(
        uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg);

static int gatt_svr_chr_access_rw_demo(
        uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg);

static void start_advertise(void);

static void put_ad(uint8_t ad_type, uint8_t ad_len, const void *ad, uint8_t *buf,
                   uint8_t *len)
{
    buf[(*len)++] = ad_len + 1;
    buf[(*len)++] = ad_type;

    memcpy(&buf[*len], ad, ad_len);

    *len += ad_len;
}

static void update_ad(void)
{
    uint8_t ad[BLE_HS_ADV_MAX_SZ];
    uint8_t ad_len = 0;
    uint8_t ad_flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    put_ad(BLE_HS_ADV_TYPE_FLAGS, 1, &ad_flags, ad, &ad_len);
    put_ad(BLE_HS_ADV_TYPE_COMP_NAME, sizeof(device_name), device_name, ad, &ad_len);

    ble_gap_adv_set_data(ad, ad_len);
}

static int gap_event_cb(struct ble_gap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            if (event->connect.status) {
                start_advertise();
            }
            break;

        case BLE_GAP_EVENT_DISCONNECT:
            start_advertise();
            break;
    }

    return 0;
}

static void start_advertise(void)
{
    struct ble_gap_adv_params advp;
    int rc;

    memset(&advp, 0, sizeof advp);
    advp.conn_mode = BLE_GAP_CONN_MODE_UND;
    advp.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                           &advp, gap_event_cb, NULL);

    puts("assert ble_gap_adv_start");
    assert(rc == 0);
    (void)rc;
}

static int gatt_svr_chr_access_device_info(
        uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    puts("service 'device info' callback triggered");

    (void) conn_handle;
    (void) attr_handle;
    (void) ctxt;
    (void) arg;

    return 0;
}

static int gatt_svr_chr_access_rw_demo(
        uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    puts("service 'rw demo' callback triggered");

    (void) conn_handle;
    (void) attr_handle;
    (void) arg;

    int rc = 0;

    const ble_uuid_t* accessed_uuid = ctxt->chr->uuid;

    int map_size1
            = sizeof(chr_cfg_mapping)
              / sizeof(chr_cfg_mapping[0]);

    int map_size2 = sizeof(config) / sizeof(config[0]);

    if (ble_uuid_cmp(accessed_uuid, (ble_uuid_t*) &gatt_svr_chr_cfg_restart_uuid.u) == 0) {
        if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
            puts("store config");
            config_store(&config_values);

            puts("restart");
            pm_reboot();
        }

        return 0;
    }

    struct ble_uuid_config_key_map *ble_uuid_config_key_map_item;
    for (int i = 0; i < map_size1; i++) {

        ble_uuid_config_key_map_item = &chr_cfg_mapping[i];

        if (ble_uuid_cmp((ble_uuid_t*) ble_uuid_config_key_map_item->key.uuid, accessed_uuid) == 0) {

            struct config_map *config_map_item;
            for (int j = 0; j < map_size2; j++) {

                config_map_item = &config[j];

                if (strcmp(ble_uuid_config_key_map_item->value.config_key, config_map_item->key.config_key) == 0) {

                    switch (ctxt->op) {

                        case BLE_GATT_ACCESS_OP_READ_CHR:

                            printf("current value for key %s: '%s'\n",
                                   config_map_item->key.config_key,
                                   (char*) (config_map_item->value.value));

                            /* send given data to the client */
                            rc = os_mbuf_append(ctxt->om, config_map_item->value.value,
                                                strlen((char*) (config_map_item->value.value)));

                            break;

                        case BLE_GATT_ACCESS_OP_WRITE_CHR:

                            printf("old value for key %s: '%s'\n",
                                   config_map_item->key.config_key,
                                   (char*) (config_map_item->value.value));

                            uint16_t om_len;
                            om_len = OS_MBUF_PKTLEN(ctxt->om);

                            /* read sent data */
                            rc = ble_hs_mbuf_to_flat(ctxt->om, config_map_item->value.value,
                                                     config_map_item->value.value_length, &om_len);
                            /* we need to null-terminate the received string */
                            config_map_item->value.value[om_len] = '\0';

                            printf("new value for key %s: '%s'\n",
                                   config_map_item->key.config_key,
                                   (char*) (config_map_item->value.value));

                            break;

                        default:
                            puts("unhandled operation!");
                            rc = 1;
                            break;
                    }
                }
            }

            return rc;
        }
    }

    puts("unhandled uuid!");
    return 1;
}

int main(void)
{
    puts("NimBLE GATT Server Example");

    config_read(&config_values_tmp);

    /*
     * if device_id is an empty string, the config flash page was probably
     * not initialized yet, because it is the first run of the application
     */
    uint8_t first_byte = config_values_tmp.device_id[0];

    /* check if first ascii is within range of allowed characters */
    if (first_byte >= 32 && first_byte <= 126) {
        puts("override defaults with the data read from flash");
        config_values = config_values_tmp;
    }
    else {
        puts("use defaults as the flash seems to be empty "
             "(first start -> config was never stored)");
    }

    int rc = 0;

    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    puts("assert ble_gatts_count_cfg");
    assert(rc == 0);

    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    puts("assert ble_gatts_add_svcs");
    assert(rc == 0);

    /* set the device name */
    ble_svc_gap_device_name_set(device_name);

    /* initialize the GAP and GATT services */
    ble_svc_gap_init();
    ble_svc_gatt_init();
    /* XXX: seems to be needed to apply the added services */
    ble_gatts_start();

    /* make sure synchronization of host and controller is done, this should
     * always be the case */
    while (!ble_hs_synced()) {}

    /* configure device address */
    rc = ble_hs_util_ensure_addr(0);
    puts("assert ble_hs_util_ensure_addr");
    assert(rc == 0);
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    puts("assert ble_hs_id_infer_auto");
    assert(rc == 0);
    (void)rc;

    /* generate the advertising data */
    update_ad();

    /* start to advertise this node */
    start_advertise();

    return 0;
}
