#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "include/k2v3.h"

static void make_key(const uint8_t *data, size_t size, char *out, size_t out_size)
{
	static const char alphabet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
	size_t alpha_len = sizeof(alphabet) - 1;
	if (out_size == 0) {
		return;
	}
	if (out_size == 1) {
		out[0] = '\0';
		return;
	}
	size_t len = 1;
	if (size > 0) {
		len = (size_t)(data[0] % (out_size - 1)) + 1;
	}
	if (len >= out_size) {
		len = out_size - 1;
	}
	for (size_t i = 0; i < len; i++) {
		uint8_t b = (i < size) ? data[i] : (uint8_t)i;
		out[i] = alphabet[b % alpha_len];
	}
	out[len] = '\0';
}

static void exercise_buffer(char *buf, const char *key)
{
	if (buf == NULL || key == NULL || key[0] == '\0') {
		return;
	}

	k2v3_cache cache = k2v3_parse(buf);
	free(k2v3_get_char(key, cache));
	k2v3_get_int(key, cache);
	k2v3_get_float(key, cache);
	k2v3_get_long(key, cache);
	k2v3_get_char_array(key, cache, NULL, 0);
	k2v3_get_float_array(key, cache, NULL, 0);
	k2v3_get_long_array(key, cache, NULL, 0);
	k2v3_free_cache(&cache);
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	if (data == NULL || size == 0) {
		return 0;
	}

	char key[16];
	make_key(data, size, key, sizeof(key));

	char *raw = malloc(size + 1);
	if (!raw) {
		return 0;
	}
	memcpy(raw, data, size);
	raw[size] = '\0';

	char *san = malloc(size + 1);
	if (!san) {
		free(raw);
		return 0;
	}
	for (size_t i = 0; i < size; i++) {
		san[i] = (data[i] == 0) ? 'A' : (char)data[i];
	}
	san[size] = '\0';

	exercise_buffer(raw, key);
	exercise_buffer(raw, "key");
	exercise_buffer(raw, "some_key");

	exercise_buffer(san, key);
	exercise_buffer(san, "key");

	free(san);
	free(raw);
	return 0;
}