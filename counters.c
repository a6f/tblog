#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

volatile uint32_t counter[2];

void maintain_counter() {
  uint32_t hi = counter[1];
  uint32_t lo = counter[0];
  // Check that the counter hasn't gone more than 1/4th of the way around since we were last called.
  static uint32_t last = 0;  // nonzero once set
  assert(last == 0 || ((lo | 1) - last) <= 1<<30);
  last = lo | 1;
  // XOR LSB of counter[1] with MSB of counter[0].
  uint32_t diff = (hi & 1) ^ (lo >> 31);
  // If they differ, incrementing counter[1] will make them match.
  counter[1] += diff;
}

int64_t read_counter() {
  uint32_t hi = counter[1];
  uint32_t lo = counter[0];
  // Imitate maintenance update.
  uint32_t diff = (hi & 1) ^ (lo >> 31);
  hi += diff;
  // Combine words.  The one overlapping bit will match.
  return ((int64_t)hi) << 31 | lo;
}

uint32_t rdtsc() {
  uint32_t a = 0;
  asm volatile("rdtsc":"=a"(a)::"edx");
  return a;
}

int main() {
  for (;;) {
    usleep(100000);
    counter[0] = rdtsc();
    maintain_counter();
    int64_t a = read_counter();
    printf("%ld\n", a);
    usleep(100000);
    counter[0] = rdtsc();
    int64_t b = read_counter();
    maintain_counter();
    int64_t c = read_counter();
    printf("%ld\n", b);
    assert(b == c);
    if (b <= a) return 1;
  }
}
