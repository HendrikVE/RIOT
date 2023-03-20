/*
 * Copyright (C) 2020 Nalys
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Test application for the candev abstraction
 *
 * @author      Wouter Symons <wosym@airsantelmo.com>
 * @author      Toon Stegen <tstegen@nalys-group.com>
 *
 * @}
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "can/device.h"

#include "can_params.h"
#include "periph/can.h"

static can_t can_samd5x;

#define ENABLE_DEBUG 0
#include <debug.h>

static candev_t *candev = NULL;

static void _can_event_callback(candev_t *dev, candev_event_t event, void *arg)
{
    (void)arg;
    (void)dev;
    struct can_frame *frame;

    switch (event) {
    case CANDEV_EVENT_ISR:
        DEBUG("_can_event: CANDEV_EVENT_ISR\n");
        // dev->driver->isr(candev);
        break;
    case CANDEV_EVENT_WAKE_UP:
        DEBUG("_can_event: CANDEV_EVENT_WAKE_UP\n");
        break;
    case CANDEV_EVENT_TX_CONFIRMATION:
        DEBUG("_can_event: CANDEV_EVENT_TX_CONFIRMATION\n");
        break;
    case CANDEV_EVENT_TX_ERROR:
        DEBUG("_can_event: CANDEV_EVENT_TX_ERROR\n");
        break;
    case CANDEV_EVENT_RX_INDICATION:
        DEBUG("_can_event: CANDEV_EVENT_RX_INDICATION\n");

        frame = (struct can_frame *)arg;

        DEBUG("            id: %" PRIx32 " dlc: %" PRIx8 " data: ", frame->can_id & 0x1FFFFFFF,
              frame->can_dlc);
        for (uint8_t i = 0; i < frame->can_dlc; i++) {
            DEBUG("0x%X ", frame->data[i]);
        }
        DEBUG_PUTS("");

        break;
    case CANDEV_EVENT_RX_ERROR:
        DEBUG("_can_event: CANDEV_EVENT_RX_ERROR\n");
        break;
    case CANDEV_EVENT_BUS_OFF:
        // dev->state = CAN_STATE_BUS_OFF;
        break;
    case CANDEV_EVENT_ERROR_PASSIVE:
        // dev->state = CAN_STATE_ERROR_PASSIVE;
        break;
    case CANDEV_EVENT_ERROR_WARNING:
        // dev->state = CAN_STATE_ERROR_WARNING;
        break;
    default:
        DEBUG("_can_event: unknown event\n");
        break;
    }
}

int main(void)
{
    puts("candev test application\n");

    can_init(&can_samd5x, &candev_conf[1]);

    candev->event_callback = _can_event_callback;
    candev->isr_arg = NULL;
    candev = &(can_samd5x.candev);

    candev->driver->init(candev);

    return 0;
}
