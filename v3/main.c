#include "include/k2v.h"
int main()
{
	char *buf = NULL;
	buf = k2v_add_config("char", "hello", 0, K2V_TYPE_CHAR, buf);
	int intval = 123;
	buf = k2v_add_config("int", &intval, 0, K2V_TYPE_INT, buf);
	float floatval = 123.456;
	buf = k2v_add_config("float", &floatval, 0, K2V_TYPE_FLOAT, buf);
	long longval = 123456789012345;
	buf = k2v_add_config("long", &longval, 0, K2V_TYPE_LONG, buf);
	bool boolval = true;
	buf = k2v_add_config("bool", &boolval, 0, K2V_TYPE_BOOL, buf);
	char *char_array[] = { "hello", "world", NULL };
	buf = k2v_add_config("char_array", char_array, 2, K2V_TYPE_CHAR_ARRAY, buf);
	int int_array[] = { 1, 2, 3, 4, 5 };
	buf = k2v_add_config("int_array", int_array, 5, K2V_TYPE_INT_ARRAY, buf);
	float float_array[] = { 1.1, 2.2, 3.3, 4.4, 5.5 };
	buf = k2v_add_config("float_array", float_array, 5, K2V_TYPE_FLOAT_ARRAY, buf);
	long long_array[] = { 1, 2, 3, 4, 5 };
	buf = k2v_add_config("long_array", long_array, 5, K2V_TYPE_LONG_ARRAY, buf);
	buf = k2v_add_config(NULL, NULL, 0, K2V_TYPE_NEWLINE, buf);
	char *comment = "This is a comment";
	buf = k2v_add_config(comment, NULL, 0, K2V_TYPE_COMMENT, buf);
	printf("%s", buf);
	k2v_buf_t kv = k2v_open_file("test.conf");
	k2v_dump_buffer(kv);
	k2v_free(kv);
	free(buf);
}