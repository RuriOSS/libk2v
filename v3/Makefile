all:
	cc -std=gnu99 -fno-omit-frame-pointer -Wno-gnu-zero-variadic-macro-arguments -fno-stack-protector -Wall -Wextra -pedantic -Wconversion -Wno-newline-eof -fsanitize=address,undefined -g -O0 *.c
fuzz:
	clang++ -fprofile-instr-generate -fcoverage-mapping -g -O0 -fno-omit-frame-pointer -Wno-gnu-zero-variadic-macro-arguments -fno-stack-protector -Wall -Wextra -pedantic -Wconversion -Wno-newline-eof -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer -Isrc/include -Wno-gnu-zero-variadic-macro-arguments -x c k2v3.c -x c fuzz_parse.c