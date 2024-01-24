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

inline int add1(int a, int b) { return (a + b); }

__attribute__((noinline)) int add2(int a, int b) { return (a + b); }

class Impl0 {
public:
  int add(int a, int b) {
    return (a + b);
  }
};

class Impl1 {
public:
  virtual int add(int a, int b) {
    return (a + b);
  }
};

class Impl2 : public Impl1 {
public:
  int add(int a, int b) { return (a + b); }

  int virtual_add(int a, int b) { return Impl1::add(a, b); }
};


class Impl3 : public Impl2 {
public:
  int local_add(int a, int b) { return (a + b); }
};

int main(void) {
#if 0
    {
        auto timestamp = getticks();
        for (auto i = 0ULL; i < 10e6; i++) {
            volatile int sum = add1(i, i+1);
        }
        std::cout << (getticks() - timestamp) << "\n";
    }
    {
        auto timestamp = getticks();
        for (auto i = 0ULL; i < 10e6; i++) {
            volatile int sum = add2(i, i+1);
        }
        std::cout << (getticks() - timestamp) << "\n";
    }
    {
        Interface obj;
        auto timestamp = getticks();
        for (auto i = 0ULL; i < 10e7; i++) {
            volatile int sum = obj.add(i, i+1);
        }
        std::cout << (getticks() - timestamp) << "\n";
    }

    {
        Impl obj;
        auto timestamp = getticks();
        for (auto i = 0ULL; i < 10e7; i++) {
            volatile int sum = obj.add(i, i+1);
        }
        std::cout << (getticks() - timestamp) << "\n";
    }

    {
        Impl2 obj;
        auto timestamp = getticks();
        for (auto i = 0ULL; i < 10e7; i++) {
            volatile int sum = obj.add(i, i+1);
        }
        std::cout << (getticks() - timestamp) << "\n";
    }
#endif

  {
    Impl0 obj;
    long double latency = 0;
    volatile int sum = 0;
    PerfTimer timer;
    for (auto i = 0ULL; i < 1000; i++) {
      flush_cache();
      //   auto timestamp = getticks();
      timer.Start();

      sum += obj.add(0, 1);
      latency += timer.Stop();
      // latency += (getticks() - timestamp);
    }
    std::cout << "regular : " << latency << " " << sum << "\n";
  }
  {
    Impl3 obj;
    long double latency = 0;
    volatile int sum = 0;
    PerfTimer timer;
    for (auto i = 0ULL; i < 1000; i++) {
      flush_cache();
      timer.Start();
      // auto timestamp = getticks();
      sum += obj.add(0, 1);
      // latency += (getticks() - timestamp);
      latency += timer.Stop();
    }
    std::cout << "virtual: " << latency << " " << sum << "\n";
  }

  cleanup();

  return 0;
}
