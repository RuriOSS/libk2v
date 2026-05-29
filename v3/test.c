#include "./k2v.h"
int main()
{
	char *buf = k2v3_open_file("test.conf");
	k2v3_cache cache = k2v3_parse(buf);
	k2v3_dump(cache);
	k2v3_free_cache(&cache);
	free(buf);
	return 0;
}