#include <stdio.h>
#include <stdlib.h>

struct structA {
    int z;
    int zz;
    int zzz;
};

void
baz(int x) {
    printf("%d\n", x);
}

void
spam() {
    int a, b = 10, i;
    a = 5;

    if (a < 5) {
        int if_statement_var = 123;
        printf("a is less than 5\n");
    } else {
        int else_statement_var = 456;
        printf("a is not less than 5\n");
    }

    int c = 0, d;
    while (a >= 1) {
        c += 10;
        d = c / 2;
        int x = 100;
        printf("d = '%d'\n", d);
        baz(c);
        a--;
    }

    int e = 0, f;
    int arr[10][10];
    for (i = 0; i < 10; ++i) {
        e += 2;
        f = e - i;
        int y = 200;
        printf("%d ", f);
        baz(e);

        for (int j = 0; j < 10; ++j) {
            arr[i][j] = y;
        }
    }
    printf("\n");
    int arr2[100][2];
    int arr3[a][b][c];

    for (int k = 0; k < 100; ++k) {
        int a1 = 1;
        int a2 = 2;
        int a3 = 3;
        int sum = a1 + a2 + a3;
        printf("%d\n", sum);

        arr2[k][0] = sum;
        arr2[k][a1] = a2;
        arr2[k][a1 * a2 * a3];
    }

    a = 0;
    b = 0;
    c = 0;
    d = 0;
    i = 0;
    int asdf;

    for (int qqq = 0; qqq < 123; ++qqq) {
        int a1 = 1;
        int a2 = 2;
        int a3 = 3;
        int sum = a1 + a2 + a3;
        printf("%d\n", sum);
    }
}

int
return_10() {
    return 10;
}

int
return_in(int in) {
    return in;
}

int ham() {
    struct structA *a = (struct structA*)malloc(sizeof(struct structA));
    char *str = (char *)malloc(10 * sizeof(char));
    a->z = 10;
    a->zz = 20;
    a->zzz = 30;
    int res = (a->zz + a->zzz + ((a->z + a->zzz) + (a->z + a->zz) - a->z)) * a->zzz;
    printf("%d\n", res);
    int arr2[100][2];

    for (int k = 0; k < 100; ++k) {
        int a1 = 1;
        int a2 = 2;
        int a3 = 3;
        int sum = a1 + a2 + a3;
        printf("%d\n", sum);

        arr2[k][0] = sum;
        arr2[k][a1] = a2;
        arr2[k][a1 * a2 * a3];
    }

    for (int i = 0; i < 100; ++i) {
        int a1 = 1;
        int a2 = 2;
        int a3 = 3;
        int sum = a1 + a2 + a3;
        printf("%d\n", sum);
    }

    for (int i = 0; i < 100; ++i) {
        int a1 = 1;
        int a2 = 2;
        int a3 = 3;
        int sum = a1 + a2 + a3;
        printf("%d\n", sum);
    }

    return res;
}

int
main() {
    spam();
    ham();
    int x = return_10();
    int y = return_in(x);
    return 0;
}
