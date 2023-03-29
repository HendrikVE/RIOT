/**
 * @ingroup     cpu_samd5x
 * @{
 *
 * @file
 * @brief       Implementation of the CAN controller driver
 *
 * @author      Firas Hamdi <firas.hamdi@ml-pa.com>
 * @author      Hendrik van Essen <hendrik.vanessen@ml-pa.com>
 * @}
 */

#include <assert.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <cpu_conf.h>

#include "candev_samd5x.h"
#include "periph/can.h"
#include "can/device.h"
#include "periph_cpu.h"
#include "ztimer.h"

#define ENABLE_DEBUG 1
#include "debug.h"

typedef enum {
    MODE_INIT,
    MODE_MONITORING,
    MODE_INTERNAL_LOOP_BACK_MODE,
    MODE_EXTERNAL_LOOP_BACK_MODE
} can_mode_t;

static can_t *_can;

static int _init(candev_t *candev);
static int _send(candev_t *candev, const struct can_frame *frame);
static int _set_filter(candev_t *candev, const struct can_filter *filter);
static int _remove_filter(candev_t *candev, const struct can_filter *filter);
static void _isr(candev_t *candev);

static int _enter_mode(Can *can, can_mode_t can_mode);
static int _set_mode(Can *can, can_mode_t can_mode);
static int _change_mode(Can *can, can_mode_t can_mode);

static const candev_driver_t candev_samd5x_driver = {
    .init = _init,
    .send = _send,
    .set_filter = _set_filter,
    .remove_filter = _remove_filter,
    .isr = _isr,
};

static const struct can_bittiming_const bittiming_const = {
    .tseg1_min = 1,
    .tseg1_max = 256,
    .tseg2_min = 1,
    .tseg2_max = 128,
    .sjw_max = 1,
    .brp_min = 1,
    .brp_max = 512,
    .brp_inc = 1,
};

static void _dump_tx_regs(can_t *dev)
{
    DEBUG_PUTS("############### TX REGISTER DUMP ###############");

    DEBUG("TXBC = %08lx\n", dev->conf->can->TXBC.reg);
    DEBUG("    TFQM=%d TFQS=%d NDTB=%d TBSA=%d\n", dev->conf->can->TXBC.bit.TFQM, dev->conf->can->TXBC.bit.TFQS, dev->conf->can->TXBC.bit.NDTB, dev->conf->can->TXBC.bit.TBSA);
    DEBUG("TXESC = %08lx\n", dev->conf->can->TXESC.reg);
    DEBUG("    TBDS=%d\n", dev->conf->can->TXESC.bit.TBDS);
    DEBUG("TXEFC = %08lx\n", dev->conf->can->TXEFC.reg);
    DEBUG("    EFWM=%d EFS=%d EFSA=%d\n", dev->conf->can->TXEFC.bit.EFWM, dev->conf->can->TXEFC.bit.EFS, dev->conf->can->TXEFC.bit.EFSA);

    DEBUG("TXFQS = %08lx\n", dev->conf->can->TXFQS.reg);
    DEBUG("    TFGI=%d TFQPI=%d TFFL=%d\n", dev->conf->can->TXFQS.bit.TFGI, dev->conf->can->TXFQS.bit.TFQPI, dev->conf->can->TXFQS.bit.TFFL);

    DEBUG("TXBRP = %08lx\n", dev->conf->can->TXBRP.reg);
    DEBUG("TXBAR = %08lx\n", dev->conf->can->TXBAR.reg);
    DEBUG("TXBCR = %08lx\n", dev->conf->can->TXBCR.reg);

    DEBUG("TXBTO = %08lx\n", dev->conf->can->TXBTO.reg);
    DEBUG("TXBCF = %08lx\n", dev->conf->can->TXBCF.reg);

    DEBUG("TXEFS = %08lx\n", dev->conf->can->TXEFS.reg);

    DEBUG_PUTS("################################################");
}

static int _power_on(can_t *dev)
{
    if (dev->conf->can == CAN0) {
        DEBUG_PUTS("CAN0 controller is used");
        MCLK->AHBMASK.reg |= MCLK_AHBMASK_CAN0;
    }
    else if (dev->conf->can == CAN1) {
        DEBUG_PUTS("CAN1 controller is used");
        MCLK->AHBMASK.reg |= MCLK_AHBMASK_CAN1;
    }
    else {
        DEBUG_PUTS("Unsupported CAN channel");
        return -ENOTSUP;
    }

    return 0;
}

static void _enter_init_mode(Can *can)
{
    can->CCCR.reg |= CAN_CCCR_INIT;
    // Todo: Add timeout
    while(!(can->CCCR.reg & CAN_CCCR_INIT));
    DEBUG_PUTS("Device in init mode");
}

static void _exit_init_mode(Can *can)
{
    if (can->CCCR.reg & CAN_CCCR_INIT) {
        can->CCCR.reg &= ~CAN_CCCR_INIT;
    }

    // Todo: Add timeout
    while(can->CCCR.reg & CAN_CCCR_INIT);
    DEBUG_PUTS("Device out of init mode");
}

static void _enter_sleep_mode(Can *can)
{
    can->CCCR.reg |= CAN_CCCR_CSR;
    // Todo: Add timeout
    while(!(can->CCCR.reg & CAN_CCCR_CSA));
    DEBUG("CCCR = 0x%08lx\n", can->CCCR.reg);
    DEBUG_PUTS("Device in sleep mode");
}

static void _exit_sleep_mode(Can *can)
{
    can->CCCR.reg &= ~CAN_CCCR_CSR;
    // Todo: Add timeout
    while(can->CCCR.reg & CAN_CCCR_CSA);
    DEBUG("CCCR = 0x%08lx\n", can->CCCR.reg);
    DEBUG_PUTS("Device out of sleep mode");
}

static int _enter_mode(Can *can, can_mode_t can_mode)
{
    return _change_mode(can, can_mode);
}

static int _set_mode(Can *can, can_mode_t can_mode)
{
    int rc = _change_mode(can, can_mode);

    if (rc < 0) {
        return rc;
    }

    _exit_init_mode(can);

    return 0;
}

static int _change_mode(Can *can, can_mode_t can_mode)
{
    _enter_init_mode(can);
    can->CCCR.reg |= CAN_CCCR_CCE;

    switch (can_mode) {
        case MODE_INIT:
            DEBUG_PUTS("MODE_INIT");
            can->CCCR.reg &= ~(CAN_CCCR_TEST | CAN_CCCR_MON);
            break;
        case MODE_MONITORING:
            DEBUG_PUTS("MODE_MONITORING");
            can->CCCR.reg &= ~CAN_CCCR_TEST;
            can->CCCR.reg |= CAN_CCCR_MON;
            break;
        case MODE_EXTERNAL_LOOP_BACK_MODE:
            DEBUG_PUTS("MODE_EXTERNAL_LOOP_BACK_MODE");
            can->CCCR.reg &= ~CAN_CCCR_MON;
            can->CCCR.reg |= CAN_CCCR_TEST;
            can->TEST.reg |= CAN_TEST_LBCK;
            break;
        case MODE_INTERNAL_LOOP_BACK_MODE:
            DEBUG_PUTS("MODE_INTERNAL_LOOP_BACK_MODE");
            can->CCCR.reg |= (CAN_CCCR_TEST | CAN_CCCR_MON);
            can->TEST.reg |= CAN_TEST_LBCK;
            break;
        default:
            DEBUG_PUTS("Unsupported mode");
            _exit_init_mode(can);
            return -ENOTSUP;
    }

    return 0;
}

static int _setup_clock(can_t *dev)
{
    if (dev->conf->can == CAN0) {
        GCLK->PCHCTRL[CAN0_GCLK_ID].reg = GCLK_PCHCTRL_CHEN | GCLK_PCHCTRL_GEN(SAM0_GCLK_MAIN);
    }
    else if (dev->conf->can == CAN1) {
        GCLK->PCHCTRL[CAN1_GCLK_ID].reg = GCLK_PCHCTRL_CHEN | GCLK_PCHCTRL_GEN(SAM0_GCLK_MAIN);
    }
    else {
        DEBUG_PUTS("CAN channel not supported");
        return -ENOTSUP;
    }

    return 0;
}

static void _set_bit_timing(can_t *dev)
{
    assert(dev->candev.bittiming.sjw >= 1);
    assert(dev->candev.bittiming.phase_seg2 >= 1);
    assert(dev->candev.bittiming.phase_seg1 + dev->candev.bittiming.prop_seg >= 1);
    assert(dev->candev.bittiming.brp >= 1);

    DEBUG("bitrate=%" PRIu32 ", sample_point=%" PRIu32 ", brp=%" PRIu32 ", prop_seg=%" PRIu32
          ", phase_seg1=%" PRIu32 ", phase_seg2=%" PRIu32 ", sjw=%" PRIu32 "\n", dev->candev.bittiming.bitrate, dev->candev.bittiming.sample_point,
          dev->candev.bittiming.brp, dev->candev.bittiming.prop_seg, dev->candev.bittiming.phase_seg1, dev->candev.bittiming.phase_seg2, dev->candev.bittiming.sjw);

    dev->conf->can->NBTP.reg = (uint32_t)((CAN_NBTP_NTSEG2(dev->candev.bittiming.phase_seg2 - 1)) |
                                          (CAN_NBTP_NTSEG1(dev->candev.bittiming.phase_seg1 + dev->candev.bittiming.prop_seg - 1)) |
                                          (CAN_NBTP_NBRP(dev->candev.bittiming.brp - 1)) |
                                          (CAN_NBTP_NSJW(dev->candev.bittiming.sjw - 1)));
}

void candev_samd5x_set_pins(can_t *dev)
{
    assert(dev->conf->tx_pin != GPIO_UNDEF);
    assert(dev->conf->rx_pin != GPIO_UNDEF);

    gpio_init_mux(dev->conf->tx_pin, dev->conf->mux);
    gpio_init_mux(dev->conf->rx_pin, dev->conf->mux);
}

void candev_samd5x_tdc_control(can_t *dev)
{
    if(dev->conf->tdc_ctrl) {
        DEBUG_PUTS("Enable Transceiver Delay Compensation");
        dev->conf->can->DBTP.reg |= CAN_DBTP_TDC;
    }
    else {
        DEBUG_PUTS("Disable Transceiver Delay Compensation");
        dev->conf->can->DBTP.reg &= ~(CAN_DBTP_TDC);
    }
}

void candev_samd5x_set_test_mode(candev_t *candev, bool silent)
{
    can_t *dev = container_of(candev, can_t, candev);

    if (silent) {
        _set_mode(dev->conf->can, MODE_INTERNAL_LOOP_BACK_MODE);
    }
    else {
        _set_mode(dev->conf->can, MODE_EXTERNAL_LOOP_BACK_MODE);
    }

    _dump_tx_regs(dev);
}

void can_init(can_t *dev, const can_conf_t *conf)
{
    dev->candev.driver = &candev_samd5x_driver;

    struct can_bittiming timing = { .bitrate = CANDEV_SAMD5X_DEFAULT_BITRATE,
                                    .sample_point = CANDEV_SAMD5X_DEFAULT_SPT };

    uint32_t clk_freq = sam0_gclk_freq(SAM0_GCLK_MAIN);
    can_device_calc_bittiming(clk_freq, &bittiming_const, &timing);

    memcpy(&dev->candev.bittiming, &timing, sizeof(timing));
    dev->conf = conf;
}

static int _init(candev_t *candev)
{
    can_t *dev = container_of(candev, can_t, candev);
    int res = 0;

    sam0_gclk_enable(SAM0_GCLK_MAIN);

    res = _setup_clock(dev);
    if (res < 0) {
        DEBUG("candev_samd5x: _init: _setup_clock returned with an error (%d)\n", res);
    }

    res = _power_on(dev);
    if (res < 0) {
        DEBUG("candev_samd5x: _init: _power_on returned with an error (%d)\n", res);
    }

    candev_samd5x_set_pins(dev);

    _exit_sleep_mode(dev->conf->can);

    res = _enter_mode(dev->conf->can, MODE_INIT);
    if (res < 0) {
        return res;
    }

    dev->conf->can->CCCR.reg &= ~(CAN_CCCR_FDOE | CAN_CCCR_BRSE | CAN_CCCR_ASM);

    candev_samd5x_tdc_control(dev);

    /*Configure the start addresses of the RAM message sections */
    dev->conf->can->SIDFC.reg = CAN_SIDFC_FLSSA((uint32_t)(dev->msg_ram_conf.std_filter))
        | CAN_SIDFC_LSS((uint32_t)(ARRAY_SIZE(dev->msg_ram_conf.std_filter)));

    dev->conf->can->XIDFC.reg = CAN_XIDFC_FLESA((uint32_t)(dev->msg_ram_conf.ext_filter))
        | CAN_XIDFC_LSE((uint32_t)(ARRAY_SIZE(dev->msg_ram_conf.ext_filter)));

    dev->conf->can->RXF0C.reg = CAN_RXF0C_F0SA((uint32_t)(dev->msg_ram_conf.rx_fifo_0))
        | CAN_RXF0C_F0S((uint32_t)(ARRAY_SIZE(dev->msg_ram_conf.rx_fifo_0)));

    dev->conf->can->RXF1C.reg = CAN_RXF1C_F1SA((uint32_t)(dev->msg_ram_conf.rx_fifo_1))
        | CAN_RXF1C_F1SA((uint32_t)(ARRAY_SIZE(dev->msg_ram_conf.rx_fifo_1)));

    dev->conf->can->RXBC.reg = CAN_RXBC_RBSA((uint32_t)(dev->msg_ram_conf.rx_buffer));

    dev->conf->can->TXEFC.reg = CAN_TXEFC_EFSA((uint32_t)(dev->msg_ram_conf.tx_event_fifo))
        | CAN_TXEFC_EFS((uint32_t)(ARRAY_SIZE(dev->msg_ram_conf.tx_event_fifo)));

    dev->conf->can->TXBC.reg = CAN_TXBC_TBSA((uint32_t)(dev->msg_ram_conf.tx_fifo_queue))
        | CAN_TXBC_TFQS((uint32_t)(ARRAY_SIZE(dev->msg_ram_conf.tx_fifo_queue)));

    dev->conf->can->TXESC.reg = CAN_TXESC_TBDS_DATA8;

    dev->conf->can->GFC.reg |= (CAN_GFC_ANFS(CAN_GFC_ANFS_REJECT_Val) |
                                CAN_GFC_ANFE(CAN_GFC_ANFE_REJECT_Val));

    _set_bit_timing(dev);

    // Enable TX FIFO
    dev->conf->can->TXBC.bit.TFQM = 0;

    dev->conf->can->IE.reg = (CAN_IE_BOE | CAN_IE_EWE | CAN_IE_EPE |
		                      CAN_IE_MRAFE | CAN_IE_TEFLE | CAN_IE_TEFNE |
                              CAN_IE_RF0NE | CAN_IE_RF1NE | CAN_IE_RF0LE |
		                      CAN_IE_RF1LE);


    /* Enable the peripheral's interrupt */
    if (dev->conf->can == CAN0) {
        NVIC_EnableIRQ(CAN0_IRQn);
    }
    else {
        NVIC_EnableIRQ(CAN1_IRQn);
    }

    /* Enable the interrupt lines */
    dev->conf->can->ILE.reg = CAN_ILE_EINT0 | CAN_ILE_EINT1;
    /* Enable the interrupt on every Tx buffer transmission */
    dev->conf->can->TXBTIE.reg = CAN_TXBTIE_MASK;

    _can = dev;

    DEBUG("before exit: CCCR = 0x%08lx\n", dev->conf->can->CCCR.reg);
    /* Exit initialization mode */
    _exit_init_mode(dev->conf->can);
    DEBUG("after exit: CCCR = 0x%08lx\n", dev->conf->can->CCCR.reg);

    return res;
}

/* Maybe do not provide this API to the user and init the interrupts according to your use case */
int candev_samd5x_irq_init(candev_t *candev, can_irq_source_t irq_source, can_irq_line_t irq_line)
{
    can_t *dev = container_of(candev, can_t, candev);

    assert((irq_line == CAN_IRQ_LINE_0) | (irq_line == CAN_IRQ_LINE_1));

    dev->conf->can->IE.reg |= (1 << irq_source);
    DEBUG("IE = 0x%08lx\n", dev->conf->can->IE.reg);

    if (irq_line == CAN_IRQ_LINE_0) {
        dev->conf->can->ILS.reg &= ~(1 << irq_source);
    }
    else {
        dev->conf->can->ILS.reg |= (1 << irq_source);
    }
    DEBUG("ILS = 0x%08lx\n", dev->conf->can->ILS.reg);

    return 0;
}

static int _send(candev_t *candev, const struct can_frame *frame)
{
    can_t *dev = container_of(candev, can_t, candev);

    if (frame->can_dlc > CAN_MAX_DLEN) {
        DEBUG_PUTS("CAN frame payload not supported");
        return -1;
    }

    /* Check if the Tx FIFO is full */
    if (dev->conf->can->TXFQS.reg & CAN_TXFQS_TFQF) {
        DEBUG_PUTS("Tx FIFO is full");
        return -1;
    }

    uint8_t get_idx = dev->conf->can->TXFQS.bit.TFGI;
    uint8_t put_idx = dev->conf->can->TXFQS.bit.TFQPI;

    _dump_tx_regs(dev);

    can_tx_buffer_t *can_tx_buf = &(dev->msg_ram_conf.tx_fifo_queue[put_idx]);

    // can_tx_buf->T0.id = frame->can_id & CAN_EFF_MASK;
    // can_tx_buf->T0.rtr = (frame->can_id & CAN_RTR_FLAG) >> 30;
    // can_tx_buf->T0.xtd = (frame->can_id & CAN_EFF_FLAG) >> 31;
    // can_tx_buf->T0.esi = 0;

    can_tx_buf->T0.id = frame->can_id & CAN_EFF_MASK;
    can_tx_buf->T0.rtr = 0;
    can_tx_buf->T0.xtd = 1;
    can_tx_buf->T0.esi = 0;

    can_tx_buf->T1.dlc = frame->can_dlc;
    can_tx_buf->T1.brs = 0;
    can_tx_buf->T1.fdf = 0;
    can_tx_buf->T1.efc = 1;

    can_mm_t can_mm = { 0 };
    can_mm.put = put_idx;
    can_mm.get = get_idx;
    memcpy(&(can_tx_buf->T1.mm), &can_mm, sizeof(can_mm_t));

    memcpy(can_tx_buf->data.data_8, frame->data, frame->can_dlc);

    /* Request transmission */
    dev->conf->can->TXBAR.reg |= (1 << put_idx);

    DEBUG_PUTS("Transmission requested");

    _dump_tx_regs(dev);

    DEBUG_PUTS("Wait for transmission to finish");

    bool bto = dev->conf->can->TXBTO.reg & (1 << put_idx);
    bool bcf = dev->conf->can->TXBCF.reg & (1 << put_idx);

    /* Wait for successful transmission */
    while(!(bto || bcf)) {
        bto = dev->conf->can->TXBTO.reg & (1 << put_idx);
        bcf = dev->conf->can->TXBCF.reg & (1 << put_idx);
    }

    if (bto && bcf) {
        DEBUG_PUTS("Successful transmission in spite of cancellation");
    }
    else if (bto) {
        DEBUG_PUTS("Successful transmission");
    }
    else if (bcf) {
        DEBUG_PUTS("Arbitration lost or frame transmission disturbed");
    }
    else {
        DEBUG_PUTS("SHOULD NOT HAPPEN");
        _dump_tx_regs(dev);
        return -1;
    }

    _dump_tx_regs(dev);

    if (dev->candev.event_callback) {
        dev->candev.event_callback(candev, CANDEV_EVENT_TX_CONFIRMATION, (void *)frame);
    }

    DEBUG_PUTS("\n\n\n");

    return 0;
}

static bool _find_filter(can_t *can, const struct can_filter *filter, bool is_std_filter, int16_t *idx)
{
    if (is_std_filter) {
        for (uint8_t i = 0; i < ARRAY_SIZE(can->msg_ram_conf.std_filter); i++) {
            if (((filter->can_id & CAN_SFF_MASK) == can->msg_ram_conf.std_filter[i].sfid1)) {
                *idx = i;
                return true;
            }
        }
    }
    else {
        for (uint8_t i = 0; i < ARRAY_SIZE(can->msg_ram_conf.ext_filter); i++) {
            if (((filter->can_id & CAN_EFF_MASK) == can->msg_ram_conf.ext_filter[i].F0.efid1)) {
                *idx = i;
                return true;
            }
        }
    }

    return false;
}

static int _set_filter(candev_t *candev, const struct can_filter *filter)
{
    can_t *dev = container_of(candev, can_t, candev);

    int16_t idx = 0;
    bool _filter_exists = false;
    if (filter->can_id & CAN_EFF_FLAG) {
        DEBUG_PUTS("Extended filter to add in the extended filter section of the message RAM");
        /* Check if the filter already exists */
        _filter_exists = _find_filter(dev, filter, false, &idx);
        if (_filter_exists) {
            DEBUG_PUTS("Extended filter already exists --> Update it");
        }
        else {
            /* Find a free slot where to save the filter */
            /* Use static function instead */
            for (;(uint16_t)idx < ARRAY_SIZE(dev->msg_ram_conf.ext_filter); idx++) {
                if (dev->msg_ram_conf.ext_filter[idx].F0.efec == CAN_FILTER_DISABLE) {
                    DEBUG_PUTS("empty slot");
                    break;
                }
            }
        }

        if (idx == ARRAY_SIZE(dev->msg_ram_conf.ext_filter)) {
            DEBUG_PUTS("Reached maximum capacity of extended filters --> Could not add filter");
            return -1;
        }

        DEBUG("Filter to add at idx = %d\n", idx);
        dev->msg_ram_conf.ext_filter[idx].F0.efid1 = filter->can_id;
        dev->msg_ram_conf.ext_filter[idx].F0.efec = filter->can_filter_conf;
        dev->msg_ram_conf.ext_filter[idx].F1.efid2 = filter->can_mask & CAN_EFF_MASK;
        dev->msg_ram_conf.ext_filter[idx].F1.eft = filter->can_filter_type & CAN_EFF_MASK;

        for (uint8_t i = 0; i < ARRAY_SIZE(dev->msg_ram_conf.ext_filter); i++) {
            DEBUG("can->msg_ram_conf.std_filter[%u] = 0x%08lx, filter conf = %u\n", i,
                    (uint32_t)(dev->msg_ram_conf.ext_filter[i].F0.efid1), dev->msg_ram_conf.ext_filter[i].F0.efec);
        }
    }
    else {
        DEBUG_PUTS("Standard filter to add in the standard filter section of the message RAM");
        /* Check if the filter already exists */
        _filter_exists = _find_filter(dev, filter, true, &idx);
        if (_filter_exists) {
            DEBUG_PUTS("Standard filter already exists --> Update it");
        }
        else {
            /* Find a free slot where to save the filter */
            /* Use static function instead */
            for (; (uint16_t)idx < ARRAY_SIZE(dev->msg_ram_conf.std_filter); idx++) {
                /* Find a free slot where to save the filter */
                if (dev->msg_ram_conf.std_filter[idx].sfec == CAN_FILTER_DISABLE) {
                    DEBUG_PUTS("empty slot");
                    break;
                }
            }
        }

        if (idx == ARRAY_SIZE(dev->msg_ram_conf.std_filter)) {
            DEBUG_PUTS("Reached maximum capacity of standard filters --> Could not add filter");
            return -1;
        }

        DEBUG("Filter to add at idx = %d\n", idx);
        dev->msg_ram_conf.std_filter[idx].sfec = filter->can_filter_conf;
        dev->msg_ram_conf.std_filter[idx].sft = filter->can_filter_type;
        dev->msg_ram_conf.std_filter[idx].sfid2 = filter->can_mask & CAN_SFF_MASK;
        dev->msg_ram_conf.std_filter[idx].sfid1 = filter->can_id & CAN_SFF_MASK;

        for (uint8_t i = 0; i < ARRAY_SIZE(dev->msg_ram_conf.std_filter); i++) {
            DEBUG("can->msg_ram_conf.std_filter[%u] = 0x%08lx, filter conf = %u\n", i,
                    (uint32_t)(dev->msg_ram_conf.std_filter[i].sfid1), dev->msg_ram_conf.std_filter[i].sfec);
        }
    }

    return idx;
}

static int _remove_filter(candev_t *candev, const struct can_filter *filter)
{
    can_t *dev = container_of(candev, can_t, candev);

    int16_t idx = 0;
    bool _filter_exists = false;
    if (filter->can_id & CAN_EFF_FLAG) {
        _filter_exists = _find_filter(dev, filter, false, &idx);
        if (_filter_exists) {
            DEBUG("Filter to disable at idx = %d\n", idx);
            dev->msg_ram_conf.ext_filter[idx].F0.efec = CAN_FILTER_DISABLE;

            for (uint8_t i = 0; i < ARRAY_SIZE(dev->msg_ram_conf.ext_filter); i++) {
                DEBUG("can->msg_ram_conf.std_filter[%u] = 0x%08lx, filter conf = %u\n", i,
                        (uint32_t)(dev->msg_ram_conf.ext_filter[i].F0.efid1), dev->msg_ram_conf.ext_filter[i].F0.efec);
            }
        }
        else {
            DEBUG_PUTS("Filter not found");
        }
    }
    else {
        _filter_exists = _find_filter(dev, filter, true, &idx);
        if(_filter_exists) {
            DEBUG("Filter to disable at idx = %d\n", idx);
            dev->msg_ram_conf.std_filter[idx].sfec = CAN_FILTER_DISABLE;

            for (uint8_t i = 0; i < ARRAY_SIZE(dev->msg_ram_conf.std_filter); i++) {
                DEBUG("can->msg_ram_conf.std_filter[%u] = 0x%08lx, filter conf = %u\n", i,
                        (uint32_t)(dev->msg_ram_conf.std_filter[i].sfid1), dev->msg_ram_conf.std_filter[i].sfec);
            }
        }
        else {
            DEBUG_PUTS("Filter not found");
        }
    }

    return idx;
}

static void _isr(candev_t *candev)
{
    can_t *dev = container_of(candev, can_t, candev);

    uint32_t irq_reg = dev->conf->can->IR.reg;

    if (irq_reg & CAN_IR_TSW) {
        DEBUG_PUTS("Timestamp wraparound interrupt");
        /* Clear the interrupt source flag */
        dev->conf->can->IR.reg |= CAN_IR_TSW;
    }
}

void ISR_CAN1(void)
{
    DEBUG_PUTS("ISR CAN1");

    if (_can->conf->can->IR.reg & CAN_IR_RF0N) {
        _can->conf->can->IR.reg |= CAN_IR_RF0N;
        DEBUG_PUTS("CAN_IR_RF0N");
    }

    if (_can->conf->can->IR.reg & CAN_IR_RF0W) {
        _can->conf->can->IR.reg |= CAN_IR_RF0W;
        DEBUG_PUTS("CAN_IR_RF0W");
    }

    if (_can->conf->can->IR.reg & CAN_IR_RF0F) {
        _can->conf->can->IR.reg |= CAN_IR_RF0F;
        DEBUG_PUTS("CAN_IR_RF0F");
    }

    if (_can->conf->can->IR.reg & CAN_IR_RF0L) {
        _can->conf->can->IR.reg |= CAN_IR_RF0L;
        DEBUG_PUTS("CAN_IR_RF0L");
    }

    if (_can->conf->can->IR.reg & CAN_IR_RF1N) {
        _can->conf->can->IR.reg |= CAN_IR_RF1N;
        DEBUG_PUTS("CAN_IR_RF1N");
    }

    if (_can->conf->can->IR.reg & CAN_IR_RF1W) {
        _can->conf->can->IR.reg |= CAN_IR_RF1W;
        DEBUG_PUTS("CAN_IR_RF1W");
    }

    if (_can->conf->can->IR.reg & CAN_IR_RF1F) {
        _can->conf->can->IR.reg |= CAN_IR_RF1F;
        DEBUG_PUTS("CAN_IR_RF1F");
    }

    if (_can->conf->can->IR.reg & CAN_IR_RF1L) {
        _can->conf->can->IR.reg |= CAN_IR_RF1L;
        DEBUG_PUTS("CAN_IR_RF1L");
    }

    if (_can->conf->can->IR.reg & CAN_IR_HPM) {
        _can->conf->can->IR.reg |= CAN_IR_HPM;
        DEBUG_PUTS("CAN_IR_HPM");
    }

    if (_can->conf->can->IR.reg & CAN_IR_TC) {
        _can->conf->can->IR.reg |= CAN_IR_TC;
        DEBUG_PUTS("CAN_IR_TC");
    }

    if (_can->conf->can->IR.reg & CAN_IR_TCF) {
        _can->conf->can->IR.reg |= CAN_IR_TCF;
        DEBUG_PUTS("CAN_IR_TCF");
    }

    if (_can->conf->can->IR.reg & CAN_IR_TFE) {
        _can->conf->can->IR.reg |= CAN_IR_TFE;
        DEBUG_PUTS("CAN_IR_TFE");
    }

    if (_can->conf->can->IR.reg & CAN_IR_TEFN) {
        _can->conf->can->IR.reg |= CAN_IR_TEFN;
        DEBUG_PUTS("CAN_IR_TEFN");
    }

    if (_can->conf->can->IR.reg & CAN_IR_TEFW) {
        _can->conf->can->IR.reg |= CAN_IR_TEFW;
        DEBUG_PUTS("CAN_IR_TEFW");
    }

    if (_can->conf->can->IR.reg & CAN_IR_TEFF) {
        _can->conf->can->IR.reg |= CAN_IR_TEFF;
        DEBUG_PUTS("CAN_IR_TEFF");
    }

    if (_can->conf->can->IR.reg & CAN_IR_TEFL) {
        _can->conf->can->IR.reg |= CAN_IR_TEFL;
        DEBUG_PUTS("CAN_IR_TEFL");
    }

    if (_can->conf->can->IR.reg & CAN_IR_TSW) {
        _can->conf->can->IR.reg |= CAN_IR_TSW;
        DEBUG_PUTS("CAN_IR_TSW");
    }

    if (_can->conf->can->IR.reg & CAN_IR_MRAF) {
        _can->conf->can->IR.reg |= CAN_IR_MRAF;
        DEBUG_PUTS("CAN_IR_MRAF");
    }

    if (_can->conf->can->IR.reg & CAN_IR_TOO) {
        _can->conf->can->IR.reg |= CAN_IR_TOO;
        DEBUG_PUTS("CAN_IR_TOO");
    }

    if (_can->conf->can->IR.reg & CAN_IR_DRX) {
        _can->conf->can->IR.reg |= CAN_IR_DRX;
        DEBUG_PUTS("CAN_IR_DRX");
    }

    if (_can->conf->can->IR.reg & CAN_IR_BEC) {
        _can->conf->can->IR.reg |= CAN_IR_BEC;
        DEBUG_PUTS("CAN_IR_BEC");
    }

    if (_can->conf->can->IR.reg & CAN_IR_BEU) {
        _can->conf->can->IR.reg |= CAN_IR_BEU;
        DEBUG_PUTS("CAN_IR_BEU");
    }

    if (_can->conf->can->IR.reg & CAN_IR_ELO) {
        _can->conf->can->IR.reg |= CAN_IR_ELO;
        DEBUG_PUTS("CAN_IR_ELO");
    }

    if (_can->conf->can->IR.reg & CAN_IR_EP) {
        _can->conf->can->IR.reg |= CAN_IR_EP;
        DEBUG_PUTS("CAN_IR_EP");
    }

    if (_can->conf->can->IR.reg & CAN_IR_EW) {
        _can->conf->can->IR.reg |= CAN_IR_EW;
        DEBUG_PUTS("CAN_IR_EW");
    }

    if (_can->conf->can->IR.reg & CAN_IR_BO) {
        _can->conf->can->IR.reg |= CAN_IR_BO;
        DEBUG_PUTS("CAN_IR_BO");
    }

    if (_can->conf->can->IR.reg & CAN_IR_WDI) {
        _can->conf->can->IR.reg |= CAN_IR_WDI;
        DEBUG_PUTS("CAN_IR_WDI");
    }

    if (_can->conf->can->IR.reg & CAN_IR_PEA) {
        _can->conf->can->IR.reg |= CAN_IR_PEA;
        DEBUG_PUTS("CAN_IR_PEA");
    }

    if (_can->conf->can->IR.reg & CAN_IR_PED) {
        _can->conf->can->IR.reg |= CAN_IR_PED;
        DEBUG_PUTS("CAN_IR_PED");
    }

    if (_can->conf->can->IR.reg & CAN_IR_ARA) {
        _can->conf->can->IR.reg |= CAN_IR_ARA;
        DEBUG_PUTS("CAN_IR_ARA");
    }

    DEBUG("ECR =%08lx\n", _can->conf->can->ECR.reg);

    DEBUG("PRS = %08lx\n", _can->conf->can->PSR.reg);

    if (_can->candev.event_callback) {
        DEBUG("execute callback");
        _can->candev.event_callback(&(_can->candev), CANDEV_EVENT_ISR, NULL);
    }
}

void ISR_CAN0(void)
{
    DEBUG_PUTS("ISR CAN0");

    if (_can->candev.event_callback) {
        DEBUG("execute callback");
        _can->candev.event_callback(&(_can->candev), CANDEV_EVENT_ISR, NULL);
    }
}