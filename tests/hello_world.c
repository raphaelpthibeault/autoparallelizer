#include <stdio.h>

void
baz(char *c) {
    printf("%s\n", c);
}

void
foo() {
    printf("foo\n");
}

void
bar() {
    printf("bar\n");
}

void
multicall2() {
    foo();
}

void
multicall1() {
    multicall2();
}

int
main() {
    printf("Hello, world!\n");
    baz("BAZ");
    foo();
    bar();
    multicall1();
    return 0;
}
