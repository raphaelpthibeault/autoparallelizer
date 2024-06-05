#include <stdio.h>
#include <omp.h>

#define SIZE 1000

void calculate_sum(int *arr, int size, long long *sum) {
    *sum = 0;
    for (int i = 0; i < size; i++) {
        *sum += arr[i];
    }
}

void find_max(int *arr, int size, int *max) {
    *max = arr[0];
    for (int i = 1; i < size; i++) {
        if (arr[i] > *max) {
            *max = arr[i];
        }
    }
}

void find_min(int *arr, int size, int *min) {
    *min = arr[0];
    for (int i = 1; i < size; i++) {
        if (arr[i] < *min) {
            *min = arr[i];
        }
    }
}

int main() {
    int arr1[SIZE], arr2[SIZE], arr3[SIZE];
    long long sum;
    int max, min;

    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i + 1;
        arr2[i] = i + 1;
        arr3[i] = i + 1;
    }

    // Section 1
    calculate_sum(arr1, SIZE, &sum);
    printf("Sum: %lld\n", sum);

    // Section 2
    find_max(arr2, SIZE, &max);
    printf("Max: %d\n", max);

    // Section 3
    find_min(arr3, SIZE, &min);
    printf("Min: %d\n", min);

    return 0;
}
