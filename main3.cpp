#include <chrono>
#include <inttypes.h>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

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
  {
    std::vector<std::unique_ptr<Impl0>> vec;
    for (auto i = 0ULL; i < 20e6; i++) {
      vec.emplace_back(std::move(std::make_unique<Impl0>()));
    }
    PerfTimer timer;
    long double latency = 0;
    timer.Start();
    for (auto i = 0ULL; i < 20e6; i++) {
      func_calls(vec[i].get(), i);
    }
    latency += timer.Stop();

    std::cout << "regular : " << latency << "\n";
  }
  {
    std::vector<std::unique_ptr<Impl3>> vec;
    for (auto i = 0ULL; i < 20e6; i++) {
      vec.emplace_back(std::move(std::make_unique<Impl3>()));
    }
    PerfTimer timer;
    long double latency = 0;
    timer.Start();
    for (auto i = 0ULL; i < 20e6; i++) {
      // vec[i]->add1(1,1);
      func_calls(vec[i].get(), i);
    }
    latency += timer.Stop();

    std::cout << "VIRTUAL : " << latency << "\n";
  }

  return 0;
}
