#include "./k2v.h"
int main(){
    char *buf = k2v_open_file("test.conf");
    k2v_cache cache = k2v_parse(buf);
    k2v_dump(cache);
    k2v_free_cache(&cache);
    free(buf);
    return 0;
}