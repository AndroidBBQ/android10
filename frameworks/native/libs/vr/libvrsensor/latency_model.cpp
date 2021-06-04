#include <private/dvr/latency_model.h>

#include <cmath>

namespace android {
namespace dvr {

LatencyModel::LatencyModel(size_t window_size) : window_size_(window_size) {}

void LatencyModel::AddLatency(int64_t latency_ns) {
  // Not enough samples yet?
  if (num_summed_ < window_size_) {
    // Accumulate.
    latency_sum_ += latency_ns;

    // Have enough samples for latency estimate?
    if (++num_summed_ == window_size_) {
      latency_ = latency_sum_ / window_size_;
    }
  }
}

}  // namespace dvr
}  // namespace android
