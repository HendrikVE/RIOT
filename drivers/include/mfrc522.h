/*
 * Copyright (C) 2020 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_mfrc522 MFRC522 controller
 * @ingroup     drivers_actuators
 * @brief       Device driver for the NXP MFRC522
 *
 * @{
 *
 * @author      Hendrik van Essen <hendrik.ve@fu-berlin.de>
 * @file
 */

#ifndef MFRC522_H
#define MFRC522_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdbool.h"

#include "periph/spi.h"

#include "mfrc522_regs.h"

/**
 * @brief   MFRC522 device initialization parameters
 */
typedef struct {
    spi_t spi_dev;          /**< SPI bus the display is connected to */
} mfrc522_params_t;

/**
 * @brief   MFRC522 device data structure type
 */
typedef struct {
    mfrc522_params_t params;     /**< Device initialization parameters */
} mfrc522_t;

/**
 * @brief   MFRC522 driver error codes
 */
enum {
    MFRC522_OK              = 0,    /**< Success */
    MFRC522_ERROR_I2C       = 1,    /**< I2C communication error */
};

/**
 * @brief Initialization.
 *
 * @param[in] dev       Device descriptor of the MFRC522
 * @param[in] params    Parameters for device initialization
 */
int mfrc522_init(pca9633_t *dev, const mfrc522_params_t *params);

#ifdef __cplusplus
}
#endif

#endif /* MFRC522_H */
/** @} */
