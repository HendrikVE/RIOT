/*
 * Copyright (C) 2020 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_aip31068 AIP31068 I2C LCD controller
 * @ingroup     drivers_actuators
 * @brief       Device driver for AIP31068
 *
 * @{
 *
 * @author      Hendrik van Essen <hendrik.ve@fu-berlin.de>
 * @file
 */

#ifndef AIP31068_H
#define AIP31068_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdbool.h"
#include "periph/i2c.h"

#include "aip31068_regs.h"

/**
* Keys for custom symbols.
*/
typedef enum {
    CUSTOM_SYMBOL_1 = 0,
    CUSTOM_SYMBOL_2 = 1,
    CUSTOM_SYMBOL_3 = 2,
    CUSTOM_SYMBOL_4 = 3,
    CUSTOM_SYMBOL_5 = 4,
    CUSTOM_SYMBOL_6 = 5,
    CUSTOM_SYMBOL_7 = 6,
    CUSTOM_SYMBOL_8 = 7,
} aip31068_custom_symbol_t;

/**
 * Defines the direction of the text insertion. Starting from the cursor, either
 * increment the column of the cursor position after insertion (LEFT_TO_RIGHT),
 * or decrement the current column of the cursor position after insertion
 * (RIGHT_TO_LEFT).
 */
typedef enum {
    LEFT_TO_RIGHT,
    RIGHT_TO_LEFT,
} aip31068_text_insertion_mode_t;

/**
* Size of a character of the display in dots/pixels.
*/
typedef enum {
    FONT_SIZE_5x8,
    FONT_SIZE_5x10,
} aip31068_font_size_t;

/**
 * Bit mode for the display.
 */
typedef enum {
    BITMODE_4_BIT,
    BITMODE_8_BIT,
} aip31068_bit_mode_t;

/**
 * @brief   AIP31068 device initialization parameters
 */
typedef struct {
    i2c_t i2c_dev;                      /**< I2C device */
    uint16_t i2c_addr;                  /**< I2C address of device */
    uint8_t row_count;                  /**< number of rows */
    uint8_t col_count;                  /**< number of columns */
    aip31068_font_size_t font_size;  /**< font size */
    aip31068_bit_mode_t bit_mode;    /**< bit mode */
} aip31068_params_t;

/**
 * @brief   AIP31068 PWM device data structure type
 */
typedef struct {
    aip31068_params_t params;    /**< Device initialization parameters */
    uint8_t _curr_display_control;  /**< Current value of DISPLAY_CONTROL */
    uint8_t _curr_entry_mode_set;   /**< Current value of ENTRY_MODE_SET */
    uint8_t _progress_bar_row;      /**< The line where the progress bar should be displayed on. */
    bool _progress_bar_enabled;     /**< Whether progress bar feature is enabled or not. */
} aip31068_t;

/**
 * @brief   AIP31068 driver error codes
 */
typedef enum {
    AIP31068_OK              = 0,    /**< Success */
    AIP31068_ERROR_I2C       = 1,    /**< I2C communication error */
} aip31068_error_t;

/**
 * Initialization.
 *
 * @param[in] dev       Device descriptor of the AIP31068
 * @param[in] params    Parameters for device initialization
 *
 * @return  AIP31068_OK on success
 * @return  -AIP31068_ERROR_I2C if acquiring of I2C bus fails
 * @return  -EIO When slave device doesn't ACK the byte
 * @return  -ENXIO When no devices respond on the address sent on the bus
 * @return  -ETIMEDOUT When timeout occurs before device's response
 * @return  -EINVAL When an invalid argument is given
 * @return  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return  -EAGAIN When a lost bus arbitration occurs
 */
int aip31068_init(aip31068_t *dev, const aip31068_params_t *params);

/**
 * Turn on the display.
 *
 * @param[in] dev   Device descriptor of the AIP31068
 *
 * @return  AIP31068_OK on success
 * @return  -AIP31068_ERROR_I2C if acquiring of I2C bus fails
 * @return  -EIO When slave device doesn't ACK the byte
 * @return  -ENXIO When no devices respond on the address sent on the bus
 * @return  -ETIMEDOUT When timeout occurs before device's response
 * @return  -EINVAL When an invalid argument is given
 * @return  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return  -EAGAIN When a lost bus arbitration occurs
 */
int aip31068_turn_on(aip31068_t *dev);

/**
 * Turn off the display.
 *
 * @param[in] dev   Device descriptor of the AIP31068
 *
 * @return  AIP31068_OK on success
 * @return  -AIP31068_ERROR_I2C if acquiring of I2C bus fails
 * @return  -EIO When slave device doesn't ACK the byte
 * @return  -ENXIO When no devices respond on the address sent on the bus
 * @return  -ETIMEDOUT When timeout occurs before device's response
 * @return  -EINVAL When an invalid argument is given
 * @return  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return  -EAGAIN When a lost bus arbitration occurs
 */
int aip31068_turn_off(aip31068_t *dev);

/**
 * Clear the display and set the cursor to position (0, 0).
 * ATTENTION: Also changes to setTextInsertionMode(LEFT_TO_RIGHT)
 *
 * @param[in] dev   Device descriptor of the AIP31068
 *
 * @return  AIP31068_OK on success
 * @return  -AIP31068_ERROR_I2C if acquiring of I2C bus fails
 * @return  -EIO When slave device doesn't ACK the byte
 * @return  -ENXIO When no devices respond on the address sent on the bus
 * @return  -ETIMEDOUT When timeout occurs before device's response
 * @return  -EINVAL When an invalid argument is given
 * @return  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return  -EAGAIN When a lost bus arbitration occurs
 */
int aip31068_clear(aip31068_t *dev);

/**
 * Reset cursor position to (0, 0) and scroll display to original position.
 *
 * @param[in] dev   Device descriptor of the AIP31068
 *
 * @return  AIP31068_OK on success
 * @return  -AIP31068_ERROR_I2C if acquiring of I2C bus fails
 * @return  -EIO When slave device doesn't ACK the byte
 * @return  -ENXIO When no devices respond on the address sent on the bus
 * @return  -ETIMEDOUT When timeout occurs before device's response
 * @return  -EINVAL When an invalid argument is given
 * @return  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return  -EAGAIN When a lost bus arbitration occurs
 */
int aip31068_return_home(aip31068_t *dev);

/**
 * Enable or disable automated scrolling.
 *
 * @param[in] dev       Device descriptor of the AIP31068
 * @param[in] enabled   Enable or disable
 *
 * @return  AIP31068_OK on success
 * @return  -AIP31068_ERROR_I2C if acquiring of I2C bus fails
 * @return  -EIO When slave device doesn't ACK the byte
 * @return  -ENXIO When no devices respond on the address sent on the bus
 * @return  -ETIMEDOUT When timeout occurs before device's response
 * @return  -EINVAL When an invalid argument is given
 * @return  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return  -EAGAIN When a lost bus arbitration occurs
 */
int aip31068_set_auto_scroll_enabled(aip31068_t *dev, bool enabled);

/**
 * Enable or disable cursor blinking.
 *
 * @param[in] dev       Device descriptor of the AIP31068
 * @param[in] enabled   Enable or disable
 *
 * @return  AIP31068_OK on success
 * @return  -AIP31068_ERROR_I2C if acquiring of I2C bus fails
 * @return  -EIO When slave device doesn't ACK the byte
 * @return  -ENXIO When no devices respond on the address sent on the bus
 * @return  -ETIMEDOUT When timeout occurs before device's response
 * @return  -EINVAL When an invalid argument is given
 * @return  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return  -EAGAIN When a lost bus arbitration occurs
 */
int aip31068_set_cursor_blinking_enabled(aip31068_t *dev, bool enabled);

/**
 * Show or hide the cursor.
 *
 * @param[in] dev       Device descriptor of the AIP31068
 * @param[in] visible   Show or hide
 *
 * @return  AIP31068_OK on success
 * @return  -AIP31068_ERROR_I2C if acquiring of I2C bus fails
 * @return  -EIO When slave device doesn't ACK the byte
 * @return  -ENXIO When no devices respond on the address sent on the bus
 * @return  -ETIMEDOUT When timeout occurs before device's response
 * @return  -EINVAL When an invalid argument is given
 * @return  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return  -EAGAIN When a lost bus arbitration occurs
 */
int aip31068_set_cursor_visible(aip31068_t *dev, bool visible);

/**
 * Move the cursor to a given position.
 *
 * @param[in] dev   Device descriptor of the AIP31068
 * @param[in] row   Row of the new cursor position (starting at 0)
 * @param[in] col   Column of the new cursor position (starting at 0)
 *
 * @return  AIP31068_OK on success
 * @return  -AIP31068_ERROR_I2C if acquiring of I2C bus fails
 * @return  -EIO When slave device doesn't ACK the byte
 * @return  -ENXIO When no devices respond on the address sent on the bus
 * @return  -ETIMEDOUT When timeout occurs before device's response
 * @return  -EINVAL When an invalid argument is given
 * @return  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return  -EAGAIN When a lost bus arbitration occurs
 */
int aip31068_set_cursor_position(aip31068_t *dev, uint8_t row, uint8_t col);

/**
 * Set the direction from which the text is inserted, starting from the cursor.
 *
 * @param[in] dev   Device descriptor of the AIP31068
 * @param[in] mode  Insertion mode
 *
 * @return  AIP31068_OK on success
 * @return  -AIP31068_ERROR_I2C if acquiring of I2C bus fails
 * @return  -EIO When slave device doesn't ACK the byte
 * @return  -ENXIO When no devices respond on the address sent on the bus
 * @return  -ETIMEDOUT When timeout occurs before device's response
 * @return  -EINVAL When an invalid argument is given
 * @return  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return  -EAGAIN When a lost bus arbitration occurs
 */
int aip31068_set_text_insertion_mode(aip31068_t *dev,
                                     aip31068_text_insertion_mode_t mode);

/**
 * Move the cursor one unit to the left. When the cursor passes the 40th
 * character of the first line and a second line is available, the cursor
 * will move to the second line.
 *
 * @param[in] dev   Device descriptor of the AIP31068
 *
 * @return  AIP31068_OK on success
 * @return  -AIP31068_ERROR_I2C if acquiring of I2C bus fails
 * @return  -EIO When slave device doesn't ACK the byte
 * @return  -ENXIO When no devices respond on the address sent on the bus
 * @return  -ETIMEDOUT When timeout occurs before device's response
 * @return  -EINVAL When an invalid argument is given
 * @return  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return  -EAGAIN When a lost bus arbitration occurs
 */
int aip31068_move_cursor_left(aip31068_t *dev);

/**
 * Move the cursor one unit to the right. When the cursor passes the 40th
 * character of the first line and a second line is available, the cursor
 * will move to the second line.
 *
 * NOTE: The cursor respects the setting for the insertion mode and is set
 *       to (1, 0) for LEFT_TO_RIGHT and to (1, COL_MAX) for RIGHT_TO_LEFT.
 *
 * @param[in] dev   Device descriptor of the AIP31068
 *
 * @return  AIP31068_OK on success
 * @return  -AIP31068_ERROR_I2C if acquiring of I2C bus fails
 * @return  -EIO When slave device doesn't ACK the byte
 * @return  -ENXIO When no devices respond on the address sent on the bus
 * @return  -ETIMEDOUT When timeout occurs before device's response
 * @return  -EINVAL When an invalid argument is given
 * @return  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return  -EAGAIN When a lost bus arbitration occurs
 */
int aip31068_move_cursor_right(aip31068_t *dev);

/**
 * Scroll the entire display content (all lines) one unit to the left.
 *
 * NOTE: The cursor respects the setting for the insertion mode and is set
 *       to (1, 0) for LEFT_TO_RIGHT and to (1, COL_MAX) for RIGHT_TO_LEFT.
 *
 * @param[in] dev   Device descriptor of the AIP31068
 *
 * @return  AIP31068_OK on success
 * @return  -AIP31068_ERROR_I2C if acquiring of I2C bus fails
 * @return  -EIO When slave device doesn't ACK the byte
 * @return  -ENXIO When no devices respond on the address sent on the bus
 * @return  -ETIMEDOUT When timeout occurs before device's response
 * @return  -EINVAL When an invalid argument is given
 * @return  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return  -EAGAIN When a lost bus arbitration occurs
 */
int aip31068_scroll_display_left(aip31068_t *dev);

/**
 * Scroll the entire display content (all lines) one unit to the right.
 *
 * @param[in] dev   Device descriptor of the AIP31068
 *
 * @return  AIP31068_OK on success
 * @return  -AIP31068_ERROR_I2C if acquiring of I2C bus fails
 * @return  -EIO When slave device doesn't ACK the byte
 * @return  -ENXIO When no devices respond on the address sent on the bus
 * @return  -ETIMEDOUT When timeout occurs before device's response
 * @return  -EINVAL When an invalid argument is given
 * @return  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return  -EAGAIN When a lost bus arbitration occurs
 */
int aip31068_scroll_display_right(aip31068_t *dev);

/*
 * Create a custom symbol.
 * Useful link: https://maxpromer.github.io/LCD-Character-Creator/
 *
 * @param[in] dev           Device descriptor of the AIP31068
 * @param[in] customSymbol  Key to which a custom symbol should be assigned
 * @param[in] charmap       Bitmap definition of the custom symbol
 *
 * @return  AIP31068_OK on success
 * @return  -AIP31068_ERROR_I2C if acquiring of I2C bus fails
 * @return  -EIO When slave device doesn't ACK the byte
 * @return  -ENXIO When no devices respond on the address sent on the bus
 * @return  -ETIMEDOUT When timeout occurs before device's response
 * @return  -EINVAL When an invalid argument is given
 * @return  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return  -EAGAIN When a lost bus arbitration occurs
 * */
int aip31068_set_custom_symbol(aip31068_t *dev,
                               aip31068_custom_symbol_t customSymbol,
                               uint8_t charmap[]);

/*
 * Print a custom symbol by key reference.
 *
 * @param[in] dev           Device descriptor of the AIP31068
 * @param[in] customSymbol  Key of the custom symbol to be printed
 *
 * @return  AIP31068_OK on success
 * @return  -AIP31068_ERROR_I2C if acquiring of I2C bus fails
 * @return  -EIO When slave device doesn't ACK the byte
 * @return  -ENXIO When no devices respond on the address sent on the bus
 * @return  -ETIMEDOUT When timeout occurs before device's response
 * @return  -EINVAL When an invalid argument is given
 * @return  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return  -EAGAIN When a lost bus arbitration occurs
 * */
int aip31068_print_custom_symbol(aip31068_t *dev,
                                 aip31068_custom_symbol_t customSymbol);

/*
 * Print a string.
 *
 * @param[in] dev   Device descriptor of the AIP31068
 * @param[in] data  String to be printed (null-terminated)
 *
 * @return  AIP31068_OK on success
 * @return  -AIP31068_ERROR_I2C if acquiring of I2C bus fails
 * @return  -EIO When slave device doesn't ACK the byte
 * @return  -ENXIO When no devices respond on the address sent on the bus
 * @return  -ETIMEDOUT When timeout occurs before device's response
 * @return  -EINVAL When an invalid argument is given
 * @return  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return  -EAGAIN When a lost bus arbitration occurs
 * */
int aip31068_print(aip31068_t *dev, const char *data);

/*
 * Print a single character.
 *
 * @param[in] dev   Device descriptor of the AIP31068
 * @param[in] c     Character to be printed
 *
 * @return  AIP31068_OK on success
 * @return  -AIP31068_ERROR_I2C if acquiring of I2C bus fails
 * @return  -EIO When slave device doesn't ACK the byte
 * @return  -ENXIO When no devices respond on the address sent on the bus
 * @return  -ETIMEDOUT When timeout occurs before device's response
 * @return  -EINVAL When an invalid argument is given
 * @return  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return  -EAGAIN When a lost bus arbitration occurs
 * */
int aip31068_print_char(aip31068_t *dev, char c);

/*
 * Enable or disable the progress bar. When enabled, the last five custom
 * symbols are reserved to display the progress bar (CUSTOM_SYMBOL_4 to
 * CUSTOM_SYMBOL_8) and can't be used. Assignments via setCustomSymbol() to
 * these keys will be ignored. The given line will be reserved completely
 * for the progress bar. Any text written to that line will be overwritten
 * by the progress bar on an update.
 *
 * NOTE: Auto scroll will be disabled and the display will be scrolled to
 * its original position. Don't use scrolling when using the
 * progressbar, otherwise it won't display correctly.
 *
 * NOTE: Text insertion mode will be set to LEFT_TO_RIGHT.
 *
 * @param[in] dev       Device descriptor of the AIP31068
 * @param[in] enabled   Enable or disable
 * */
int aip31068_set_progress_bar_enabled(aip31068_t *dev, bool enabled);

/*
 * Set the row for displaying the progress bar. Defaults to the last row,
 * according to the given row count in the constructor.
 *
 * @param[in] dev   Device descriptor of the AIP31068
 * @param[in] row   Row where the progress bar is displayed
 * */
void aip31068_set_progress_bar_row(aip31068_t *dev, uint8_t row);

/*
 * Set the progress of the progress bar and draw the update.
 *
 * NOTE: This function changes the cursor position. You will have to use
 * setCursorPosition in order to return to your required cursor position.
 *
 * @param[in] dev       Device descriptor of the AIP31068
 * @param[in] progress  Progress in percentage (0 to 100)
 * */
int aip31068_set_progress(aip31068_t *dev, uint8_t progress);

#ifdef __cplusplus
}
#endif

#endif /* AIP31068_H */
/** @} */
