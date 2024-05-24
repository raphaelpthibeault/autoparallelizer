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

void
recursive(int count) {
    if (count <= 1) {
        printf("bye\n");
    } else {
        multicall1();
        recursive(count -= 1);
    }
}

int
main() {
    printf("Hello, world!\n");
    baz("BAZ");
    foo();
    bar();
    recursive(10);
    multicall1();
    return 0;
}
