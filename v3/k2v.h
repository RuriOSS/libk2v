// SPDX-License-Identifier: MIT
/*
 *
 * This file is part of libk2v, with ABSOLUTELY NO WARRANTY.
 *
 * MIT License
 *
 * Copyright (c) 2026 Moe-hacker
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
 *
 *
 */

/*
 * WARNING:
 * NO ANY FALLBACKS, JUST PANIC ON ANY ERROR.
 * USERS SHOULD BE RESPONSIBLE FOR THE INPUT.
 * SECURITY IS THE MOST IMPORTANT THING, SO WE DO NOT ALLOW ANY UNEXPECTED INPUTS.
 */
#pragma once
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
// Bool!!!
#if __STDC_VERSION__ < 202000L
#ifndef bool
#define bool _Bool
#define true ((_Bool) + 1u)
#define false ((_Bool) + 0u)
#endif
#endif
#ifndef _Nullable
#define _Nullable
#endif
#ifndef _Nonnull
#define _Nonnull
#endif
// Version info.
#define LIBK2V_MAJOR 3
#define LIBK2V_MINOR 0
enum K2V_TYPE {
	K2V_SCALAR,
	K2V_ARRAY,
};
struct K2V_BUF {
	enum K2V_TYPE type;
	char *key;
	char *scalar_val;
	size_t array_len;
	char **array_val;
	bool empty;
	struct K2V_BUF *next;
};
typedef struct K2V_BUF *k2v_cache;

#define k2v_error(...)                                                                              \
	do {                                                                                        \
		fprintf(stderr, "[libk2v]: in %s() at %s line %d :", __func__, __FILE__, __LINE__); \
		fprintf(stderr, __VA_ARGS__);                                                       \
		fprintf(stderr, "\n");                                                              \
		exit(114);                                                                          \
	} while (0)

k2v_cache k2v_parse(char *const _Nonnull buf);
void k2v_free_cache(k2v_cache *cache);
char *k2v_open_file(char *const _Nonnull path);
// For debugging.
void k2v_dump(k2v_cache cache);