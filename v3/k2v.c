#include "include/k2v.h"
bool k2v_stop_at_warning = false;
bool k2v_show_warning = true;
// Warnings && stop at warning.
#define k2v_warning(...)                                                                                  \
	{                                                                                                 \
		if (k2v_show_warning) {                                                                   \
			fprintf(stderr, "\033[31mAt %s at %d at %s:\n", __FILE__, __LINE__, __func__);    \
			fprintf(stderr, ##__VA_ARGS__);                                                   \
			fprintf(stderr, "\033[0m");                                                       \
			if (k2v_stop_at_warning) {                                                        \
				fprintf(stderr, "\033[31mlibk2v stop_at_warning set, exit now\n\033[0m"); \
			}                                                                                 \
		}                                                                                         \
		if (k2v_stop_at_warning) {                                                                \
			exit(114);                                                                        \
		}                                                                                         \
	}
//
// Inner functions.
//
static void __k2v_check_singularity(k2v_buf_t _Nonnull kv)
{
	/*
	 * Check if the key is unique.
	 * Return 0 if the key is unique,
	 * -1 if the key is not unique.
	 */
	if (kv == NULL) {
		return;
	}
	// Get the length of kv.
	size_t len = 0;
	for (int i = 0; kv[i] != NULL; i++) {
		len++;
	}
	char **keys = malloc(sizeof(char *) * (len + 1));
	keys[0] = NULL;
	// Check if the key is unique.
	for (int i = 0; kv[i] != NULL; i++) {
		for (int j = 0; j < i; j++) {
			if (strcmp(kv[i]->key, kv[j]->key) == 0) {
				k2v_warning("Key should be unique: %s\n", kv[i]->key);
				free(keys);
				return;
			}
		}
		keys[i] = kv[i]->key;
		keys[i + 1] = NULL;
	}
	free(keys);
}
static void __print_current_line(const char *p)
{
	/*
	 * Print the current line.
	 */
	if (p == NULL) {
		return;
	}
	while (*p != '\n' && *p != '\0') {
		putchar(*p);
		p++;
	}
	putchar('\n');
}
static char *correct_backslash(char *buf)
{
	/*
	 * Delete the backslash.
	 * '\n' -> '\n' (no change)
	 * '\t' -> '\t' (no change)
	 * '\r' -> '\r' (no change)
	 * '\\' -> '\' (delete one backslash)
	 * '\x' -> 'x' (delete the backslash)
	 * '\0' -> '0' (delete the backslash)
	 * '\"' -> '\"' (no change)
	 * As I need it to follow the Shell standard,
	 * '\"' will output as '\"'.
	 */
	char *ret = strdup(buf);
	int j = 0;
	for (size_t i = 0; i < strlen(buf); i++) {
		if (buf[i] == '\\') {
			if (i < strlen(buf) - 1) {
				i++;
				if (buf[i] == 'n') {
					ret[j] = '\\';
					j++;
					ret[j] = 'n';
				} else if (buf[i] == 't') {
					ret[j] = '\\';
					j++;
					ret[j] = 't';
				} else if (buf[i] == 'r') {
					ret[j] = '\\';
					j++;
					ret[j] = 'r';
				} else if (buf[i] == '"') {
					ret[j] = '\\';
					j++;
					ret[j] = '"';
				} else {
					ret[j] = buf[i];
				}
				j++;
				ret[j] = '\0';
				continue;
			}
		}
		ret[j] = buf[i];
		j++;
		ret[j] = '\0';
	}
	return ret;
}
static char *__k2v_v3_open_file(const char *path)
{
	/*
	 * Open the file and read it into a buffer.
	 * The buffer will be used to store the key-value pairs.
	 */
	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		k2v_warning("open() failed\n");
		return NULL;
	}
	struct stat st;
	if (fstat(fd, &st) == -1) {
		close(fd);
		k2v_warning("fstat() failed\n");
		return NULL;
	}
	char *buf = malloc(st.st_size + 1);
	if (buf == NULL) {
		close(fd);
		k2v_warning("malloc() failed\n");
		return NULL;
	}
	size_t bytes_read = read(fd, buf, st.st_size);
	buf[bytes_read] = '\0';
	close(fd);
	if (strlen(buf) != bytes_read) {
		k2v_warning("The lenth of the string read is not equal to the length of the file\n");
	}
	char *ret = correct_backslash(buf);
	free(buf);
	return ret;
}
static const char *__jump_to_next_line(const char *p)
{
	/*
	 * Jump to the next line.
	 */
	while (*p != '\n' && *p != '\0') {
		p++;
	}
	if (*p == '\0') {
		return NULL;
	}
	if (*p == '\n') {
		p++;
		while (*p == ' ' || *p == '\n') {
			p++;
		}
		if (*p == '\0') {
			return NULL;
		}
		return p;
	}
	return NULL;
}
typedef enum { K2V_PARSER_START, K2V_PASER_WAIT_VALUE_START, K2V_PASER_IN_STRING, K2V_PASER_WAIT_COMMA_OR_END, K2V_PASER_OUT_OF_CONF } k2v_fsm_t;
static const char *__k2v_parse_current_line(const char *p, k2v_buf_t *_Nullable kv)
{
	return NULL;
}
//
// Deserialization.
//
void k2v_free(k2v_buf_t _Nonnull kv)
{
	/*
	 * Free the key-value buffer.
	 */
	if (kv == NULL) {
		return;
	}
	for (int i = 0; kv[i] != NULL; i++) {
		free(kv[i]->key);
		if (kv[i]->type == K2V_DATA_TYPE_ARRAY) {
			for (int j = 0; (((kv[i])->data.array)[j]) != NULL; j++) {
				free(((kv[i])->data.array)[j]);
			}
			free((kv[i])->data.scalar);
		}
		free(kv[i]);
	}
	free(kv);
}
ssize_t k2v_get_key_value(const char *_Nonnull key, k2v_buf_t _Nonnull kv, void *_Nonnull value, k2v_value_type_t type)
{
	/*
	 * Get the key-value pair from the buffer.
	 * The key-value pair will be stored in the value.
	 * The type will be used to determine the type of the value.
	 * Return 0 or the length of the value.
	 * Return -1 if the key is not found.
	 */

	if (kv == NULL) {
		k2v_warning("kv buffer is NULL when searching key '%s'\n", key);
		return -1;
	}

	for (int i = 0; kv[i] != NULL; i++) {
		if (strcmp(kv[i]->key, key) == 0) {
			if (kv[i]->data.scalar == NULL && kv[i]->type == K2V_DATA_TYPE_SCALAR) {
				k2v_warning("key '%s' has NULL scalar value\n", key);
				return -1;
			}

			switch (type) {
			case K2V_TYPE_CHAR:
				if (kv[i]->type != K2V_DATA_TYPE_SCALAR) {
					k2v_warning("key '%s' expected SCALAR but got ARRAY\n", key);
					return -1;
				}
				*(char **)value = kv[i]->data.scalar;
				return 0;

			case K2V_TYPE_INT:
				if (kv[i]->type != K2V_DATA_TYPE_SCALAR) {
					k2v_warning("key '%s' expected SCALAR but got ARRAY\n", key);
					return -1;
				}
				*(int *)value = kv[i]->data.scalar ? atoi(kv[i]->data.scalar) : 0;
				return 0;

			case K2V_TYPE_BOOL:
				if (kv[i]->type != K2V_DATA_TYPE_SCALAR) {
					k2v_warning("key '%s' expected SCALAR but got ARRAY\n", key);
					return -1;
				}
				if (strcasecmp(kv[i]->data.scalar, "true") == 0) {
					*(bool *)value = true;
				} else if (strcasecmp(kv[i]->data.scalar, "false") == 0) {
					*(bool *)value = false;
				} else {
					k2v_warning("key '%s' has invalid BOOL value '%s'\n", key, kv[i]->data.scalar);
					return -1;
				}
				return 0;

			case K2V_TYPE_FLOAT:
				if (kv[i]->type != K2V_DATA_TYPE_SCALAR) {
					k2v_warning("key '%s' expected SCALAR but got ARRAY\n", key);
					return -1;
				}
				*(float *)value = atof(kv[i]->data.scalar);
				return 0;

			case K2V_TYPE_LONG:
				if (kv[i]->type != K2V_DATA_TYPE_SCALAR) {
					k2v_warning("key '%s' expected SCALAR but got ARRAY\n", key);
					return -1;
				}
				*(long *)value = atol(kv[i]->data.scalar);
				return 0;

			case K2V_TYPE_CHAR_ARRAY:
			case K2V_TYPE_INT_ARRAY:
			case K2V_TYPE_FLOAT_ARRAY:
			case K2V_TYPE_LONG_ARRAY: {
				if (kv[i]->type != K2V_DATA_TYPE_ARRAY) {
					k2v_warning("key '%s' expected ARRAY but got SCALAR\n", key);
					return -1;
				}
				size_t len = 0;
				for (size_t j = 0; ((char **)kv[i]->data.array)[j] != NULL; j++) {
					len++;
				}

				if (len == 0) {
					k2v_warning("key '%s' array is empty\n", key);
					return 0;
				}

				switch (type) {
				case K2V_TYPE_CHAR_ARRAY: {
					char **arr = malloc(sizeof(char *) * (len + 1));
					if (!arr) {
						k2v_warning("malloc failed for key '%s'\n", key);
						return -1;
					}
					for (size_t j = 0; j < len; j++) {
						arr[j] = strdup(((char **)kv[i]->data.array)[j]);
					}
					arr[len] = NULL;
					*(char ***)value = arr;
					return (ssize_t)len;
				}
				case K2V_TYPE_INT_ARRAY: {
					int *arr = malloc(sizeof(int) * len);
					if (!arr) {
						k2v_warning("malloc failed for key '%s'\n", key);
						return -1;
					}
					for (size_t j = 0; j < len; j++) {
						arr[j] = atoi(((char **)kv[i]->data.array)[j]);
					}
					*(int **)value = arr;
					return (ssize_t)len;
				}
				case K2V_TYPE_FLOAT_ARRAY: {
					float *arr = malloc(sizeof(float) * len);
					if (!arr) {
						k2v_warning("malloc failed for key '%s'\n", key);
						return -1;
					}
					for (size_t j = 0; j < len; j++) {
						arr[j] = atof(((char **)kv[i]->data.array)[j]);
					}
					*(float **)value = arr;
					return (ssize_t)len;
				}
				case K2V_TYPE_LONG_ARRAY: {
					long *arr = malloc(sizeof(long) * len);
					if (!arr) {
						k2v_warning("malloc failed for key '%s'\n", key);
						return -1;
					}
					for (size_t j = 0; j < len; j++) {
						arr[j] = atol(((char **)kv[i]->data.array)[j]);
					}
					*(long **)value = arr;
					return (ssize_t)len;
				}
				default:
					break;
				}
			}
			default:
				k2v_warning("unsupported type %d requested for key '%s'\n", type, key);
				return -1;
			}
		}
	}

	k2v_warning("key '%s' not found\n", key);
	return -1;
}
k2v_buf_t k2v_open_file(const char *_Nonnull path)
{
	char *buf = __k2v_v3_open_file(path);
	if (buf == NULL) {
		return NULL;
	}
	k2v_buf_t ret = malloc(sizeof(k2v_buf_t));
	ret[0] = NULL;
	// Parse the buffer and return the key-value pairs.
	const char *p = buf;
	while (p) {
		p = __k2v_parse_current_line(p, &ret);
		if (p == NULL) {
			break;
		}
	}
	// Free the buffer.
	free(buf);
	// Check if the key is unique.
	__k2v_check_singularity(ret);
	return ret;
}
k2v_buf_t k2v_from_buf(const char *_Nonnull buf)
{
	k2v_buf_t ret = malloc(sizeof(k2v_buf_t));
	ret[0] = NULL;
	const char *p = buf;
	while (p) {
		p = __k2v_parse_current_line(p, &ret);
		if (p == NULL) {
			break;
		}
	}
	// Check if the key is unique.
	__k2v_check_singularity(ret);
	return ret;
}
//
// Utility functions.
//
void k2v_dump_buffer(k2v_buf_t _Nonnull kv)
{
	/*
	 * Dump the buffer.
	 * This function is for debugging purpose only.
	 */
	if (kv == NULL) {
		return;
	}
	printf("Key-Value pairs:\n\n");
	for (int i = 0; kv[i] != NULL; i++) {
		printf("Key: %s\n", kv[i]->key);
		if (kv[i]->type == K2V_DATA_TYPE_ARRAY) {
			for (int j = 0; (kv[i]->data.array)[j] != NULL; j++) {
				printf("\"%s\" ", (kv[i]->data.array)[j]);
			}
			printf("\n");
		} else {
			printf("value: %s\n\n", kv[i]->data.scalar);
		}
	}
}
bool k2v_have_key(const char *_Nonnull key, k2v_buf_t _Nonnull kv, k2v_data_type_t type)
{
	/*
	 * Check if the key is in the buffer.
	 * Return true if the key is in the buffer,
	 * false if the key is not in the buffer.
	 */
	if (kv == NULL) {
		return false;
	}
	for (int i = 0; kv[i] != NULL; i++) {
		if (strcmp(kv[i]->key, key) == 0) {
			if (kv[i]->type != type) {
				return false;
			}
			return true;
		}
	}
	return false;
}
//
// Serialization.
//
char *k2v_add_config(const char *_Nullable key, const void *_Nullable value, size_t len, k2v_value_type_t type, char *buf)
{
	/*
	 * Add the key-value pair to the buffer.
	 * The key-value pair will be stored in the buffer.
	 * The type will be used to determine the type of the value.
	 * The original buffer will be freed.
	 * The new buffer will be returned.
	 */
	char *ret = NULL;
	if (type == K2V_TYPE_NEWLINE) {
		if (buf == NULL) {
			ret = malloc(2);
			sprintf(ret, "\n");
		} else {
			size_t size = strlen(buf) + 2;
			ret = malloc(size);
			sprintf(ret, "%s\n", buf);
			free(buf);
			return ret;
		}
	}
	if (key == NULL) {
		return buf;
	}
	size_t size = 0;
	switch (type) {
	case K2V_TYPE_COMMENT:
		if (key == NULL) {
			return buf;
		}
		if (buf == NULL) {
			ret = malloc(strlen(key) + 8);
			sprintf(ret, "#%s\n", key);
		} else {
			size_t size = strlen(buf) + strlen(key) + 8;
			ret = malloc(size);
			sprintf(ret, "%s#%s\n", buf, key);
			free(buf);
		}
		return ret;
	case K2V_TYPE_CHAR:
		if (buf == NULL) {
			ret = malloc(strlen(key) + strlen(value) + 8);
			sprintf(ret, "%s=\"%s\"\n", key, (char *)value);
		} else {
			size_t size = strlen(buf) + strlen(key) + strlen(value) + 8;
			ret = malloc(size);
			sprintf(ret, "%s%s=\"%s\"\n", buf, key, (char *)value);
			free(buf);
		}
		return ret;
	case K2V_TYPE_INT:
		if (buf == NULL) {
			ret = malloc(strlen(key) + 20);
			sprintf(ret, "%s=\"%d\"\n", key, *(int *)value);
		} else {
			size_t size = strlen(buf) + strlen(key) + 20;
			ret = malloc(size);
			sprintf(ret, "%s%s=\"%d\"\n", buf, key, *(int *)value);
			free(buf);
		}
		return ret;
	case K2V_TYPE_BOOL:
		if (buf == NULL) {
			ret = malloc(strlen(key) + 20);
			sprintf(ret, "%s=\"%s\"\n", key, (*(bool *)value) ? "true" : "false");
		} else {
			size_t size = strlen(buf) + strlen(key) + 20;
			ret = malloc(size);
			sprintf(ret, "%s%s=\"%s\"\n", buf, key, (*(bool *)value) ? "true" : "false");
			free(buf);
		}
		return ret;
	case K2V_TYPE_FLOAT:
		if (buf == NULL) {
			ret = malloc(strlen(key) + 20);
			sprintf(ret, "%s=\"%f\"\n", key, *(float *)value);
		} else {
			size_t size = strlen(buf) + strlen(key) + 20;
			ret = malloc(size);
			sprintf(ret, "%s%s=\"%f\"\n", buf, key, *(float *)value);
			free(buf);
		}
		return ret;
	case K2V_TYPE_LONG:
		if (buf == NULL) {
			ret = malloc(strlen(key) + 20);
			sprintf(ret, "%s=\"%ld\"\n", key, *(long *)value);
		} else {
			size_t size = strlen(buf) + strlen(key) + 20;
			ret = malloc(size);
			sprintf(ret, "%s%s=\"%ld\"\n", buf, key, *(long *)value);
			free(buf);
		}
		return ret;
	case K2V_TYPE_CHAR_ARRAY:
		for (int i = 0; i < len; i++) {
			size += strlen(((char **)value)[i]) + 6;
		}
		if (buf == NULL) {
			ret = malloc(strlen(key) + size);
			char *tmp = malloc(size + strlen(key) + 2);
			sprintf(tmp, "%s=[", key);
			for (int i = 0; i < len; i++) {
				sprintf(tmp, "%s\"%s\"", tmp, ((char **)value)[i]);
				if (i != len - 1) {
					strcat(tmp, ",");
				} else {
					strcat(tmp, "]");
				}
			}
			sprintf(ret, "%s%s\n", buf, tmp);
			free(tmp);
			free(buf);
		} else {
			ret = malloc(strlen(buf) + strlen(key) + size + 16);
			char *tmp = malloc(size + strlen(key) + 2);
			sprintf(tmp, "%s=[", key);
			for (int i = 0; i < len; i++) {
				sprintf(tmp, "%s\"%s\"", tmp, ((char **)value)[i]);
				if (i != len - 1) {
					strcat(tmp, ",");
				} else {
					strcat(tmp, "]");
				}
			}
			sprintf(ret, "%s%s\n", buf, tmp);
			free(tmp);
			free(buf);
		}
		return ret;
	case K2V_TYPE_INT_ARRAY:
		if (buf == NULL) {
			ret = malloc(strlen(key) + 20 * (len + 2));
			char *tmp = malloc(20 * (len + 2));
			sprintf(tmp, "%s=[", key);
			for (int i = 0; i < len; i++) {
				sprintf(tmp, "%s\"%d\"", tmp, ((int *)value)[i]);
				if (i != len - 1) {
					strcat(tmp, ",");
				} else {
					strcat(tmp, "]");
				}
			}
			sprintf(ret, "%s%s\n", buf, tmp);
			free(tmp);
			free(buf);
		} else {
			size_t size = strlen(buf) + strlen(key) + 20 * (len + 2);
			ret = malloc(size);
			char *tmp = malloc(20 * (len + 2));
			sprintf(tmp, "%s=[", key);
			for (int i = 0; i < len; i++) {
				sprintf(tmp, "%s\"%d\"", tmp, ((int *)value)[i]);
				if (i != len - 1) {
					strcat(tmp, ",");
				} else {
					strcat(tmp, "]");
				}
			}
			sprintf(ret, "%s%s\n", buf, tmp);
			free(tmp);
			free(buf);
		}
		return ret;
	case K2V_TYPE_FLOAT_ARRAY:
		if (buf == NULL) {
			ret = malloc(strlen(key) + 20 * (len + 2));
			char *tmp = malloc(20 * (len + 2));
			sprintf(tmp, "%s=[", key);
			for (int i = 0; i < len; i++) {
				sprintf(tmp, "%s\"%f\"", tmp, ((float *)value)[i]);
				if (i != len - 1) {
					strcat(tmp, ",");
				} else {
					strcat(tmp, "]");
				}
			}
			sprintf(ret, "%s%s\n", buf, tmp);
			free(tmp);
			free(buf);
		} else {
			size_t size = strlen(buf) + strlen(key) + 20 * (len + 2);
			ret = malloc(size);
			char *tmp = malloc(20 * (len + 2));
			sprintf(tmp, "%s=[", key);
			for (int i = 0; i < len; i++) {
				sprintf(tmp, "%s\"%f\"", tmp, ((float *)value)[i]);
				if (i != len - 1) {
					strcat(tmp, ",");
				} else {
					strcat(tmp, "]");
				}
			}
			sprintf(ret, "%s%s\n", buf, tmp);
			free(tmp);
			free(buf);
		}
		return ret;
	case K2V_TYPE_LONG_ARRAY:
		if (buf == NULL) {
			ret = malloc(strlen(key) + 20 * (len + 2));
			char *tmp = malloc(20 * (len + 2));
			sprintf(tmp, "%s=[", key);
			for (int i = 0; i < len; i++) {
				sprintf(tmp, "%s\"%ld\"", tmp, ((long *)value)[i]);
				if (i != len - 1) {
					strcat(tmp, ",");
				} else {
					strcat(tmp, "]");
				}
			}
			sprintf(ret, "%s%s\n", buf, tmp);
			free(tmp);
			free(buf);
		} else {
			size_t size = strlen(buf) + strlen(key) + 20 * (len + 2);
			ret = malloc(size);
			char *tmp = malloc(20 * (len + 2));
			sprintf(tmp, "%s=[", key);
			for (int i = 0; i < len; i++) {
				sprintf(tmp, "%s\"%ld\"", tmp, ((long *)value)[i]);
				if (i != len - 1) {
					strcat(tmp, ",");
				} else {
					strcat(tmp, "]");
				}
			}
			sprintf(ret, "%s%s\n", buf, tmp);
			free(tmp);
			free(buf);
		}
		return ret;
	default:
		return buf;
	}
	return ret;
}