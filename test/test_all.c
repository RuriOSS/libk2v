#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "k2v.h"

int main()
{
	k2v_stop_at_warning = false; // Don't stop at warnings during tests
	k2v_show_warning = false; // Don't show warnings during tests
	// ---------------------------------------------------------
	// 1. Parsing Valid Configuration Data
	// ---------------------------------------------------------
	const char *cfg = "# Normal configuration scenarios\n"
			  "str_val=\"hello world\"\n"
			  "quoted_str=\"hello world\"\n"
			  "integer_val=\"42\"\n"
			  "floating_val=\"3.14\"\n"
			  "boolean_val=\"true\"\n"
			  "long_val=\"8589934592\"\n"
			  "int_array=[\"1\",\"2\",\"3\",\"4\",\"5\"]\n"
			  "float_array=[\"1.1\",\"2.2\",\"3.3\"]\n"
			  "str_array=[\"a\",\"b\",\"c\"]\n";

	printf("=> Testing Parsing Normal Configs...\n");

	// Check existence
	printf("  [Check] have_key(\"str_val\") == %d\n", have_key("str_val", cfg));
	assert(have_key("str_val", cfg) == true);
	printf("  [Check] have_key(\"unknown_val\") == %d\n", have_key("unknown_val", cfg));
	assert(have_key("unknown_val", cfg) == false);

	// Check string extraction
	char *s = key_get_char("str_val", cfg);
	printf("  [Check] key_get_char(\"str_val\") == \"%s\"\n", s ? s : "NULL");
	assert(s != NULL && strcmp(s, "hello world") == 0);
	free(s);

	char *qs = key_get_char("quoted_str", cfg);
	printf("  [Check] key_get_char(\"quoted_str\") == \"%s\"\n", qs ? qs : "NULL");
	assert(qs != NULL && strcmp(qs, "hello world") == 0);
	free(qs);

	// Check numeric and boolean extraction
	int i_val = key_get_int("integer_val", cfg);
	printf("  [Check] key_get_int(\"integer_val\") == %d\n", i_val);
	assert(i_val == 42);

	float f = key_get_float("floating_val", cfg);
	printf("  [Check] key_get_float(\"floating_val\") == %f\n", f);
	assert(f > 3.1f && f < 3.2f);

	bool b_val = key_get_bool("boolean_val", cfg);
	printf("  [Check] key_get_bool(\"boolean_val\") == %d\n", b_val);
	assert(b_val == true);

	long long_v = key_get_long("long_val", cfg);
	printf("  [Check] key_get_long(\"long_val\") == %ld\n", long_v);
	assert(long_v == 8589934592LL);

	// Check arrays extraction
	int i_arr[5];
	int len = key_get_int_array("int_array", cfg, i_arr, 5);
	printf("  [Check] key_get_int_array(\"int_array\") length == %d, first == %d, last == %d\n", len, i_arr[0], i_arr[len - 1]);
	assert(len == 5 && i_arr[0] == 1 && i_arr[4] == 5);

	float f_arr[5];
	len = key_get_float_array("float_array", cfg, f_arr, 5);
	printf("  [Check] key_get_float_array(\"float_array\") length == %d, first == %f\n", len, f_arr[0]);
	assert(len == 3 && f_arr[0] > 1.0f);

	char *s_arr[5] = { NULL };
	len = key_get_char_array("str_array", cfg, s_arr, 5);
	printf("  [Check] key_get_char_array(\"str_array\") length == %d, elements: [\"%s\", \"%s\", \"%s\"]\n", len, s_arr[0], s_arr[1], s_arr[2]);
	assert(len == 3 && strcmp(s_arr[0], "a") == 0 && strcmp(s_arr[2], "c") == 0);
	for (int i = 0; i < len; i++)
		free(s_arr[i]);

	// ---------------------------------------------------------
	// 2. Parsing Edge Cases and Invalid Data
	// ---------------------------------------------------------
	printf("=> Testing Parsing Edge Cases and Invalid Configs...\n");
	const char *bad_cfg = "missing_eq_val\n"
			      "empty_val=\"\"\n"
			      "wrong_type_int=\"abc\"\n"
			      "unclosed_array=[\"1\",\"2\",\"3\"\n"
			      "=\"empty_key\"\n"
			      "trailing_spaces=\"val   \"\n"
			      "unterminated_quote=\"unclosed\n";

	assert(have_key("missing_eq_val", bad_cfg) == false);
	printf("  [Check] missing_eq_val not found (expected)\n");

	char *empty_s = key_get_char("empty_val", bad_cfg);
	printf("  [Check] empty_val == \"%s\"\n", empty_s ? empty_s : "NULL");
	if (empty_s)
		free(empty_s);

	int w_int = key_get_int("wrong_type_int", bad_cfg);
	printf("  [Check] wrong_type_int parsed as int == %d\n", w_int);
	bool w_bool = key_get_bool("wrong_type_int", bad_cfg); // trying to parse 'abc' as bool
	printf("  [Check] wrong_type_int parsed as bool == %d\n", w_bool);

	int bad_i_arr[10];
	int uc_len = key_get_int_array("unclosed_array", bad_cfg, bad_i_arr, 10);
	printf("  [Check] unclosed_array length parsed == %d\n", uc_len);

	char *ts = key_get_char("trailing_spaces", bad_cfg);
	printf("  [Check] trailing_spaces == \"%s\"\n", ts ? ts : "NULL");
	if (ts)
		free(ts);

	char *uq = key_get_char("unterminated_quote", bad_cfg);
	printf("  [Check] unterminated_quote == \"%s\"\n", uq ? uq : "NULL");
	if (uq)
		free(uq);

	// ---------------------------------------------------------
	// 3. Generation Logic (Writing configs)
	// ---------------------------------------------------------
	printf("=> Testing Configuration Generation...\n");
	char *gen = NULL;

	gen = k2v_add_comment(gen, " This is a generated test file");
	gen = k2v_add_config(char, gen, "gen_str", "generated value");
	gen = k2v_add_config(int, gen, "gen_int", 100);
	gen = k2v_add_config(bool, gen, "gen_bool", false);
	gen = k2v_add_config(float, gen, "gen_float", 2.718f);
	gen = k2v_add_config(long, gen, "gen_long_val", 9876543210LL);

	int ia[] = { 10, 20, 30 };
	gen = k2v_add_config(int_array, gen, "gen_iarr", ia, 3);

	char *sa[] = { "str1", "str2" };
	gen = k2v_add_config(char_array, gen, "gen_sarr", sa, 2);

	float fa[] = { 1.5f, 2.5f };
	gen = k2v_add_config(float_array, gen, "gen_farr", fa, 2);

	gen = k2v_add_newline(gen);

	// Validate that our generated config can be parsed right back
	printf("  [Check] generated string:\n%s\n", gen);

	assert(have_key("gen_int", gen) == true);
	assert(have_key("gen_sarr", gen) == true);

	int g_int = key_get_int("gen_int", gen);
	printf("  [Check] gen_int == %d\n", g_int);
	assert(g_int == 100);

	bool g_bool = key_get_bool("gen_bool", gen);
	printf("  [Check] gen_bool == %d\n", g_bool);
	assert(g_bool == false);

	long g_long = key_get_long("gen_long_val", gen);
	printf("  [Check] gen_long_val == %ld\n", g_long);
	assert(g_long == 9876543210LL);

	char *gs = key_get_char("gen_str", gen);
	printf("  [Check] gen_str == \"%s\"\n", gs ? gs : "NULL");
	assert(gs != NULL && strcmp(gs, "generated value") == 0);
	free(gs);

	int p_ia[5];
	len = key_get_int_array("gen_iarr", gen, p_ia, 5);
	printf("  [Check] gen_iarr length == %d, element[1] == %d\n", len, p_ia[1]);
	assert(len == 3 && p_ia[1] == 20);

	free(gen);

	// ---------------------------------------------------------
	// 4. Fuzzing with garbage data
	// ---------------------------------------------------------
	printf("=> Testing with garbage/random data (Fuzzing)...\n");
	const char *garbage_cfg = "afkjsdklfjsdklfjsdklfjsdklfjsdklfjsdklfj\n"
				  "1234567890-=!@#$%^&*()_+{}|:\"<>?~`-=[]\\;',./\n"
				  "key=\"value\n"
				  "key=value\"\n"
				  "key=[\"val1\", \"val2\"\n"
				  "key=[\"val1, val2]\n"
				  "=value\n"
				  "=\"value\"\n"
				  "[key]\n"
				  "key=val1,val2\n"
				  "\x01\x02\x03\x04\xff\xfe=1\n";

	// Just ensure it doesn't crash
	char *gc = key_get_char("key", garbage_cfg);
	if (gc)
		free(gc);
	key_get_int("key", garbage_cfg);
	key_get_bool("key", garbage_cfg);
	key_get_float("key", garbage_cfg);
	key_get_long("key", garbage_cfg);

	int garbage_i_arr[10];
	key_get_int_array("key", garbage_cfg, garbage_i_arr, 10);
	float garbage_f_arr[10];
	key_get_float_array("key", garbage_cfg, garbage_f_arr, 10);
	char *garbage_s_arr[10] = { 0 };
	int gc_len = key_get_char_array("key", garbage_cfg, garbage_s_arr, 10);
	for (int i = 0; i < gc_len; i++)
		free(garbage_s_arr[i]);

	// Random binary data (multiple iterations)
	printf("=> Testing with multiple illegal random string iterations...\n");
	char random_data[1024];
	for (int iter = 0; iter < 128 * 128; iter++) {
		srand(iter); // Seed with iteration for reproducibility
		int r_len = rand() % 1023 + 1;
		for (int i = 0; i < r_len; i++) {
			// Avoid null terminator in the middle for basic string functions to process it fully
			int r = rand() % 255 + 1;
			random_data[i] = (char)r;
		}
		random_data[r_len] = '\0';
		// Add `\n` and `=` to increase chances of triggering parsing logic
		for (int i = 0; i < 64; i++) {
			random_data[rand() % r_len] = '\n';
			random_data[rand() % r_len] = '=';
			random_data[rand() % r_len] = '"';
			random_data[rand() % r_len] = '[';
			random_data[rand() % r_len] = ']';
			random_data[rand() % r_len] = ',';
			random_data[rand() % r_len] = '\\';
			random_data[rand() % r_len] = '#';
		}
		char *rc = key_get_char("some_key", random_data);
		if (rc)
			free(rc);
		key_get_int("some_key", random_data);
		key_get_bool("some_key", random_data);
		key_get_float("some_key", random_data);
		key_get_long("some_key", random_data);
		key_get_int_array("some_key", random_data, garbage_i_arr, 10);
		key_get_float_array("some_key", random_data, garbage_f_arr, 10);

		char *rs_arr[5] = { 0 };
		int rs_len = key_get_char_array("some_key", random_data, rs_arr, 5);
		for (int j = 0; j < rs_len; j++)
			free(rs_arr[j]);
	}
	printf("Passed %d iterations of random legal string fuzzing without crashing.\n", 128 * 128);
	// Random binary data (multiple iterations)
	printf("=> Testing with multiple random legal string iterations...\n");
	for (int iter = 0; iter < 128 * 128; iter++) {
		srand(iter); // Seed with iteration for reproducibility
		int r_len = rand() % 1023 + 1;
		for (int i = 0; i < r_len; i++) {
			// A legal character from ASCII range, but still random
			int r = rand() % 127 + 1;
			random_data[i] = (char)r;
		}
		random_data[r_len] = '\0';
		// Add `\n` and `=` to increase chances of triggering parsing logic
		for (int i = 0; i < 64; i++) {
			random_data[rand() % r_len] = '\n';
			random_data[rand() % r_len] = '=';
			random_data[rand() % r_len] = '"';
			random_data[rand() % r_len] = '[';
			random_data[rand() % r_len] = ']';
			random_data[rand() % r_len] = ',';
			random_data[rand() % r_len] = '\\';
			random_data[rand() % r_len] = '#';
		}
		char *rc = key_get_char("some_key", random_data);
		if (rc)
			free(rc);
		key_get_int("some_key", random_data);
		key_get_bool("some_key", random_data);
		key_get_float("some_key", random_data);
		key_get_long("some_key", random_data);
		key_get_int_array("some_key", random_data, garbage_i_arr, 10);
		key_get_float_array("some_key", random_data, garbage_f_arr, 10);

		char *rs_arr[5] = { 0 };
		int rs_len = key_get_char_array("some_key", random_data, rs_arr, 5);
		for (int j = 0; j < rs_len; j++)
			free(rs_arr[j]);
	}
	printf("Passed %d iterations of random legal string fuzzing without crashing.\n", 128 * 128);
	// ---------------------------------------------------------
	// 5. Null Byte Injections
	// ---------------------------------------------------------
	printf("=> Testing Null Byte Injections...\n");

	// Test 5A: Pure Null Bytes Injection in strings
	char null_test_str[] = "key=[\"val1\0\", \"val2\"]\nkey2=\"\0\0\0\"\n";
	char *nc = key_get_char("key", null_test_str);
	if (nc)
		free(nc);

	char *nc_arr[5] = { 0 };
	int nc_len = key_get_char_array("key", null_test_str, nc_arr, 5);
	for (int i = 0; i < nc_len; i++)
		free(nc_arr[i]);

	printf("=> All tests successfully passed! Coverage across parsing, generation, and edge cases verified.\n");
	return 0;
}