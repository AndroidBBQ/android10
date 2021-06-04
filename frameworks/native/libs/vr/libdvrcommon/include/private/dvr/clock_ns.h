#ifndef ANDROID_DVR_CLOCK_NS_H_
#define ANDROID_DVR_CLOCK_NS_H_

#include <stdint.h>
#include <time.h>

namespace android {
namespace dvr {

constexpr int64_t kNanosPerSecond = 1000000000ll;

// Returns the standard Dream OS monotonic system time that corresponds with all
// timestamps found in Dream OS APIs.
static inline timespec GetSystemClock() {
  timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return t;
}

static inline timespec GetSystemClockRaw() {
  timespec t;
  clock_gettime(CLOCK_MONOTONIC_RAW, &t);
  return t;
}

static inline int64_t GetSystemClockNs() {
  timespec t = GetSystemClock();
  int64_t ns = kNanosPerSecond * (int64_t)t.tv_sec + (int64_t)t.tv_nsec;
  return ns;
}

static inline int64_t GetSystemClockRawNs() {
  timespec t = GetSystemClockRaw();
  int64_t ns = kNanosPerSecond * (int64_t)t.tv_sec + (int64_t)t.tv_nsec;
  return ns;
}

static inline double NsToSec(int64_t nanoseconds) {
  return nanoseconds / static_cast<double>(kNanosPerSecond);
}

static inline double GetSystemClockSec() { return NsToSec(GetSystemClockNs()); }

static inline double GetSystemClockMs() { return GetSystemClockSec() * 1000.0; }

// Converts a nanosecond timestamp to a timespec. Based on the kernel function
// of the same name.
static inline timespec NsToTimespec(int64_t ns) {
  timespec t;
  int32_t remainder;

  t.tv_sec = ns / kNanosPerSecond;
  remainder = ns % kNanosPerSecond;
  if (remainder < 0) {
    t.tv_nsec--;
    remainder += kNanosPerSecond;
  }
  t.tv_nsec = remainder;

  return t;
}

// Timestamp comparison functions that handle wrapping values correctly.
static inline bool TimestampLT(int64_t a, int64_t b) {
  return static_cast<int64_t>(static_cast<uint64_t>(a) -
                              static_cast<uint64_t>(b)) < 0;
}
static inline bool TimestampLE(int64_t a, int64_t b) {
  return static_cast<int64_t>(static_cast<uint64_t>(a) -
                              static_cast<uint64_t>(b)) <= 0;
}
static inline bool TimestampGT(int64_t a, int64_t b) {
  return static_cast<int64_t>(static_cast<uint64_t>(a) -
                              static_cast<uint64_t>(b)) > 0;
}
static inline bool TimestampGE(int64_t a, int64_t b) {
  return static_cast<int64_t>(static_cast<uint64_t>(a) -
                              static_cast<uint64_t>(b)) >= 0;
}

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_CLOCK_NS_H_
