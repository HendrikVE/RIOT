/*
 * Copyright (C) 2020 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_actuators
 * @brief       Default configuration for the MFRC522 controller
 *
 * @{
 *
 * @author      Hendrik van Essen <hendrik.ve@fu-berlin.de>
 * @file
 */
#ifndef MFRC522_PARAMS_H
#define MFRC522_PARAMS_H

#include <stdbool.h>

#include "periph/spi.h"

#include "mfrc522_regs.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
* @name    Set default configuration parameters
* @{
*/
#ifndef MFRC522_PARAM_I2C_DEV
/** I2C device is SPI_DEV(0) */
#define MFRC522_PARAM_I2C_DEV               SPI_DEV(0)
#endif

#ifndef MFRC522_PARAMS
#define MFRC522_PARAMS                                          \
    {                                                           \
        .spi_dev = MFRC522_PARAM_I2C_DEV,                       \
    }
#endif /* MFRC522_PARAMS */
/**@}*/

/**
 * @brief   Allocate some memory to store the actual configuration
 */
static const mfrc522_params_t mfrc522_params[] =
{
        MFRC522_PARAMS
};

#ifdef __cplusplus
}
#endif

#endif /* MFRC522_PARAMS_H */
/** @} */
