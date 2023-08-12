/*
 * MIT License
 *
 * Copyright (c) 2023 Davidson Francis <davidsondfgl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "log.h"

/**
 * @brief Get the current timestamp (HH:MM:SS)
 * as a constant string.
 *
 * If success, returns the timestamp, otherwise,
 * NULL.
 */
static const char *get_timestamp(void)
{
    time_t now;
    struct tm *now_tm;
    static char buffer[64];

    now    = time(NULL);
    now_tm = localtime(&now);

    SDL_memset(buffer, 0, sizeof(buffer));
    if (!strftime(buffer, sizeof(buffer), "%X", now_tm))
    	return (NULL);

    return (buffer);
}

/**
 * @brief Logs a given message using the SDL logging
 * routines but printing the timestamp too.
 *
 * @param priority SDL priority message type.
 * @param fmt      String format.
 */
void log_msg(int priority,
	SDL_PRINTF_FORMAT_STRING const char *fmt, ...)
{
    va_list list;
    char log[2048];

    /* Print log message into a buffer */
    SDL_memset(log, 0, sizeof log);
    va_start(list, fmt);
    (void)SDL_vsnprintf(log, sizeof(log) - 1, fmt, list);
    va_end(list);

	/* Log with timestamp and newline */
    SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION,
    	priority, "(%s) %s", get_timestamp(), log);
}
