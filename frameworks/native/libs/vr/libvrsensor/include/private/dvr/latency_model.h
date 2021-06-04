#ifndef ANDROID_DVR_LATENCY_MODEL_H_
#define ANDROID_DVR_LATENCY_MODEL_H_

#include <vector>

namespace android {
namespace dvr {

// This class models the latency from sensors. It will look at the first
// window_size measurements and return their average after that.
class LatencyModel {
 public:
  explicit LatencyModel(size_t window_size);
  ~LatencyModel() = default;

  void AddLatency(int64_t latency_ns);
  int64_t CurrentLatencyEstimate() const { return latency_; }

 private:
  size_t window_size_;
  int64_t latency_sum_ = 0;
  size_t num_summed_ = 0;
  int64_t latency_ = 0;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_LATENCY_MODEL_H_
