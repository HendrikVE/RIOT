/*
 * Copyright (C) 2022 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ztimer.h"
#include "utlist.h"
#include "thread.h"
#include "mfrc522.h"
#include "mfrc522_params.h"
#include "pca9633.h"
#include "pca9633_params.h"
#include "aip31068.h"
#include "aip31068_params.h"

#define ENABLE_DEBUG    1
#include "debug.h"

#define ID_LEN 4

static mfrc522_t _mfrc522_dev;
static aip31068_t _aip31068_dev;
static pca9633_t _pca9633_dev;

static uint8_t _master_id[ID_LEN];

typedef struct id_item {
    uint8_t id[ID_LEN];
    struct id_item *prev, *next;
} id_item;

static id_item *_stored_ids = NULL;

static kernel_pid_t _standby_thread_pid;
static char _standby_thread_stack[THREAD_STACKSIZE_SMALL];
static volatile bool _stop_standby_thread = false;

static int _my_cmp(id_item *a, id_item *b)
{
    return memcmp(a->id, b->id, ID_LEN);
}

static int _get_uid(uint8_t *id, uint8_t buf_size)
{
    int rc = 0;
    mfrc522_uid_t uid;

    /* Getting ready for Reading PICCs
     * If a new PICC placed to RFID reader continue
     */
    if (!(rc = mfrc522_picc_is_new_card_present(&_mfrc522_dev))) {
//        DEBUG("No card detected ...\n");
        return 0;
    }

    DEBUG("Card detected. Read card serial ...\n");

    /* Since a PICC placed get Serial and continue */
    rc = mfrc522_picc_read_card_serial(&_mfrc522_dev, &uid);
    if (rc != 0) {
        return 0;
    }

    DEBUG("Scanned PICC's UID: ");
    assert(buf_size >= uid.size);
    for (uint8_t i = 0; i < buf_size; i++) {
        id[i] = uid.uid_byte[i];
        DEBUG("%x", id[i]);
    }
    DEBUG("\n");

    /* Stop reading */
    mfrc522_picc_halt_a(&_mfrc522_dev);

    return 1;
}

static int _scan_uid(uint8_t *id, uint8_t buf_size)
{
    while (!_get_uid(id, buf_size)) {
        ztimer_sleep(ZTIMER_MSEC, 500);
    }

    return 0;
}

static void _enter_master_mode(void)
{
    uint8_t tmp_id[ID_LEN];

    aip31068_clear(&_aip31068_dev);
    aip31068_print(&_aip31068_dev, "  MASTER MODE");

    pca9633_set_rgb(&_pca9633_dev, 255, 255, 0);

    while (1) {
        /* add new or remove known keys */
        _scan_uid(tmp_id, sizeof(tmp_id));

        id_item *search_item = malloc(sizeof(id_item));
        memcpy(search_item->id, tmp_id, ID_LEN);

        id_item *found_item;

        if (memcmp(_master_id, tmp_id, ID_LEN) == 0) {
            DEBUG("Leave master mode\n");
            /* back to continuous scanning */
            break;
        }

        DL_SEARCH(_stored_ids, found_item, search_item, _my_cmp);
        if (found_item != NULL) {
            DEBUG("Remove known key\n");

            DL_DELETE(_stored_ids, found_item);
            free(found_item);
            free(search_item);

            aip31068_clear(&_aip31068_dev);
            aip31068_print(&_aip31068_dev, "  Key removed");
        }
        else {
            DEBUG("Add new key\n");

            DL_APPEND(_stored_ids, search_item);

            aip31068_clear(&_aip31068_dev);
            aip31068_print(&_aip31068_dev, "   Key added");
        }

        ztimer_sleep(ZTIMER_MSEC, 1500);
        aip31068_clear(&_aip31068_dev);
        aip31068_print(&_aip31068_dev, "  MASTER MODE");
    }

    aip31068_clear(&_aip31068_dev);
    pca9633_set_rgb(&_pca9633_dev, 255, 255, 255);
}

void *standby_thread(void *arg)
{
    (void) arg;

    int start_col = 3;

    aip31068_set_cursor_position(&_aip31068_dev, 0, start_col);
    aip31068_print(&_aip31068_dev, "ON STANDBY");

    bool scroll_toggle = true;
    int i = start_col;
    while (true) {
        ztimer_sleep(ZTIMER_MSEC, 500);

        if (scroll_toggle) {
            aip31068_scroll_display_right(&_aip31068_dev);
            i++;
        }
        else {
            aip31068_scroll_display_left(&_aip31068_dev);
            i--;
        }

        if (i < 1 || i > 5) {
            scroll_toggle = !scroll_toggle;
        }

        if (_stop_standby_thread) {
            _stop_standby_thread = false;
            return NULL;
        }
    }
}

static void _enter_standby_mode(void)
{
    _standby_thread_pid = thread_create(_standby_thread_stack, sizeof(_standby_thread_stack),
                                        THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
                                        standby_thread, NULL, "standby_thread");
}

static void _leave_standby_mode(void)
{
    _stop_standby_thread = true;

    while (thread_getstatus(_standby_thread_pid) != STATUS_NOT_FOUND) {
        ztimer_sleep(ZTIMER_MSEC, 100);
    }
}

int main(void)
{
    int rc = 0;

    if ((rc = mfrc522_pcd_init(&_mfrc522_dev, &mfrc522_params[0])) != 0) {
        DEBUG("Initialization of mfrc522 failed! rc = %d\n", rc);
        return 1;
    }

    if ((rc = aip31068_init(&_aip31068_dev, &aip31068_params[0])) != 0) {
        DEBUG("Initialization of aip31068 failed! rc = %d\n", rc);
        return 1;
    }

    if (pca9633_init(&_pca9633_dev, &pca9633_params[0]) != PCA9633_OK) {
        DEBUG("Initialization of pca9633 failed!\n");
        return 1;
    }

    aip31068_turn_on(&_aip31068_dev);
    pca9633_turn_on(&_pca9633_dev);
    pca9633_set_ldr_state_all(&_pca9633_dev, PCA9633_LDR_STATE_IND_GRP);
    pca9633_set_group_control_mode(&_pca9633_dev, PCA9633_GROUP_CONTROL_MODE_DIMMING);

    aip31068_clear(&_aip31068_dev);
    aip31068_print(&_aip31068_dev, "   Powered by");
    aip31068_set_cursor_position(&_aip31068_dev, 1, 0);
    aip31068_print(&_aip31068_dev, "    RIOT OS");

    pca9633_set_rgb(&_pca9633_dev, 255, 0, 0);
    ztimer_sleep(ZTIMER_MSEC, 750);
    pca9633_set_rgb(&_pca9633_dev, 0, 255, 0);
    ztimer_sleep(ZTIMER_MSEC, 750);
    pca9633_set_rgb(&_pca9633_dev, 0, 0, 255);
    ztimer_sleep(ZTIMER_MSEC, 750);
    pca9633_set_rgb(&_pca9633_dev, 255, 255, 255);

    aip31068_clear(&_aip31068_dev);
    aip31068_print(&_aip31068_dev, " Please assign");
    aip31068_set_cursor_position(&_aip31068_dev, 1, 0);
    aip31068_print(&_aip31068_dev, " a master key");
    _scan_uid(_master_id, sizeof(_master_id));

    aip31068_clear(&_aip31068_dev);
    aip31068_print(&_aip31068_dev, "   Master key");
    aip31068_set_cursor_position(&_aip31068_dev, 1, 0);
    aip31068_print(&_aip31068_dev, "  was assigned");

    ztimer_sleep(ZTIMER_MSEC, 1500);
    aip31068_clear(&_aip31068_dev);
    _enter_standby_mode();

    /* continuous scanning */
    uint8_t tmp_id[ID_LEN];
    while (1) {
        _scan_uid(tmp_id, sizeof(tmp_id));

        if (memcmp(_master_id, tmp_id, ID_LEN) == 0) {
            _leave_standby_mode();
            DEBUG("Enter master mode\n");
            _enter_master_mode();
            _enter_standby_mode();
        }
        else {
            _leave_standby_mode();

            id_item search_item;
            memcpy(search_item.id, tmp_id, ID_LEN);

            id_item *found_item;

            DL_SEARCH(_stored_ids, found_item, &search_item, _my_cmp);
            if (found_item != NULL) {
                DEBUG("Access granted\n");

                aip31068_clear(&_aip31068_dev);
                aip31068_print(&_aip31068_dev, " Access granted");
                pca9633_set_rgb(&_pca9633_dev, 0, 255, 0);
            }
            else {
                DEBUG("Access denied\n");

                aip31068_clear(&_aip31068_dev);
                aip31068_print(&_aip31068_dev, " Access denied");
                pca9633_set_rgb(&_pca9633_dev, 255, 0, 0);
            }

            ztimer_sleep(ZTIMER_MSEC, 1500);

            aip31068_clear(&_aip31068_dev);
            _enter_standby_mode();
            pca9633_set_rgb(&_pca9633_dev, 255, 255, 255);
        }
    }

    return 0;
}
