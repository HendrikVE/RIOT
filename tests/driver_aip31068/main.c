/*
 * Copyright (C) 2020 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @brief       Test application for the AIP31068 I2C LCD controller
 * @author      Hendrik van Essen <hendrik.ve@fu-berlin.de>
 * @file
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xtimer.h"
#include "shell.h"

#include "aip31068.h"
#include "aip31068_params.h"

#define ROW_COUNT   2
#define COL_COUNT   16

aip31068_t aip31068_dev;

uint8_t custom_char_heart[] = { 0x0, 0x0, 0xA, 0x1F, 0x1F, 0xE, 0x4, 0x0 };

int turn_on(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    aip31068_turn_on(&aip31068_dev);
    return 0;
}

int turn_off(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    aip31068_turn_off(&aip31068_dev);
    return 0;
}

int clear(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    aip31068_clear(&aip31068_dev);
    return 0;
}

int home(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    aip31068_return_home(&aip31068_dev);
    return 0;
}

int autoscroll(int argc, char **argv)
{
    if (argc != 2) {
        puts("usage: autoscroll <0 or 1>");
    }
    else {
        uint8_t enabled = atoi(argv[1]);
        aip31068_set_auto_scroll_enabled(&aip31068_dev, enabled);
    }

    return 0;
}

int cursor_blinking(int argc, char **argv)
{
    if (argc != 2) {
        puts("usage: cursor_blinking <0 or 1>");
    }
    else {
        uint8_t enabled = atoi(argv[1]);
        aip31068_set_cursor_blinking_enabled(&aip31068_dev, enabled);
    }

    return 0;
}

int cursor_visible(int argc, char **argv)
{
    if (argc != 2) {
        puts("usage: cursor_visible <0 or 1>");
    }
    else {
        uint8_t enabled = atoi(argv[1]);
        aip31068_set_cursor_visible(&aip31068_dev, enabled);
    }

    return 0;
}

int cursor_position(int argc, char **argv)
{
    if (argc != 3) {
        puts("usage: cursor_position <row (e.g. 0 for first row)> <column (e.g. 0 for first column)>");
    }
    else {
        uint8_t row = atoi(argv[1]);
        uint8_t column = atoi(argv[2]);

        aip31068_set_cursor_position(&aip31068_dev, row, column);
    }

    return 0;
}

int text_insertion(int argc, char **argv)
{
    if (argc != 2) {
        puts("usage: text_insertion <mode (0-1)>");

        puts("  mode 0: LEFT_TO_RIGHT");
        puts("  mode 1: RIGHT_TO_LEFT");
    }
    else {
        aip31068_text_insertion_mode_t mode;
        switch (atoi(argv[1])) {
            case 0:
                mode = LEFT_TO_RIGHT;
                break;
            case 1:
                mode = RIGHT_TO_LEFT;
                break;
            default:
                puts("mode needs to be one of [0-1]");
                puts("  mode 0: LEFT_TO_RIGHT");
                puts("  mode 1: RIGHT_TO_LEFT");
                return -1;
        }

        aip31068_set_text_insertion_mode(&aip31068_dev, mode);
    }

    return 0;
}

int cursor_left(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    aip31068_move_cursor_left(&aip31068_dev);

    return 0;
}

int cursor_right(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    aip31068_move_cursor_right(&aip31068_dev);

    return 0;
}

int scroll_left(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    aip31068_scroll_display_left(&aip31068_dev);
    return 0;
}

int scroll_right(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    aip31068_scroll_display_right(&aip31068_dev);
    return 0;
}

int create_custom_symbol(int argc, char **argv)
{
    if (argc != 10) {
        puts("usage: create_custom_symbol <symbol (0-7)> <row 0 (0-31)> ... <row 7 (0-31)>");
        puts("       row values in decimal (e.g. 31 for a full row)");
        puts("       heart example: 0 0 10 31 31 14 4 0");
    }
    else {
        uint8_t symbol = atoi(argv[1]);

        uint8_t row0 = atoi(argv[2]);
        uint8_t row1 = atoi(argv[3]);
        uint8_t row2 = atoi(argv[4]);
        uint8_t row3 = atoi(argv[5]);
        uint8_t row4 = atoi(argv[6]);
        uint8_t row5 = atoi(argv[7]);
        uint8_t row6 = atoi(argv[8]);
        uint8_t row7 = atoi(argv[9]);

        if (symbol > 7) {
            puts("custom symbol needs to be one of [0-7]");
            return -1;
        }

        uint8_t charmap[] = { row0, row1, row2, row3, row4, row5, row6, row7 };
        aip31068_set_custom_symbol(&aip31068_dev, symbol, charmap);
    }

    return 0;
}

int print_custom_symbol(int argc, char **argv)
{
    if (argc != 2) {
        puts("usage: print_custom_symbol <symbol (0-7)>");
    }
    else {
        uint8_t symbol = atoi(argv[1]);

        if (symbol > 7) {
            puts("custom symbol needs to be one of [0-7]");
            return -1;
        }

        aip31068_print_custom_symbol(&aip31068_dev, symbol);
    }

    return 0;
}

int print(int argc, char **argv)
{
    if (argc != 2) {
        puts("usage: print <text>");
    }
    else {
        aip31068_print(&aip31068_dev, argv[1]);
    }

    return 0;
}

int progressbar(int argc, char **argv)
{
    if (argc != 2) {
        puts("usage: progressbar <0 or 1>");
    }
    else {
        uint8_t enabled = atoi(argv[1]);
        aip31068_set_progress_bar_enabled(&aip31068_dev, enabled);
    }

    return 0;
}

int progressbar_row(int argc, char **argv)
{
    if (argc != 2) {
        puts("usage: progressbar_row <row (e.g. 0 for first row)>");
    }
    else {
        uint8_t row = atoi(argv[1]);
        aip31068_set_progress_bar_row(&aip31068_dev, row);
    }

    return 0;
}

int progress(int argc, char **argv)
{
    if (argc != 2) {
        puts("usage: progress <progress (0-100)>");
    }
    else {
        uint8_t progress = atoi(argv[1]);
        aip31068_set_progress(&aip31068_dev, progress);
    }

    return 0;
}

int run_demo(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    puts("[DEMO START]");

    /* 1. custom symbol */
    aip31068_print(&aip31068_dev, "Hello world! ");
    aip31068_print_custom_symbol(&aip31068_dev, CUSTOM_SYMBOL_1);

    xtimer_sleep(1);

    /* 2.a scroll right with text insertion following LEFT_TO_RIGHT */
    aip31068_clear(&aip31068_dev);
    aip31068_set_text_insertion_mode(&aip31068_dev, LEFT_TO_RIGHT);
    aip31068_print(&aip31068_dev, "scroll right");
    xtimer_usleep(500 * US_PER_MS);

    for (int i = 0; i < 4; i++) {
        aip31068_scroll_display_right(&aip31068_dev);
        xtimer_usleep(500 * US_PER_MS);
    }

    /* 2.b scroll left with text insertion following RIGHT_TO_LEFT */
    aip31068_clear(&aip31068_dev);
    aip31068_set_text_insertion_mode(&aip31068_dev, RIGHT_TO_LEFT);
    aip31068_set_cursor_position(&aip31068_dev, 0, COL_COUNT - 1);
    aip31068_print(&aip31068_dev, "tfel llorcs");
    xtimer_usleep(500 * US_PER_MS);

    for (int i = 0; i < 5; i++) {
        aip31068_scroll_display_left(&aip31068_dev);
        xtimer_usleep(500 * US_PER_MS);
    }
    aip31068_set_text_insertion_mode(&aip31068_dev, LEFT_TO_RIGHT);

    /* 3. turning on/off display */
    aip31068_clear(&aip31068_dev);
    aip31068_print(&aip31068_dev, "turning off...");
    xtimer_sleep(1);
    aip31068_turn_off(&aip31068_dev);
    xtimer_sleep(1);

    aip31068_clear(&aip31068_dev);
    aip31068_print(&aip31068_dev, "turned on again");
    aip31068_turn_on(&aip31068_dev);
    xtimer_sleep(1);

    /* 4. autoscroll */
    aip31068_clear(&aip31068_dev);

    if (ROW_COUNT > 1) {
        aip31068_set_cursor_position(&aip31068_dev, 1, 0);
        aip31068_print(&aip31068_dev, "This is also a very long line");
    }

    aip31068_set_cursor_position(&aip31068_dev, 0, COL_COUNT);
    aip31068_set_auto_scroll_enabled(&aip31068_dev, true);

    char long_line[] = "This is a very long line";
    for (int i = 0; i < (int) strlen(long_line); i++) {
        aip31068_print_char(&aip31068_dev, long_line[i]);
        xtimer_usleep(250 * US_PER_MS);
    }
    aip31068_set_auto_scroll_enabled(&aip31068_dev, false);
    xtimer_sleep(2);

    /* 5. return home */
    aip31068_return_home(&aip31068_dev);
    xtimer_sleep(1);

    /* 6. cursor blinking */
    aip31068_clear(&aip31068_dev);
    aip31068_set_cursor_blinking_enabled(&aip31068_dev, true);
    xtimer_sleep(2);

    for (int i = 0; i < COL_COUNT; i++) {

        if (i == (int) (COL_COUNT / 2)) {
            aip31068_set_cursor_blinking_enabled(&aip31068_dev, false);
        }

        if (i == (int) (COL_COUNT / 2) + 2) {
            aip31068_set_cursor_blinking_enabled(&aip31068_dev, true);
        }

        aip31068_set_cursor_position(&aip31068_dev, 0, i);
        xtimer_usleep(500 * US_PER_MS);
    }
    xtimer_sleep(1);
    aip31068_return_home(&aip31068_dev);
    xtimer_usleep(500 * US_PER_MS);
    aip31068_set_cursor_blinking_enabled(&aip31068_dev, false);

    /* 7. show and move cursor */
    aip31068_set_cursor_visible(&aip31068_dev, true);
    aip31068_clear(&aip31068_dev);
    xtimer_sleep(1);
    for (int i = 0; i <= 9; i++) {
        aip31068_print_char(&aip31068_dev, '0' + i);
        xtimer_usleep(250 * US_PER_MS);
    }

    aip31068_return_home(&aip31068_dev);
    xtimer_usleep(100 * US_PER_MS);
    for(int i = 0; i < 50; i++) {
        aip31068_move_cursor_right(&aip31068_dev);
        xtimer_usleep(100 * US_PER_MS);
    }
    for(int i = 50; i > 0 ; i--) {
        aip31068_move_cursor_left(&aip31068_dev);
        xtimer_usleep(100 * US_PER_MS);
    }

    aip31068_set_cursor_visible(&aip31068_dev, false);
    aip31068_clear(&aip31068_dev);

    /* 8. write 10 characters on the next line (if available), because 40
     * characters per line is the maximum */
    if (ROW_COUNT > 1) {
        aip31068_clear(&aip31068_dev);
        aip31068_set_text_insertion_mode(&aip31068_dev, LEFT_TO_RIGHT);
        for (int i = 0; i < 50; i++) {
            if (i % 2 == 0) {
                aip31068_print(&aip31068_dev, "A");
            }
            else {
                aip31068_print(&aip31068_dev, "B");
            }

            xtimer_usleep(100 * US_PER_MS);
        }

        aip31068_clear(&aip31068_dev);
        aip31068_set_text_insertion_mode(&aip31068_dev, RIGHT_TO_LEFT);
        aip31068_set_cursor_position(&aip31068_dev, 0, COL_COUNT - 1);
        for (int i = 0; i < 50; i++) {
            if (i % 2 == 0) {
                aip31068_print(&aip31068_dev, "X");
            }
            else {
                aip31068_print(&aip31068_dev, "Y");
            }

            xtimer_usleep(100 * US_PER_MS);
        }
    }

    aip31068_clear(&aip31068_dev);

    /* 9. progress bar */
    aip31068_set_progress_bar_enabled(&aip31068_dev, true);
    aip31068_set_cursor_position(&aip31068_dev, 0, 0);
    aip31068_print(&aip31068_dev, "Progress: ");

    for (int j = 0; j <= 100; j++) {

        aip31068_set_cursor_position(&aip31068_dev, 0, 10);

        char string_rep[6];
        sprintf(string_rep, "%d %%", j);

        aip31068_print(&aip31068_dev, string_rep);

        aip31068_set_progress(&aip31068_dev, j);
        xtimer_usleep(100 * US_PER_MS);
    }
    aip31068_set_progress_bar_enabled(&aip31068_dev, false);
    aip31068_clear(&aip31068_dev);

    puts("[DEMO END]");

    return 0;
}

static const shell_command_t shell_commands[] = {
    { "turn_on", "Turn on LCD.", turn_on },
    { "turn_off", "Turn off LCD.", turn_off },
    { "clear", "Clear display content.", clear },
    { "home", "Return cursor to home position (0, 0).", home },
    { "autoscroll", "Enable or disable autoscroll.", autoscroll },
    { "cursor_blinking", "Enable or disable cursor blinking.", cursor_blinking },
    { "cursor_visible", "Show or hide the cursor.", cursor_visible },
    { "cursor_position", "Set cursor position.", cursor_position },
    { "text_insertion", "Set text insertion mode.", text_insertion },
    { "cursor_left", "Move cursor one to the left.", cursor_left },
    { "cursor_right", "Move cursor one to the right.", cursor_right },
    { "scroll_left", "Scroll display content one to the left.", scroll_left },
    { "scroll_right", "Scroll display content one to the right.", scroll_right },
    { "create_custom_symbol", "Create a custom symbol.", create_custom_symbol },
    { "print_custom_symbol", "Print a custom symbol.", print_custom_symbol },
    { "print", "Print a string.", print },
    { "progressbar", "Enable/ disable progressbar feature.", progressbar },
    { "progressbar_row", "Set row for progressbar.", progressbar_row },
    { "progress", "Set progress for progressbar.", progress },
    { "run_demo", "Demonstration of all functions.", run_demo },
    { NULL, NULL, NULL }
};

int main(void)
{
    int rc = 0;
    if ((rc = aip31068_init(&aip31068_dev, &aip31068_params[0])) != AIP31068_OK) {
        printf("Initialization failed! rc = %d", rc);
        return 1;
    }

    aip31068_turn_on(&aip31068_dev);
    aip31068_set_custom_symbol(&aip31068_dev, CUSTOM_SYMBOL_1, custom_char_heart);

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
