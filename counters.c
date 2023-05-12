#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define bool _Bool

const int W = (8 * sizeof(unsigned));

volatile unsigned counter[2];

void maintain_counter() {
  unsigned hi = counter[1];
  unsigned lo = counter[0];
  // Check that the counter hasn't gone more than 1/4th of the way around since we were last called.
  static unsigned last = 0;  // nonzero once set
  assert(last == 0 || ((lo | 1) - last) >> (W-2) == 0);
  last = lo | 1;
  // XOR LSB of counter[1] with MSB of counter[0].
  unsigned diff = (hi & 1) ^ (lo >> (W-1));
  // If they differ, incrementing counter[1] will make them match.
  counter[1] += diff;
}

long read_counter() {
  unsigned hi = counter[1];
  unsigned lo = counter[0];
  // Imitate maintenance update.
  unsigned diff = (hi & 1) ^ (lo >> (W-1));
  hi += diff;
  // Combine words.  The one overlapping bit will match.
  return ((long)hi) << (W-1) | lo;
}

unsigned rdtsc() {
  unsigned a = 0;
  asm volatile("rdtsc":"=a"(a)::"edx");
  return a;
}

int main() {
  for (;;) {
    usleep(100000);
    counter[0] = rdtsc();
    maintain_counter();
    long a = read_counter();
    printf("%ld\n", a);
    usleep(100000);
    counter[0] = rdtsc();
    long b = read_counter();
    maintain_counter();
    long c = read_counter();
    printf("%ld\n", b);
    assert(b == c);
    if (b <= a) return 1;
  }
}
