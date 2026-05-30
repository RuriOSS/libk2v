#include "k2v3.h"
#include "../src/include/k2v.h"
int main(int __attribute((unused)) argc, char *argv[])
{
	k2v3_stop_at_warning(0);
	char *buf = k2v3_open_file(argv[1]);
	if (!buf) {
		printf("Failed to open file: %s\n", argv[1]);
		return 1;
	}
	k2v3_cache cache = k2v3_parse(buf);
	switch (k2v3_errno(K2V3_QUERY)) {
	case K2V3_UNRECOVERABLE:
		printf("Unrecoverable error occurred during parsing.\n");
		break;
	case K2V3_RECOVERABLE:
		printf("Recoverable error occurred during parsing.\n");
		break;
	default:
		printf("No errors occurred during parsing.\n");
		break;
	}
	k2v3_dump(cache);
	k2v3_free_cache(&cache);
	free(buf);
	return 0;
}