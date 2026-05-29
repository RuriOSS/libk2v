#include "k2v3.h"
#include "../src/include/k2v.h"
int main()
{
	char *buf = k2v3_open_file("test.conf");
	k2v3_cache cache = k2v3_parse(buf);
	k2v3_dump(cache);
	printf("\n\n");
	printf("bool_val: %s\n", k2v3_get_bool("bool_val", cache) ? "true" : "false");
	printf("int_val: %d\n", k2v3_get_int("int_val", cache));
	printf("char_val: %s\n", k2v3_get_char("char_val", cache));
	printf("float_val: %f\n", k2v3_get_float("float_val", cache));
	printf("long_val: %lld\n", k2v3_get_long("long_val", cache));
	// Get int array.
	int int_array[3];
	int int_array_len = k2v3_get_int_array("int_array_val", cache, int_array, 3);
	printf("int_array_val: [");
	for (int i = 0; i < int_array_len; i++) {
		printf("%d", int_array[i]);
		if (i < int_array_len - 1) {
			printf(", ");
		}
	}
	printf("]\n");
	// Get float array.
	float float_array[3];
	int float_array_len = k2v3_get_float_array("float_array_val", cache, float_array, 3);
	printf("float_array_val: [");
	for (int i = 0; i < float_array_len; i++) {
		printf("%f", float_array[i]);
		if (i < float_array_len - 1) {
			printf(", ");
		}
	}
	printf("]\n");
	// Get char array.
	char *char_array[3];
	int char_array_len = k2v3_get_char_array("char_array_val", cache, char_array, 3);
	printf("char_array_val: [");
	for (int i = 0; i < char_array_len; i++) {
		printf("\"%s\"", char_array[i]);
		if (i < char_array_len - 1) {
			printf(", ");
		}
	}
	printf("]\n");
	// Get long array.
	long long long_array[3];
	int long_array_len = k2v3_get_long_array("long_array_val", cache, long_array, 3);
	printf("long_array_val: [");
	for (int i = 0; i < long_array_len; i++) {
		printf("%lld", long_array[i]);
		if (i < long_array_len - 1) {
			printf(", ");
		}
	}
	printf("]\n");
	k2v3_free_cache(&cache);
	free(buf);
	return 0;
}