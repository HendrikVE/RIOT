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
#include "ztimer.h"

static can_t can_samd5x;
candev_t *candev = NULL;

#define ENABLE_DEBUG 1
#include <debug.h>

static void _can_event_callback(candev_t *dev, candev_event_t event, void *arg)
{
    (void)dev;
    struct can_frame *frame;

    switch (event) {
    case CANDEV_EVENT_ISR:
        DEBUG("_can_event: CANDEV_EVENT_ISR\n");
        dev->driver->isr(candev);
        break;
    case CANDEV_EVENT_WAKE_UP:
        DEBUG("_can_event: CANDEV_EVENT_WAKE_UP\n");
        break;
    case CANDEV_EVENT_TX_CONFIRMATION:
        DEBUG("_can_event: CANDEV_EVENT_TX_CONFIRMATION\n");
        frame = (struct can_frame *)arg;
        DEBUG("frame to send ID = 0x%08lx\n", frame->can_id);
        break;
    case CANDEV_EVENT_TX_ERROR:
        DEBUG("_can_event: CANDEV_EVENT_TX_ERROR\n");
        break;
    case CANDEV_EVENT_RX_INDICATION:
        DEBUG("_can_event: CANDEV_EVENT_RX_INDICATION\n");
        break;
    case CANDEV_EVENT_RX_ERROR:
        DEBUG("_can_event: CANDEV_EVENT_RX_ERROR\n");
        break;
    default:
        DEBUG("_can_event: unknown event\n");
        break;
    }
}

int main(void)
{
    // Wait for the terminal to boot and catch our debug output
    ztimer_sleep(ZTIMER_MSEC, 500);

    puts("candev test application\n");

    gpio_init(GPIO_PIN(PC, 13), GPIO_IN);
    DEBUG("stby = %d\n", gpio_read(GPIO_PIN(PC, 13)));
    can_init(&can_samd5x, &candev_conf[1]);

    candev = &(can_samd5x.candev);
    candev->event_callback = _can_event_callback;
    candev->isr_arg = NULL;

    candev->driver->init(candev);
    candev_samd5x_irq_init(candev, CAN_TSW_IRQ, CAN_IRQ_LINE_1);
    candev_samd5x_irq_init(candev, CAN_TEFN_IRQ, CAN_IRQ_LINE_1);
    candev_samd5x_irq_init(candev, CAN_TFE_IRQ, CAN_IRQ_LINE_1);
    candev_samd5x_irq_init(candev, CAN_DRX_IRQ, CAN_IRQ_LINE_1);

    struct can_filter filter = {0};
    filter.can_filter_conf = CAN_FILTER_RX_FIFO_1;
    filter.can_filter_type = CAN_FILTER_TYPE_CLASSIC;
    filter.can_id = 0x0111;
    filter.can_mask = 0x7FF;
    candev->driver->set_filter(candev, &filter);
    filter.can_id = 0x0113;
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

    filter.can_id = 0x8C000001;
    filter.can_mask = 0x9FFFFFFF;
    candev->driver->remove_filter(candev, &filter);

    struct can_frame frame_1 = {
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

    struct can_frame frame_2 = {
        .can_dlc = 5,
        .can_id = 0x9B001122,
        .data = {
            4,
            5,
            6,
            7,
            0xE
        }
    };

    candev_samd5x_set_test_mode(candev, false);

    candev->driver->send(candev, &frame_1);
    candev->driver->send(candev, &frame_2);

    return 0;
}
