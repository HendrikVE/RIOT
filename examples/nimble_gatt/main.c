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

#define GATT_DEVICE_INFO_UUID                   0x180A
#define GATT_MANUFACTURER_NAME_UUID             0x2A29
#define GATT_MODEL_NUMBER_UUID                  0x2A24

/* UUID = 1bce38b3-d137-48ff-a13e-033e14c7a335 */
static const ble_uuid128_t gatt_svr_svc_rw_demo_uuid
        = BLE_UUID128_INIT(0x35, 0xa3, 0xc7, 0x14, 0x3e, 0x03, 0x3e, 0xa1, 0xff,
                0x48, 0x37, 0xd1, 0xb3, 0x38, 0xce, 0x1b);

/* UUID = 35f28386-3070-4f3b-ba38-27507e991762 */
static const ble_uuid128_t gatt_svr_chr_rw_demo_write_uuid
        = BLE_UUID128_INIT(0x62, 0x17, 0x99, 0x7e, 0x50, 0x27, 0x38, 0xba, 0x3b,
                0x4f, 0x70, 0x30, 0x86, 0x83, 0xf2, 0x35);

/* UUID = ccdd113f-40d5-4d68-86ac-a728dd82f4aa */
static const ble_uuid128_t gatt_svr_chr_rw_demo_readonly_uuid
        = BLE_UUID128_INIT(0xaa, 0xf4, 0x82, 0xdd, 0x28, 0xa7, 0xac, 0x86, 0x68,
                0x4d, 0xd5, 0x40, 0x3f, 0x11, 0xdd, 0xcc);

static const char device_name[] = "NimBLE on RIOT";
static uint8_t own_addr_type;

static char rm_demo_write_data[64] = "This characteristic is read- and writeable!";

static int gatt_svr_chr_access_device_info(
        uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg);

static int gatt_svr_chr_access_rw_demo(
        uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg);

static void start_advertise(void);

/* define several bluetooth services for our device */
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    /*
     * access_cb defines a callback for read and write access events on
     * given characteristics
     */
    {
        /* Service: Device Information */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(GATT_DEVICE_INFO_UUID),
        .characteristics = (struct ble_gatt_chr_def[]) { {
            /* Characteristic: * Manufacturer name */
            .uuid = BLE_UUID16_DECLARE(GATT_MANUFACTURER_NAME_UUID),
            .access_cb = gatt_svr_chr_access_device_info,
            .flags = BLE_GATT_CHR_F_READ,
        }, {
            /* Characteristic: Model number string */
            .uuid = BLE_UUID16_DECLARE(GATT_MODEL_NUMBER_UUID),
            .access_cb = gatt_svr_chr_access_device_info,
            .flags = BLE_GATT_CHR_F_READ,
        }, {
            0, /* No more characteristics in this service */
        }, }
    },
    {
        /* Service: Read/Write Demo */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = (ble_uuid_t*) &gatt_svr_svc_rw_demo_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]) { {
            /* Characteristic: Read/Write Demo write */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_rw_demo_write_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        }, {
            /* Characteristic: Read/Write Demo read only */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_rw_demo_readonly_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ,
        }, {
            0, /* No more characteristics in this service */
        }, }
    },
    {
        0, /* No more services */
    },
};

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

    ble_uuid_t* write_uuid = (ble_uuid_t*) &gatt_svr_chr_rw_demo_write_uuid.u;
    ble_uuid_t* readonly_uuid = (ble_uuid_t*) &gatt_svr_chr_rw_demo_readonly_uuid.u;

    if (ble_uuid_cmp(ctxt->chr->uuid, write_uuid) == 0) {

        puts("access to characteristic 'rw demo (write)'");

        switch (ctxt->op) {

            case BLE_GATT_ACCESS_OP_READ_CHR:
                puts("read from characteristic");
                printf("current value of rm_demo_write_data: '%s'\n",
                       rm_demo_write_data);

                /* send given data to the client */
                rc = os_mbuf_append(ctxt->om, &rm_demo_write_data,
                                    strlen(rm_demo_write_data));

                break;

            case BLE_GATT_ACCESS_OP_WRITE_CHR:
                puts("write to characteristic");

                printf("old value of rm_demo_write_data: '%s'\n",
                       rm_demo_write_data);

                uint16_t om_len;
                om_len = OS_MBUF_PKTLEN(ctxt->om);

                /* read sent data */
                rc = ble_hs_mbuf_to_flat(ctxt->om, &rm_demo_write_data,
                                         sizeof rm_demo_write_data, &om_len);
                /* we need to null-terminate the received string */
                rm_demo_write_data[om_len] = '\0';

                printf("new value of rm_demo_write_data: '%s'\n",
                       rm_demo_write_data);

                break;

            case BLE_GATT_ACCESS_OP_READ_DSC:
                puts("read from descriptor");
                break;

            case BLE_GATT_ACCESS_OP_WRITE_DSC:
                puts("write to descriptor");
                break;

            default:
                puts("unhandled operation!");
                rc = 1;
                break;
        }

        puts("");

        return rc;
    }
    else if (ble_uuid_cmp(ctxt->chr->uuid, readonly_uuid) == 0) {

        puts("access to characteristic 'rw demo (read-only)'");

        if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
            char random_digit;
            /* get random char between '0' and '9' */
            random_digit = 48 + (rand() % 10);

            char message[64];
            sprintf(message, "new random number: %c", random_digit);

            printf("return string '%s'\n", message);

            rc = os_mbuf_append(ctxt->om, &message, strlen(message));

            puts("");

            return rc;
        }

        return 0;
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
