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

#ifndef TEST_MODE
#define TEST_MODE     0
#endif

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

    struct can_frame frame = {
        .can_dlc = 5,
        .can_id = 0x8C122330,
        .data = {
            0,
            1,
            2,
            3,
            0xF
        }
    };

#if IS_ACTIVE(TEST_MODE)
    candev_samd5x_enter_test_mode(candev);
#endif

    candev->driver->send(candev, &frame);
    candev->driver->send(candev, &frame);
    candev->driver->send(candev, &frame);

    struct can_filter filter = {0};
    filter.can_filter_conf = CAN_FILTER_RX_FIFO_1;
    filter.can_filter_type = CAN_FILTER_TYPE_CLASSIC;
    filter.can_id = 0x0111;
    filter.can_mask = 0x7FF;
    candev->driver->set_filter(candev, &filter);
    filter.can_id = 0x0112;
    filter.can_filter_conf = CAN_FILTER_DISABLE;
    candev->driver->set_filter(candev, &filter);
    filter.can_id = 0x0112;
    filter.can_filter_conf = CAN_FILTER_RX_FIFO_0;
    candev->driver->set_filter(candev, &filter);
    filter.can_id = 0x8C000000;
    filter.can_mask = 0x9FFFFFFF;
    candev->driver->set_filter(candev, &filter);
    filter.can_id = 0x8C000001;
    filter.can_mask = 0x9FFFFFFF;
    candev->driver->set_filter(candev, &filter);

    return 0;
}
