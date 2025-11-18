#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>

using String = std::string;

template <typename T>
T constrain(T x, T a, T b) {
  return std::min(std::max(x, a), b);
}

// Convenience overload to handle mixed numeric types
template <typename T, typename A, typename B>
T constrain(T x, A a, B b) {
  return std::min<T>(std::max<T>(x, static_cast<T>(a)),
                     static_cast<T>(b));
}

class HardwareSerial {
 public:
  template <typename T>
  void println(const T&) {}

  template <typename T>
  void print(const T&) {}

  template <typename... Args>
  void printf(const char*, Args...) {}
};

extern HardwareSerial Serial;

inline unsigned long millis() {
  static unsigned long counter = 0;
  return ++counter;
}

inline void delay(unsigned long) {}

inline void ledcWrite(uint8_t, uint32_t) {}
