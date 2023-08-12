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

#ifndef LOG_H
#define LOG_H

	#include <stdlib.h>
	#include <unistd.h>
	#include <SDL3/SDL.h>

	extern void log_msg(int priority,
		SDL_PRINTF_FORMAT_STRING const char *fmt, ...);

	/* Log messages. */
	#define log_oom(msg) \
		do {\
			write(STDOUT_FILENO, "Out of Memory: ", 15); \
			write(STDOUT_FILENO, (msg), strlen(msg)); \
			_exit(1); \
		} while (0)

	#define log_panic(...) \
		do {\
			log_msg(SDL_LOG_PRIORITY_CRITICAL, __VA_ARGS__); \
			exit(1); \
		} while (0)

	#define log_err_to(lbl, ...) \
		do {\
			log_msg(SDL_LOG_PRIORITY_ERROR, __VA_ARGS__); \
			goto lbl; \
		} while (0)

	#define log_info(...) log_msg(SDL_LOG_PRIORITY_INFO, __VA_ARGS__)

#endif /* LOG_H. */
