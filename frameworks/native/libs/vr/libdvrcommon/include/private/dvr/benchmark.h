#ifndef ANDROID_DVR_BENCHMARK_H_
#define ANDROID_DVR_BENCHMARK_H_

#include <stdio.h>
#include <time.h>

#include <cutils/trace.h>

#include <private/dvr/clock_ns.h>

// Set benchmark traces, using Android systrace.
//
// The simplest one-parameter version of btrace automatically sets the
// timestamp with the system clock. The other versions can optionally set the
// timestamp manually, or pass additional data to be written to the log line.
//
// Example:
// Btrace("Start execution");
// ... code to benchmark ...
// Btrace("End execution");
//
// Use compute_benchmarks.py
// with the trace path "Start execution,End execution",
// to report the elapsed time between the two calls.
//
// Btrace will either output to standard atrace, or to a file if specified.
// The versions BtraceData also allow an int64_t to be included in the trace.

// Btrace without data payload.
static inline void Btrace(const char* name, int64_t nanoseconds_monotonic);
static inline void Btrace(const char* name);
static inline void Btrace(FILE* file, const char* name,
                          int64_t nanoseconds_monotonic);
static inline void Btrace(FILE* file, const char* name);

// Btrace with data payload.
static inline void BtraceData(const char* name, int64_t nanoseconds_monotonic,
                              int64_t data);
static inline void BtraceData(const char* name, int64_t data);
static inline void BtraceData(FILE* file, const char* name,
                              int64_t nanoseconds_monotonic, int64_t data);
static inline void BtraceData(FILE* file, const char* name, int64_t data);

static inline void Btrace(const char* name, int64_t nanoseconds_monotonic) {
  const int kLogMessageLength = 256;
  char log_message[kLogMessageLength];
  snprintf(log_message, kLogMessageLength, "#btrace#%s", name);
  atrace_int64(ATRACE_TAG_WEBVIEW, log_message, nanoseconds_monotonic);
}

static inline void Btrace(const char* name) {
  Btrace(name, android::dvr::GetSystemClockNs());
}

static inline void Btrace(FILE* file, const char* name,
                          int64_t nanoseconds_monotonic) {
  fprintf(file, "#btrace#%s|%" PRId64 "\n", name, nanoseconds_monotonic);
}

static inline void Btrace(FILE* file, const char* name) {
  Btrace(file, name, android::dvr::GetSystemClockNs());
}

static inline void BtraceData(const char* name, int64_t nanoseconds_monotonic,
                              int64_t data) {
  const int kLogMessageLength = 256;
  char log_message[kLogMessageLength];
  snprintf(log_message, kLogMessageLength, "#btrace#%s|%" PRId64, name, data);
  atrace_int64(ATRACE_TAG_WEBVIEW, log_message, nanoseconds_monotonic);
}

static inline void BtraceData(const char* name, int64_t data) {
  BtraceData(name, android::dvr::GetSystemClockNs(), data);
}

static inline void BtraceData(FILE* file, const char* name,
                              int64_t nanoseconds_monotonic, int64_t data) {
  fprintf(file, "#btrace#%s|%" PRId64 "|%" PRId64 "\n", name, data,
          nanoseconds_monotonic);
}

static inline void BtraceData(FILE* file, const char* name, int64_t data) {
  BtraceData(file, name, android::dvr::GetSystemClockNs(), data);
}

#endif  // ANDROID_DVR_BENCHMARK_H_
