#ifndef ANDROID_PDX_TRACE_H_
#define ANDROID_PDX_TRACE_H_

#include <array>

#include <utils/Trace.h>

// Enables internal tracing in libpdx. This is disabled by default to avoid
// spamming the trace buffers during normal trace activities. libpdx must be
// built with this set to true to enable internal tracing.
#ifndef PDX_LIB_TRACE_ENABLED
#define PDX_LIB_TRACE_ENABLED false
#endif

namespace android {
namespace pdx {

// Utility to generate scoped tracers with arguments.
class ScopedTraceArgs {
 public:
  template <typename... Args>
  ScopedTraceArgs(uint64_t tag, const char* format, Args&&... args)
      : tag_{tag} {
    if (atrace_is_tag_enabled(tag_)) {
      std::array<char, 1024> buffer;
      snprintf(buffer.data(), buffer.size(), format,
               std::forward<Args>(args)...);
      atrace_begin(tag_, buffer.data());
    }
  }

  ~ScopedTraceArgs() { atrace_end(tag_); }

 private:
  uint64_t tag_;

  ScopedTraceArgs(const ScopedTraceArgs&) = delete;
  void operator=(const ScopedTraceArgs&) = delete;
};

// Utility to generate scoped tracers.
class ScopedTrace {
 public:
  template <typename... Args>
  ScopedTrace(uint64_t tag, bool enabled, const char* name)
      : tag_{tag}, enabled_{enabled} {
    if (enabled_)
      atrace_begin(tag_, name);
  }

  ~ScopedTrace() {
    if (enabled_)
      atrace_end(tag_);
  }

 private:
  uint64_t tag_;
  bool enabled_;

  ScopedTrace(const ScopedTrace&) = delete;
  void operator=(const ScopedTrace&) = delete;
};

}  // namespace pdx
}  // namespace android

// Macro to define a scoped tracer with arguments. Uses PASTE(x, y) macro
// defined in utils/Trace.h.
#define PDX_TRACE_FORMAT(format, ...)                         \
  ::android::pdx::ScopedTraceArgs PASTE(__tracer, __LINE__) { \
    ATRACE_TAG, format, ##__VA_ARGS__                         \
  }

// TODO(eieio): Rename this to PDX_LIB_TRACE_NAME() for internal use by libpdx
// and rename internal uses inside the library. This version is only enabled
// when PDX_LIB_TRACE_ENABLED is true.
#define PDX_TRACE_NAME(name)                              \
  ::android::pdx::ScopedTrace PASTE(__tracer, __LINE__) { \
    ATRACE_TAG, PDX_LIB_TRACE_ENABLED, name               \
  }

#endif  // ANDROID_PDX_TRACE_H_
