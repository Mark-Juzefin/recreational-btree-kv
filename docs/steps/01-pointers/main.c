//
// Created by Illia on 17.04.2026.
//

#include <stdio.h>

void increment(int *x) {
  *x = *x + 1;
}

int sum(int *arr, size_t n) {
  int s = 0;
  for (size_t i = 0; i < n; i++) {
    s += arr[i];
  }
  return s;
}

void reverse(int *arr, size_t n) {
  size_t left = 0;
  size_t right = n - 1;
  while (left < right) {
    int tmp = arr[left];
    arr[left] = arr[right];
    arr[right] = tmp;
    left++;
    right--;
  }
}

int main(void) {
  int n = 5;
  increment(&n);
  printf("increment: %d\n", n);

  int arr[5] = {10, 20, 30, 40, 50};
  printf("sum: %d\n", sum(arr, 5));

  int rev[5] = {1, 2, 3, 4, 5};
  reverse(rev, 5);
  printf("reverse:");
  for (size_t i = 0; i < 5; i++) {
    printf(" %d", rev[i]);
  }
  printf("\n");

  return 0;
}

