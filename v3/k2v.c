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
#include "./k2v.h"
/*
 * WARNING:
 * NO ANY FALLBACKS, JUST PANIC ON ANY ERROR.
 * USERS SHOULD BE RESPONSIBLE FOR THE INPUT.
 * SECURITY IS THE MOST IMPORTANT THING, SO WE DO NOT ALLOW ANY UNEXPECTED INPUTS.
 */
struct K2V3_STRING {
	char *str;
	size_t len;
};
struct K2V3_KEY {
	char *key;
	size_t jmp;
};
struct K2V3_VALUE {
	enum K2V3_TYPE type;
	char *scalar_val;
	size_t array_len;
	char **array_val;
	bool empty;
	size_t jmp;
};
static struct K2V3_KEY get_key(struct K2V3_STRING conf)
{
	struct K2V3_KEY ret = { .key = NULL, .jmp = 0 };
	// Key should be a non-empty string without spaces, and end with `=`.
	// Only 0-9, a-z, A-Z and `_` are allowed in keys.
	size_t i = 0;
	while (i < conf.len && conf.str[i] != '=' && conf.str[i] != '\n') {
		if (conf.str[i] == ' ') {
			k2v3_error("Spaces are not allowed in keys.");
		}
		if (!((conf.str[i] >= '0' && conf.str[i] <= '9') || (conf.str[i] >= 'a' && conf.str[i] <= 'z') || (conf.str[i] >= 'A' && conf.str[i] <= 'Z') || conf.str[i] == '_')) {
			k2v3_error("Invalid character in key, got '%c'. Only 0-9, a-z, A-Z and '_' are allowed.", conf.str[i]);
		}
		i++;
	}
	if (i == 0) {
		k2v3_error("Key cannot be empty.");
	}
	// Dump key.
	ret.key = strndup(conf.str, i);
	if (!ret.key) {
		k2v3_error("Failed to allocate memory for key.");
	}
	// Get jump length.
	if (i < conf.len && conf.str[i] == '=') {
		ret.jmp = i + 1;
	} else {
		k2v3_error("Key must end with '='.");
	}
	return ret;
}
static struct K2V3_VALUE get_value(struct K2V3_STRING conf)
{
	struct K2V3_VALUE ret = { .type = K2V3_SCALAR, .scalar_val = NULL, .array_val = NULL, .jmp = 0, .empty = false };
	// Value can be a scalar or an array.
	// Value should start with `"` for scalar and `[` for array.
	if (conf.len == 0) {
		k2v3_error("Value cannot be empty.");
	}
	// We allow `[]` for null array, `""` for null scalar.
	if (conf.len >= 2) {
		if (conf.str[0] == '"' && conf.str[1] == '"') {
			ret.type = K2V3_SCALAR;
			ret.scalar_val = NULL;
			ret.jmp = 2;
			ret.empty = true;
			return ret;
		}
		if (conf.str[0] == '[' && conf.str[1] == ']') {
			ret.type = K2V3_ARRAY;
			ret.array_len = 0;
			ret.array_val = NULL;
			ret.jmp = 2;
			ret.empty = true;
			return ret;
		}
	}
	// Parse value.
	if (conf.str[0] == '"') {
		ret.type = K2V3_SCALAR;
		// Scalar value. Get the string until the next unescaped `"` character.
		// Boundary is next "\n" or end of string.
		size_t boundary = conf.len;
		char *newline = memchr(conf.str, '\n', conf.len);
		if (newline) {
			boundary = (size_t)(newline - conf.str);
		}
		size_t i = 1;
		while (i < conf.len) {
			if (conf.str[i] == '"' && conf.str[i - 1] != '\\') {
				break;
			}
			if (i >= boundary) {
				k2v3_error("Unterminated scalar value.");
			}
			i++;
		}
		// Dump scalar value.
		if (i >= conf.len) {
			k2v3_error("Unterminated scalar value.");
		}
		ret.scalar_val = strndup(conf.str + 1, i - 1);
		if (!ret.scalar_val) {
			k2v3_error("Failed to allocate memory for scalar value.");
		}
		// Get jump length.
		ret.jmp = i + 1;
	} else if (conf.str[0] == '[') {
		ret.type = K2V3_ARRAY;
		// Array value.
		// Boundry is next excepted `]` character that is not escaped.
		// Newline is allowed in array values, so we do not consider newline as boundary here.
		size_t i = 1;
		while (i < conf.len) {
			if (conf.str[i] == ']' && conf.str[i - 1] != '\\') {
				break;
			}
			i++;
		}
		if (i >= conf.len) {
			k2v3_error("Unterminated array value.");
		}
		// Get array elements.
		// Each element should be a scalar value, and separated by `,` or `;`.
		// We do not allow trailing separators, so the last element should not be followed by `,` or `;`.
		// We also do not allow empty elements.
		size_t array_len = 0;
		char **array_val = NULL;
		size_t j = 1;
		while (j < i) {
			// First is `"`.
			if (conf.str[j] != '"') {
				k2v3_error("Array elements must start with '\"'.");
			}
			// Get the string until the next unescaped `"` character.
			// If there is no next unescaped `"` character before the boundary, it's an error.
			// Get the next unescaped `"` character.
			size_t k = j + 1;
			while (k < i) {
				if (conf.str[k] == '"' && conf.str[k - 1] != '\\') {
					break;
				}
				k++;
			}
			if (k == 0) {
				k2v3_error("Empty array element.");
			}
			if (k >= i) {
				k2v3_error("Unterminated array element.");
			}
			// Dump array element.
			char *element = strndup(conf.str + j + 1, k - j - 1);
			if (!element) {
				k2v3_error("Failed to allocate memory for array element.");
			}
			// Add to array.
			char **new_array_val = realloc(array_val, sizeof(char *) * (array_len + 1));
			if (!new_array_val) {
				free(element);
				k2v3_error("Failed to allocate memory for array.");
			}
			array_val = new_array_val;
			array_val[array_len] = element;
			array_len++;
			// Move to the next element.
			j = k + 1;
			// Skip spaces after the element.
			while (j < i && (conf.str[j] == ' ' || conf.str[j] == '\n')) {
				j++;
			}
			// Skip separators.
			if (conf.str[j] == ',' || conf.str[j] == ';') {
				j++;
			} else {
				// If there is no separator, it should be the end of array.
				if (j < i) {
					printf("Debug info: conf.str:\n%s\n", conf.str);
					k2v3_error("Array elements must be separated by ',' or ';'.");
				}
			}
			while (j < i && (conf.str[j] == ' ' || conf.str[j] == '\n')) {
				j++;
			}
		}
		ret.array_len = array_len;
		ret.array_val = array_val;
		// Get jump length.
		ret.jmp = i + 1;
	} else {
		k2v3_error("Value must start with '\"' for scalar or '[' for array.");
	}
	// Ensure the value is followed by a newline or end of string.
	if (ret.jmp < conf.len && conf.str[ret.jmp] != '\n') {
		printf("Debug info: conf.str:\n%s\n", conf.str);
		k2v3_error("Value must be followed by a newline, but got '%c'.", conf.str[ret.jmp]);
	}
	return ret;
}
k2v3_cache k2v3_parse(char *const _Nonnull buf)
{
	k2v3_cache ret = NULL;
	struct K2V3_STRING conf = { .str = buf, .len = strlen(buf) };
	while (conf.len > 0) {
		// We do not allow leading spaces.
		if (conf.str[0] == ' ') {
			k2v3_error("Leading spaces are not allowed.");
		}
		if (strlen(conf.str) != conf.len) {
			printf("Debug info: conf.str:\n%s\n", conf.str);
			k2v3_error("Internal error: string length mismatch. Expected %zu but got %zu.", conf.len, strlen(conf.str));
		}
		// Skip comments and empty lines.
		if (conf.str[0] == '\n') {
			if (strlen(conf.str) != conf.len) {
				printf("Debug info: conf.str:\n%s\n", conf.str);
				k2v3_error("Internal error: string length mismatch. Expected %zu but got %zu.", conf.len, strlen(conf.str));
			}
			conf.str++;
			conf.len--;
			continue;
		}
		if (conf.str[0] == '#') {
			if (strlen(conf.str) != conf.len) {
				printf("Debug info: conf.str:\n%s\n", conf.str);
				k2v3_error("Internal error: string length mismatch. Expected %zu but got %zu.", conf.len, strlen(conf.str));
			}
			char *newline = memchr(conf.str, '\n', conf.len);
			if (!newline) {
				break;
			}
			conf.str = newline + 1;
			conf.len = strlen(conf.str);
			if (strlen(conf.str) != conf.len) {
				printf("Debug info: conf.str:\n%s\n", conf.str);
				k2v3_error("Internal error: string length mismatch. Expected %zu but got %zu.", conf.len, strlen(conf.str));
			}
			continue;
		} else {
			// Get key.
			struct K2V3_KEY key = get_key(conf);
			if (!key.key) {
				k2v3_error("Failed to parse key.");
			}
			conf.str += key.jmp;
			conf.len -= key.jmp;
			if (strlen(conf.str) != conf.len) {
				printf("Debug info: conf.str:\n%s\n", conf.str);
				k2v3_error("Internal error: string length mismatch after parsing key. Expected %zu but got %zu.", conf.len, strlen(conf.str));
			}
			// Get value.
			struct K2V3_VALUE value = get_value(conf);
			if (!value.scalar_val && !value.array_val && !value.empty) {
				printf("Debug info: conf.str:\n%s\n", conf.str);
				k2v3_error("Failed to parse value.");
			}
			conf.str += value.jmp;
			conf.len -= value.jmp;
			if (strlen(conf.str) != conf.len) {
				printf("Debug info: conf.str:\n%s\n", conf.str);
				k2v3_error("Internal error: string length mismatch after parsing value. Expected %zu but got %zu.", conf.len, strlen(conf.str));
			}
			// Add to cache.
			k2v3_cache node = malloc(sizeof(struct K2V3_BUF));
			if (!node) {
				k2v3_error("Failed to allocate cache node.");
			}
			*node = (struct K2V3_BUF){ .type = value.type, .key = key.key, .next = NULL };
			if (value.empty) {
				node->empty = true;
			} else {
				node->empty = false;
				if (value.type == K2V3_ARRAY) {
					node->array_len = value.array_len;
					node->array_val = value.array_val;
				} else {
					node->scalar_val = value.scalar_val;
				}
			}
			// Append to the cache.
			if (ret == NULL) {
				ret = node;
			} else {
				k2v3_cache cur = ret;
				while (cur->next != NULL) {
					// Also check for duplicate keys.
					if (strcmp(cur->key, key.key) == 0) {
						k2v3_error("Duplicate keys are not allowed.");
					}
					cur = cur->next;
				}
				cur->next = node;
			}
		}
	}
	return ret;
}
void k2v3_free_cache(k2v3_cache *cache)
{
	if (cache == NULL || *cache == NULL) {
		return;
	}
	k2v3_cache cur = *cache;
	while (cur != NULL) {
		k2v3_cache next = cur->next;
		if (cur->type == K2V3_ARRAY) {
			for (size_t i = 0; i < cur->array_len; i++) {
				free(cur->array_val[i]);
			}
			free(cur->array_val);
		} else {
			free(cur->scalar_val);
		}
		free(cur->key);
		free(cur);
		cur = next;
	}
	*cache = NULL;
}
void k2v3_dump(k2v3_cache cache)
{
	while (cache != NULL) {
		if (cache->empty) {
			printf("%s: %s\n", cache->key, cache->type == K2V3_ARRAY ? "[]" : "\"\"");
			cache = cache->next;
			continue;
		}
		if (cache->type == K2V3_ARRAY) {
			printf("%s: [", cache->key);
			for (size_t i = 0; i < cache->array_len; i++) {
				printf("\"%s\"", cache->array_val[i]);
				if (i < cache->array_len - 1) {
					printf(", ");
				}
			}
			printf("]\n");
		} else {
			printf("%s: %s\n", cache->key, cache->scalar_val);
		}
		cache = cache->next;
	}
}
char *k2v3_open_file(char *const _Nonnull path)
{
	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		k2v3_error("Failed to open file '%s'.", path);
	}
	struct stat st;
	if (fstat(fd, &st) < 0) {
		close(fd);
		k2v3_error("Failed to get file size for '%s'.", path);
	}
	if (st.st_size == 0) {
		close(fd);
		k2v3_error("File '%s' is empty.", path);
	}
	if (st.st_size > 128 * 1024) {
		close(fd);
		k2v3_error("File '%s' is too large. Maximum allowed size is 128k.", path);
	}
	char *buf = malloc((size_t)st.st_size + 1);
	if (!buf) {
		close(fd);
		k2v3_error("Failed to allocate memory for file content.");
	}
	ssize_t read_size = read(fd, buf, (size_t)st.st_size);
	if (read_size < 0) {
		free(buf);
		close(fd);
		k2v3_error("Failed to read file '%s'.", path);
	}
	buf[read_size] = '\0';
	close(fd);
	return buf;
}
bool k2v3_have_key(k2v3_cache cache, char *const _Nonnull key)
{
	while (cache != NULL) {
		if (strcmp(cache->key, key) == 0) {
			return true;
		}
		cache = cache->next;
	}
	return false;
}
