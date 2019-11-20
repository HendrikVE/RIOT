/*
 * Copyright (C) 2009, Freie Universitaet Berlin (FUB).
 * Copyright (C) 2013, INRIA.
 * Copyright (C) 2015 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_shell
 * @{
 *
 * @file
 * @brief       Implementation of a very simple command interpreter.
 *              For each command (i.e. "echo"), a handler can be specified.
 *              If the first word of a user-entered command line matches the
 *              name of a handler, the handler will be called with the whole
 *              command line as parameter.
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      René Kijewski <rene.kijewski@fu-berlin.de>
 * @author      Hendrik van Essen <hendrik.ve@fu-berlin.de>
 *
 * @}
 */

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "xtimer.h"
#include "thread.h"
#include "shell.h"
#include "shell_commands.h"

#define ETX '\x03'  /** ASCII "End-of-Text", or ctrl-C */
#if !defined(SHELL_NO_ECHO) || !defined(SHELL_NO_PROMPT)
    #ifdef MODULE_NEWLIB
        /* use local copy of putchar, as it seems to be inlined,
         * enlarging code by 50% */
        static void _putchar(int c) {
            putchar(c);
        }
    #else
        #define _putchar putchar
    #endif
#endif

#ifdef MODULE_SHELL_LOCK
bool shell_is_locked = true;
#endif

#ifdef MODULE_SHELL_LOCK_AUTO_LOCKING
uint64_t _timestamp_shell_auto_lock_ms = 0;
#endif

static void flush_if_needed(void)
{
    #ifdef MODULE_NEWLIB
    fflush(stdout);
    #endif
}

#ifdef MODULE_SHELL_LOCK_AUTO_LOCKING
char _auto_shell_lock_thread_stack[THREAD_STACKSIZE_MAIN];

void *_auto_shell_lock_thread(void *arg)
{
    (void) arg;

    while (1) {

        uint32_t time_to_sleep_ms;

        if (!shell_is_locked) {
            uint64_t time_now_ms = xtimer_now_usec64() / 1000;

            if (time_now_ms >= _timestamp_shell_auto_lock_ms) {
                shell_is_locked = true;
                break;
            }
            else {
                time_to_sleep_ms = (_timestamp_shell_auto_lock_ms - time_now_ms)
                        + TIMER_SLEEP_OFFSET_MS;
            }
        }
        else {
            time_to_sleep_ms = MAX_AUTO_LOCK_PAUSE_MS;
        }

        xtimer_usleep(time_to_sleep_ms * 1000);
    }

    return NULL;
}
#endif /* MODULE_SHELL_LOCK_AUTO_LOCKING */

static shell_command_handler_t find_handler(
        const shell_command_t **command_lists, int list_count, char *command)
{
    /* iterating over command_lists */
    for (int i = 0; i < list_count; i++) {

        const shell_command_t *entry;

        if ((entry = command_lists[i])) {
            /* iterating over commands in command_lists entry */
            while (entry->name != NULL) {
                if (strcmp(entry->name, command) == 0) {
                    return entry->handler;
                }
                else {
                    entry++;
                }
            }
        }
    }

    return NULL;
}

static void print_help(const shell_command_t **command_lists, int list_count)
{
    printf("%-20s %s\n", "Command", "Description");
    puts("---------------------------------------");

    /* iterating over command_lists */
    for (int i = 0; i < list_count; i++) {

        const shell_command_t *entry;

        if ((entry = command_lists[i])) {
            /* iterating over commands in command_lists entry */
            while (entry->name != NULL) {
                printf("%-20s %s\n", entry->name, entry->desc);
                entry++;
            }
        }
    }
}

static void handle_input_line(
        const shell_command_t **command_list, int list_count, char *line)
{
    static const char *INCORRECT_QUOTING = "shell: incorrect quoting";

    /* first we need to calculate the number of arguments */
    unsigned argc = 0;
    char *pos = line;
    int contains_esc_seq = 0;
    while (1) {
        if ((unsigned char) *pos > ' ') {
            /* found an argument */
            if (*pos == '"' || *pos == '\'') {
                /* it's a quoted argument */
                const char quote_char = *pos;
                do {
                    ++pos;
                    if (!*pos) {
                        puts(INCORRECT_QUOTING);
                        return;
                    }
                    else if (*pos == '\\') {
                        /* skip over the next character */
                        ++contains_esc_seq;
                        ++pos;
                        if (!*pos) {
                            puts(INCORRECT_QUOTING);
                            return;
                        }
                        continue;
                    }
                } while (*pos != quote_char);
                if ((unsigned char) pos[1] > ' ') {
                    puts(INCORRECT_QUOTING);
                    return;
                }
            }
            else {
                /* it's an unquoted argument */
                do {
                    if (*pos == '\\') {
                        /* skip over the next character */
                        ++contains_esc_seq;
                        ++pos;
                        if (!*pos) {
                            puts(INCORRECT_QUOTING);
                            return;
                        }
                    }
                    ++pos;
                    if (*pos == '"') {
                        puts(INCORRECT_QUOTING);
                        return;
                    }
                } while ((unsigned char) *pos > ' ');
            }

            /* count the number of arguments we got */
            ++argc;
        }

        /* zero out the current position (space or quotation mark) and advance */
        if (*pos > 0) {
            *pos = 0;
            ++pos;
        }
        else {
            break;
        }
    }
    if (!argc) {
        return;
    }

    /* then we fill the argv array */
    char *argv[argc + 1];
    argv[argc] = NULL;
    pos = line;
    for (unsigned i = 0; i < argc; ++i) {
        while (!*pos) {
            ++pos;
        }
        if (*pos == '"' || *pos == '\'') {
            ++pos;
        }
        argv[i] = pos;
        while (*pos) {
            ++pos;
        }
    }
    for (char **arg = argv; contains_esc_seq && *arg; ++arg) {
        for (char *c = *arg; *c; ++c) {
            if (*c != '\\') {
                continue;
            }
            for (char *d = c; *d; ++d) {
                *d = d[1];
            }
            if (--contains_esc_seq == 0) {
                break;
            }
        }
    }

    /* then we call the appropriate handler */
    shell_command_handler_t handler = find_handler(command_list, list_count, argv[0]);
    if (handler != NULL) {
        handler(argc, argv);
    }
    else {
        if (strcmp("help", argv[0]) == 0) {
            print_help(command_list, list_count);
        }
        else {
            printf("shell: command not found: %s\n", argv[0]);
        }
    }
}

static int readline(char *buf, size_t size)
{
    char *line_buf_ptr = buf;

    while (1) {
        if ((line_buf_ptr - buf) >= ((int) size) - 1) {
            return -1;
        }

        int c = getchar();
        if (c < 0) {
            return EOF;
        }

        /* We allow Unix linebreaks (\n), DOS linebreaks (\r\n), and Mac linebreaks (\r). */
        /* QEMU transmits only a single '\r' == 13 on hitting enter ("-serial stdio"). */
        /* DOS newlines are handled like hitting enter twice, but empty lines are ignored. */
        /* Ctrl-C cancels the current line. */
        if (c == '\r' || c == '\n' || c == ETX) {
            *line_buf_ptr = '\0';

            #ifndef SHELL_NO_ECHO
            _putchar('\r');
            _putchar('\n');
            #endif

            /* return 1 if line is empty, 0 otherwise */
            return c == ETX || line_buf_ptr == buf;
        }
        /* QEMU uses 0x7f (DEL) as backspace, while 0x08 (BS) is for most terminals */
        else if (c == 0x08 || c == 0x7f) {
            if (line_buf_ptr == buf) {
                /* The line is empty. */
                continue;
            }

            *--line_buf_ptr = '\0';
            /* white-tape the character */
            #ifndef SHELL_NO_ECHO
            _putchar('\b');
            _putchar(' ');
            _putchar('\b');
            #endif
        }
        else {
            *line_buf_ptr++ = c;

            #ifndef SHELL_NO_ECHO
            _putchar(c);
            #endif
        }
        flush_if_needed();
    }
}

static inline void print_prompt(void)
{
    #ifndef SHELL_NO_PROMPT
    _putchar('>');
    _putchar(' ');
    #endif

    flush_if_needed();
}

#ifdef MODULE_SHELL_LOCK

int _lock_handler(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    shell_is_locked = true;

    return 0;
}

const shell_command_t _shell_lock_command_list[] = {
        {"lock", "lock the shell", _lock_handler},
        {NULL, NULL, NULL}
};

static bool is_line_delim(char c)
{
    return c == '\r' || c == '\n';
}

static bool is_line_cancel(char c)
{
    return c == 0x03 || c == 0x04;
}

enum LINE_TAINT {
    LINE_OK = 0,
    LINE_LONG = 1,
    LINE_OK_DONE = 2,
    LINE_LONG_DONE = 3,
    LINE_CANCELLED = 4,
};

#define MARK_DONE(t) ((t) | LINE_OK_DONE)
#define IS_DONE(t) ((t) & LINE_OK_DONE)

/**
 * Get a line of input.
 *
 * The line is read to line_buf. EOF, ctrl-c, ctrl-d cancel the input
 * and (-LINE_CANCELLED) is returned.
 * Otherwise, return the number of characters read or, if the buffer size was
 * exceeded, (-LINE_LONG).
 *
 * At most buf_size-1 characters will be stored, and the last character will
 * always be a terminator.
 *
 * Even if the buffer size is exceeded, characters will continue to be read
 * from the input.
 */
static int my_gets(char *line_buf, size_t buf_size)
{
    size_t length = 0;
    enum LINE_TAINT state = LINE_OK;

    do {
        int c = getchar();

        if (c == EOF || is_line_cancel(c)) {
            state = LINE_CANCELLED;
        }
        else if (is_line_delim(c)) {
            state = MARK_DONE(state);
        }
        else {
            if (length + 1 < buf_size) {
                line_buf[length++] = c;
            }
            else {
                state = LINE_LONG;
            }
        }
    } while (state != LINE_CANCELLED && !IS_DONE(state));

    line_buf[length++] = '\0';

    return state == LINE_OK_DONE ? (int) length : (int) -state;
}

static bool login(char *line_buf, size_t buf_size)
{
    int read_len;
    int success = false;

    printf("Password: \n");
    flush_if_needed();

    print_prompt();
    read_len = my_gets(line_buf, buf_size);

    if (read_len != -LINE_CANCELLED && read_len > 0) {
        if (strcmp(line_buf, SHELL_LOCK_PASSWORD) == 0) {
            success = true;
        }
    }

    return success;
}

/**
 * Repeatedly prompt for the password.
 *
 * This function won't return until the correct password has been
 * introduced.
 */
void login_barrier(char *line_buf, size_t buf_size)
{
    while (1) {
        int attempts = ATTEMPTS_BEFORE_TIME_LOCK;

        while (attempts--) {
            if (login(line_buf, buf_size)) {
                return;
            }
            puts("Wrong password");
            xtimer_sleep(1);
        }
        xtimer_sleep(7);
    }
}
#endif /* MODULE_SHELL_LOCK */

#ifdef MODULE_SHELL_LOCK_AUTO_LOCKING
void reset_shell_auto_lock(void)
{
    uint64_t time_now_ms = xtimer_now_usec64() / 1000;

    _timestamp_shell_auto_lock_ms = time_now_ms + MAX_AUTO_LOCK_PAUSE_MS;
}
#endif

void shell_run_once(const shell_command_t *shell_commands, char *line_buf, int len)
{
    #ifdef MODULE_SHELL_LOCK
    if (shell_is_locked) {
        printf("The shell is locked. Enter a valid password to unlock.\n\n");

        login_barrier(line_buf, SHELL_DEFAULT_BUFSIZE);

        #ifndef MODULE_SHELL_LOCK_AUTO_LOCKING
        printf("Shell was unlocked.\n\n"
               "IMPORTANT: Don't forget to lock the shell after usage, "
               "because it won't lock itself.\n\n");
        #else
        printf("Shell was unlocked.\n\n");
        #endif

        shell_is_locked = false;
    }

    #ifdef MODULE_SHELL_LOCK_AUTO_LOCKING
    reset_shell_auto_lock();

    thread_create(_auto_shell_lock_thread_stack, sizeof(_auto_shell_lock_thread_stack),
              THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
              _auto_shell_lock_thread, NULL, "_auto_shell_lock_thread");
    #endif /* MODULE_SHELL_LOCK_AUTO_LOCKING */

    #endif /* MODULE_SHELL_LOCK */

    const shell_command_t *command_lists[] = {
            shell_commands,

            #ifdef MODULE_SHELL_LOCK
            _shell_lock_command_list,
            #endif

            #ifdef MODULE_SHELL_COMMANDS
            _shell_command_list,
            #endif
    };

    print_prompt();

    while (1) {
        int res = readline(line_buf, len);

        if (res == EOF) {
            break;
        }

        #ifdef MODULE_SHELL_LOCK
        if (shell_is_locked) {
            break;
        }

        #ifdef MODULE_SHELL_LOCK_AUTO_LOCKING
        /* reset lock countdown in case of new input */
        reset_shell_auto_lock();
        #endif /* MODULE_SHELL_LOCK_AUTO_LOCKING */

        #endif /* MODULE_SHELL_LOCK */

        if (!res) {
            handle_input_line(command_lists, ARRAY_SIZE(command_lists), line_buf);
        }

        print_prompt();
    }
}
