int __attribute__((noinline)) add(int a, int b) { return a + b; }

#include <stdio.h>
int main() {
  int a = 5;
  int b = 7;
  int c = add(a, b);
  printf("%d\n", c);
  return 0;
}
