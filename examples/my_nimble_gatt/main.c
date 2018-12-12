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

#include "host/ble_hs.h"
#include "host/util/util.h"
#include "host/ble_gatt.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "ble_service_uuids.h"

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
            = sizeof(characteristic_config_mapping)
              / sizeof(characteristic_config_mapping[0]);

    int map_size2
            = sizeof(config)
              / sizeof(config[0]);

    struct Map1 *map1_item;
    for (int i = 0; i < map_size1; i++) {

        map1_item = &characteristic_config_mapping[i];

        if (ble_uuid_cmp((ble_uuid_t*) map1_item->key.uuid, accessed_uuid) == 0) {

            struct Map2 *map2_item;
            for (int j = 0; j < map_size2; j++) {

                map2_item = &config[j];

                if (strcmp(map1_item->value.config_key, map2_item->key.config_key) == 0) {

                    switch (ctxt->op) {

                        case BLE_GATT_ACCESS_OP_READ_CHR:

                            printf("current value for key %s: '%s'\n",
                                   map2_item->key.config_key,
                                   map2_item->value.value);

                            /* send given data to the client */
                            rc = os_mbuf_append(ctxt->om, &map2_item->value.value,
                                                strlen(map2_item->value.value));

                            break;

                        case BLE_GATT_ACCESS_OP_WRITE_CHR:

                            printf("old value for key %s: '%s'\n",
                                   map2_item->key.config_key,
                                   map2_item->value.value);

                            uint16_t om_len;
                            om_len = OS_MBUF_PKTLEN(ctxt->om);

                            /* read sent data */
                            rc = ble_hs_mbuf_to_flat(ctxt->om, &map2_item->value.value,
                                                     sizeof map2_item->value.value, &om_len);
                            /* we need to null-terminate the received string */
                            map2_item->value.value[om_len] = '\0';

                            printf("new value for key %s: '%s'\n",
                                   map2_item->key.config_key,
                                   map2_item->value.value);

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

    int rc = 0;

    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    assert(rc == 0);

    rc = ble_gatts_add_svcs(gatt_svr_svcs);
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
    assert(rc == 0);
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    assert(rc == 0);
    (void)rc;

    /* generate the advertising data */
    update_ad();

    /* start to advertise this node */
    start_advertise();

    return 0;
}
