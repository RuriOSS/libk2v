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
#pragma once
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <threads.h>
#include <setjmp.h>
#include <sys/stat.h>
// Bool!!!
#if __STDC_VERSION__ < 202000L
#ifndef bool
#define bool _Bool
#define true ((_Bool)1u)
#define false ((_Bool)0u)
#endif
#endif
#ifndef _Nullable
#define _Nullable
#endif
#ifndef _Nonnull
#define _Nonnull
#endif
// Version info.
#define LIBK2V3_MAJOR 3
#define LIBK2V3_MINOR 0
enum K2V3_TYPE {
	K2V3_SCALAR,
	K2V3_ARRAY,
	K2V3_ANY,
};
struct K2V3_BUF {
	enum K2V3_TYPE type;
	char *key;
	char *scalar_val;
	size_t array_len;
	char **array_val;
	bool empty;
	struct K2V3_BUF *next;
};
typedef struct K2V3_BUF *k2v3_cache;
enum K2V3_ERRNO {
	K2V3_QUERY,
	K2V3_NORMAL,
	K2V3_RECOVERABLE,
	K2V3_UNRECOVERABLE,
};
// -1 for query, 0 for false, 1 for true.
bool k2v3_stop_at_warning(int req);
// -1 for query, 0 for false, 1 for true.
bool k2v3_show_warning(int req);
// Follow enum definitions in k2v3_errno.
enum K2V3_ERRNO k2v3_errno(enum K2V3_ERRNO req);
// Warning: when this happen, this lib has fully internal error and you should panic yourself.
extern thread_local jmp_buf k2v3_jmp;
#define k2v3_error(...)                                                                             \
	do {                                                                                        \
		k2v3_errno(K2V3_UNRECOVERABLE);                                                     \
		fprintf(stderr, "[libk2v]: in %s() at %s line %d :", __func__, __FILE__, __LINE__); \
		fprintf(stderr, __VA_ARGS__);                                                       \
		fprintf(stderr, "\n");                                                              \
		if (k2v3_stop_at_warning(-1)) {                                                     \
			fprintf(stderr, "[libk2v]: k2v_stop_at_warning set, exit\n");               \
			exit(114);                                                                  \
		}                                                                                   \
		longjmp(k2v3_jmp, 1);                                                               \
	} while (0)
#define k2v3_warning(...)                                                                                   \
	do {                                                                                                \
		k2v3_errno(K2V3_RECOVERABLE);                                                               \
		if (k2v3_show_warning(-1) || k2v3_stop_at_warning(-1)) {                                    \
			fprintf(stderr, "[libk2v]: in %s() at %s line %d :", __func__, __FILE__, __LINE__); \
			fprintf(stderr, __VA_ARGS__);                                                       \
			fprintf(stderr, "\n");                                                              \
		}                                                                                           \
		if (k2v3_stop_at_warning(-1)) {                                                             \
			fprintf(stderr, "[libk2v]: k2v_stop_at_warning set, exit\n");                       \
			exit(114);                                                                          \
		}                                                                                           \
	} while (0)
#ifdef K2V3_FUZZ
#undef k2v3_warning
#define k2v3_warning(...) \
	do {              \
	} while (0)
#endif
k2v3_cache k2v3_parse(char *const _Nonnull buf);
void k2v3_free_cache(k2v3_cache *cache);
char *k2v3_open_file(const char *_Nonnull path, off_t limit);
int k2v3_have_key(k2v3_cache cache, const char *_Nonnull key, enum K2V3_TYPE type);
char *k2v3_get_char(const char *_Nonnull key, k2v3_cache cache);
int k2v3_get_int(const char *_Nonnull key, k2v3_cache cache);
float k2v3_get_float(const char *_Nonnull key, k2v3_cache cache);
bool k2v3_get_bool(const char *_Nonnull key, k2v3_cache cache);
long long k2v3_get_long(const char *_Nonnull key, k2v3_cache cache);
int k2v3_get_int_array(const char *_Nonnull key, k2v3_cache cache, int *_Nonnull array, int limit);
int k2v3_get_char_array(const char *_Nonnull key, k2v3_cache cache, char *_Nonnull array[], int limit);
int k2v3_get_float_array(const char *_Nonnull key, k2v3_cache cache, float *_Nonnull array, int limit);
int k2v3_get_long_array(const char *_Nonnull key, k2v3_cache cache, long long *_Nonnull array, int limit);
#define k2v3_get(type, ...) k2v3_get_##type(__VA_ARGS__)
// For debugging.
void k2v3_dump(k2v3_cache cache);
// Generation functions.
char *k2v3_add_comment(char *_Nullable buf, char *_Nonnull comment);
char *k2v3_add_newline(char *_Nullable buf);
char *char_to_k2v3(const char *_Nonnull key, const char *val);
char *int_to_k2v3(const char *_Nonnull key, int val);
char *bool_to_k2v3(const char *_Nonnull key, bool val);
char *float_to_k2v3(const char *_Nonnull key, float val);
char *long_to_k2v3(const char *_Nonnull key, long long val);
char *char_array_to_k2v3(const char *_Nonnull key, char *const *_Nonnull val, int len);
char *int_array_to_k2v3(const char *_Nonnull key, int *_Nonnull val, int len);
char *float_array_to_k2v3(const char *_Nonnull key, float *_Nonnull val, int len);
char *long_array_to_k2v3(const char *_Nonnull key, long long *_Nonnull val, int len);
char *k2v3_add_config_func(char *_Nullable buf, char *_Nonnull tmp);
#define k2v3_add_config(type, __k2v3_buf, ...) k2v3_add_config_func(__k2v3_buf, type##_to_k2v3(__VA_ARGS__))