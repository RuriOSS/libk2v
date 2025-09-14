#pragma once
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <linux/seccomp.h>
// Bool!!!
#if __STDC_VERSION__ < 202000L
#ifndef bool
#define bool _Bool
#define true ((_Bool) + 1u)
#define false ((_Bool) + 0u)
#endif
#endif
// Nullability.
#ifndef _Nullable
#define _Nullable
#endif
#ifndef _Nonnull
#define _Nonnull
#endif
// If this is true, we will exit() when we meet a warning.
extern bool k2v_stop_at_warning;
// If this is true, we will show warning.
extern bool k2v_show_warning;
// Version info.
#define LIBK2V_MAJOR 3
#define LIBK2V_MINOR 0
// For k2v_get_key_value().
typedef enum { K2V_TYPE_CHAR = 0, K2V_TYPE_INT, K2V_TYPE_BOOL, K2V_TYPE_FLOAT, K2V_TYPE_LONG, K2V_TYPE_CHAR_ARRAY, K2V_TYPE_INT_ARRAY, K2V_TYPE_FLOAT_ARRAY, K2V_TYPE_LONG_ARRAY, K2V_TYPE_COMMENT, K2V_TYPE_NEWLINE } k2v_value_type_t;
// For k2v_key_value_t.
typedef enum { K2V_DATA_TYPE_SCALAR = 0, K2V_DATA_TYPE_ARRAY } k2v_data_type_t;
typedef union {
	char **array;
	char *scalar;
} k2v_value_t;
typedef struct {
	char *key;
	k2v_value_t data;
	k2v_data_type_t type;
} k2v_key_value_t;
typedef k2v_key_value_t **k2v_buf_t;
/*
 * This function will free the buffer,
 * please always call this function after you finished all k2v_get_key_value().
 */
void k2v_free(k2v_buf_t _Nonnull kv);
/*
 * This function is used to deserialize the config file.
 * As C does not support generic types,
 * you need to provide the type to k2v_get_key_value(),
 * so that the config file can finally be deserialized.
 * Note: always use k2v_free() to free the buffer.
 */
k2v_buf_t k2v_open_file(const char *_Nonnull path);
/*
 * Get the value of the key from k2v_buf.
 * Always use a pointer to store the value.
 * If value is array, the memory will be allocated by malloc,
 * for example:
 * // For char array.
 * char **value = NULL;
 * ssize_t len = k2v_get_key_value("key", kv, &value, K2V_TYPE_CHAR_ARRAY);
 * // For int array.
 * int *value = NULL;
 * ssize_t len = k2v_get_key_value("key", kv, &value, K2V_TYPE_INT_ARRAY);
 * // For single int.
 * int value = 0;
 * ssize_t ret = k2v_get_key_value("key", kv, &value, K2V_TYPE_INT);
 * The caller should free the value.
 * Return value:
 * -1 if the key is not found or the type is not matched.
 * 0 for scalar type.
 * len for array type.
 */
ssize_t k2v_get_key_value(const char *_Nonnull key, k2v_buf_t _Nonnull kv, void *_Nonnull value, k2v_value_type_t type);
/*
 * This function is used to generate the config file,
 * as the implementation of serialization.
 */
char *k2v_add_config(const char *_Nullable key, const void *_Nullable value, size_t len, k2v_value_type_t type, char *buf);
/*
 * Only for debugging.
 */
void k2v_dump_buffer(k2v_buf_t _Nonnull kv);
/*
 * This function will check if the key is in the buffer with the type.
 * Note that this can not make sure that the config is correct.
 */
bool k2v_have_key(const char *_Nonnull key, k2v_buf_t _Nonnull kv, k2v_data_type_t type);
/*
 * Caller should make sure that the buffer is legal.
 */
k2v_buf_t k2v_from_buf(const char *_Nonnull buf);