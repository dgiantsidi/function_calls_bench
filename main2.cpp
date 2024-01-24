#include <chrono>
#include <inttypes.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

typedef unsigned long long ticks;

static __inline__ ticks getticks(void) {
  unsigned a, d;
  asm volatile("rdtsc" : "=a"(a), "=d"(d));
  return ((ticks)a) | (((ticks)d) << 32);
}

class PerfTimer {
private:
  std::chrono::high_resolution_clock::time_point startTime;

public:
  // Starts the timer
  void Start() { startTime = std::chrono::high_resolution_clock::now(); }

  // Stops the timer, returning the elapsed time since start in seconds.
  double Stop() {
    auto endTime = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(endTime -
                                                                     startTime)
        .count();
  }
};

volatile int *tmp = nullptr;

void flush_cache() {
  constexpr int kBlockSize = 1024 * 1024 * 4;
  if (!tmp) {
    tmp = new int[kBlockSize];
  }

  for (int i = 0; i < kBlockSize; ++i) {
    tmp[i] = rand();
  }
}

void cleanup() {
  if (tmp)
    delete[] tmp;
}

class Impl0 {
public:
  int add1(int a, int b) { return (a + b); }
  int add2(int a, int b) { return (a + b); }
  int add3(int a, int b) { return (a + b); }
};

class Impl1 {
public:
  virtual int add1(int a, int b) { return (a + b); }

  virtual int add2(int a, int b) { return (a + b); }

  virtual int add3(int a, int b) { return (a + b); }
};

class Impl2 : public Impl1 {
public:
};

class Impl3 : public Impl2 {
public:
  virtual int add1(int a, int b) { return (a + b); }
};

int func_calls(Impl1 *obj, int i) {
  if (i % 10 == 1) {
    return obj->add1(1, 1);
  } else if (i % 10 == 2) {

    return obj->add2(2, 1) + obj->add3(3, 1);
  } else if (i % 10 == 3)
    return obj->add3(3, 1);
  else
    return obj->add2(2, 1) + obj->add3(3, 1) + obj->add1(1, 1);
}

int func_calls(Impl0 *obj, int i) {
  if (i % 10 == 1) {
    return obj->add1(1, 1);
  } else if (i % 10 == 2) {

    return obj->add2(2, 1) + obj->add3(3, 1);
  } else if (i % 10 == 3)
    return obj->add3(3, 1);
  else
    return obj->add2(2, 1) + obj->add3(3, 1) + obj->add1(1, 1);
}

int main(void) {
  srand((unsigned)time(NULL));
  {
    Impl0 obj1;
    Impl0 obj2;
    long double latency = 0;
    volatile int sum = 0;
    PerfTimer timer;
    for (auto i = 0ULL; i < 1000; i++) {
      // flush_cache();
      //   auto timestamp = getticks();
      timer.Start();

      sum += func_calls(&obj1, rand());
      sum += func_calls(&obj2, rand());
      latency += timer.Stop();
      // latency += (getticks() - timestamp);
    }
    std::cout << "regular : " << latency << " " << sum << "\n";
  }
  {
    Impl3 obj1;
    Impl3 obj2;
    long double latency = 0;
    volatile int sum = 0;
    PerfTimer timer;
    for (auto i = 0ULL; i < 1000; i++) {
      // flush_cache();
      timer.Start();
      // auto timestamp = getticks();
      sum += func_calls(&obj1, rand());
      sum += func_calls(&obj2, rand());
      // latency += (getticks() - timestamp);
      latency += timer.Stop();
    }
    std::cout << "virtual: " << latency << " " << sum << "\n";
  }

  cleanup();

  return 0;
}
