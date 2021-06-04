#include "hardware_composer.h"

#include <binder/IServiceManager.h>
#include <cutils/properties.h>
#include <cutils/sched_policy.h>
#include <fcntl.h>
#include <log/log.h>
#include <poll.h>
#include <stdint.h>
#include <sync/sync.h>
#include <sys/eventfd.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/system_properties.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <utils/Trace.h>

#include <algorithm>
#include <chrono>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <tuple>

#include <dvr/dvr_display_types.h>
#include <dvr/performance_client_api.h>
#include <private/dvr/clock_ns.h>
#include <private/dvr/ion_buffer.h>

using android::hardware::Return;
using android::hardware::Void;
using android::pdx::ErrorStatus;
using android::pdx::LocalHandle;
using android::pdx::Status;
using android::pdx::rpc::EmptyVariant;
using android::pdx::rpc::IfAnyOf;

using namespace std::chrono_literals;

namespace android {
namespace dvr {

namespace {

const char kDvrPerformanceProperty[] = "sys.dvr.performance";
const char kDvrStandaloneProperty[] = "ro.boot.vr";

const char kRightEyeOffsetProperty[] = "dvr.right_eye_offset_ns";

const char kUseExternalDisplayProperty[] = "persist.vr.use_external_display";

// Surface flinger uses "VSYNC-sf" and "VSYNC-app" for its version of these
// events. Name ours similarly.
const char kVsyncTraceEventName[] = "VSYNC-vrflinger";

// How long to wait after boot finishes before we turn the display off.
constexpr int kBootFinishedDisplayOffTimeoutSec = 10;

constexpr int kDefaultDisplayWidth = 1920;
constexpr int kDefaultDisplayHeight = 1080;
constexpr int64_t kDefaultVsyncPeriodNs = 16666667;
// Hardware composer reports dpi as dots per thousand inches (dpi * 1000).
constexpr int kDefaultDpi = 400000;

// Get time offset from a vsync to when the pose for that vsync should be
// predicted out to. For example, if scanout gets halfway through the frame
// at the halfway point between vsyncs, then this could be half the period.
// With global shutter displays, this should be changed to the offset to when
// illumination begins. Low persistence adds a frame of latency, so we predict
// to the center of the next frame.
inline int64_t GetPosePredictionTimeOffset(int64_t vsync_period_ns) {
  return (vsync_period_ns * 150) / 100;
}

// Attempts to set the scheduler class and partiton for the current thread.
// Returns true on success or false on failure.
bool SetThreadPolicy(const std::string& scheduler_class,
                     const std::string& partition) {
  int error = dvrSetSchedulerClass(0, scheduler_class.c_str());
  if (error < 0) {
    ALOGE(
        "SetThreadPolicy: Failed to set scheduler class \"%s\" for "
        "thread_id=%d: %s",
        scheduler_class.c_str(), gettid(), strerror(-error));
    return false;
  }
  error = dvrSetCpuPartition(0, partition.c_str());
  if (error < 0) {
    ALOGE(
        "SetThreadPolicy: Failed to set cpu partiton \"%s\" for thread_id=%d: "
        "%s",
        partition.c_str(), gettid(), strerror(-error));
    return false;
  }
  return true;
}

// Utility to generate scoped tracers with arguments.
// TODO(eieio): Move/merge this into utils/Trace.h?
class TraceArgs {
 public:
  template <typename... Args>
  explicit TraceArgs(const char* format, Args&&... args) {
    std::array<char, 1024> buffer;
    snprintf(buffer.data(), buffer.size(), format, std::forward<Args>(args)...);
    atrace_begin(ATRACE_TAG, buffer.data());
  }

  ~TraceArgs() { atrace_end(ATRACE_TAG); }

 private:
  TraceArgs(const TraceArgs&) = delete;
  void operator=(const TraceArgs&) = delete;
};

// Macro to define a scoped tracer with arguments. Uses PASTE(x, y) macro
// defined in utils/Trace.h.
#define TRACE_FORMAT(format, ...) \
  TraceArgs PASTE(__tracer, __LINE__) { format, ##__VA_ARGS__ }

// Returns "primary" or "external". Useful for writing more readable logs.
const char* GetDisplayName(bool is_primary) {
  return is_primary ? "primary" : "external";
}

}  // anonymous namespace

HardwareComposer::HardwareComposer()
    : initialized_(false), request_display_callback_(nullptr) {}

HardwareComposer::~HardwareComposer(void) {
  UpdatePostThreadState(PostThreadState::Quit, true);
  if (post_thread_.joinable())
    post_thread_.join();
  composer_callback_->SetVsyncService(nullptr);
}

bool HardwareComposer::Initialize(
    Hwc2::Composer* composer, hwc2_display_t primary_display_id,
    RequestDisplayCallback request_display_callback) {
  if (initialized_) {
    ALOGE("HardwareComposer::Initialize: already initialized.");
    return false;
  }

  is_standalone_device_ = property_get_bool(kDvrStandaloneProperty, false);

  request_display_callback_ = request_display_callback;

  primary_display_ = GetDisplayParams(composer, primary_display_id, true);

  vsync_service_ = new VsyncService;
  sp<IServiceManager> sm(defaultServiceManager());
  auto result = sm->addService(String16(VsyncService::GetServiceName()),
      vsync_service_, false);
  LOG_ALWAYS_FATAL_IF(result != android::OK,
      "addService(%s) failed", VsyncService::GetServiceName());

  post_thread_event_fd_.Reset(eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK));
  LOG_ALWAYS_FATAL_IF(
      !post_thread_event_fd_,
      "HardwareComposer: Failed to create interrupt event fd : %s",
      strerror(errno));

  post_thread_ = std::thread(&HardwareComposer::PostThread, this);

  initialized_ = true;

  return initialized_;
}

void HardwareComposer::Enable() {
  UpdatePostThreadState(PostThreadState::Suspended, false);
}

void HardwareComposer::Disable() {
  UpdatePostThreadState(PostThreadState::Suspended, true);

  std::unique_lock<std::mutex> lock(post_thread_mutex_);
  post_thread_ready_.wait(lock, [this] {
    return !post_thread_resumed_;
  });
}

void HardwareComposer::OnBootFinished() {
  std::lock_guard<std::mutex> lock(post_thread_mutex_);
  if (boot_finished_)
    return;
  boot_finished_ = true;
  post_thread_wait_.notify_one();
  if (is_standalone_device_)
    request_display_callback_(true);
}

// Update the post thread quiescent state based on idle and suspended inputs.
void HardwareComposer::UpdatePostThreadState(PostThreadStateType state,
                                             bool suspend) {
  std::unique_lock<std::mutex> lock(post_thread_mutex_);

  // Update the votes in the state variable before evaluating the effective
  // quiescent state. Any bits set in post_thread_state_ indicate that the post
  // thread should be suspended.
  if (suspend) {
    post_thread_state_ |= state;
  } else {
    post_thread_state_ &= ~state;
  }

  const bool quit = post_thread_state_ & PostThreadState::Quit;
  const bool effective_suspend = post_thread_state_ != PostThreadState::Active;
  if (quit) {
    post_thread_quiescent_ = true;
    eventfd_write(post_thread_event_fd_.Get(), 1);
    post_thread_wait_.notify_one();
  } else if (effective_suspend && !post_thread_quiescent_) {
    post_thread_quiescent_ = true;
    eventfd_write(post_thread_event_fd_.Get(), 1);
  } else if (!effective_suspend && post_thread_quiescent_) {
    post_thread_quiescent_ = false;
    eventfd_t value;
    eventfd_read(post_thread_event_fd_.Get(), &value);
    post_thread_wait_.notify_one();
  }
}

void HardwareComposer::CreateComposer() {
  if (composer_)
    return;
  composer_.reset(new Hwc2::impl::Composer("default"));
  composer_callback_ = new ComposerCallback;
  composer_->registerCallback(composer_callback_);
  LOG_ALWAYS_FATAL_IF(!composer_callback_->GotFirstHotplug(),
      "Registered composer callback but didn't get hotplug for primary"
      " display");
  composer_callback_->SetVsyncService(vsync_service_);
}

void HardwareComposer::OnPostThreadResumed() {
  ALOGI("OnPostThreadResumed");
  EnableDisplay(*target_display_, true);

  // Trigger target-specific performance mode change.
  property_set(kDvrPerformanceProperty, "performance");
}

void HardwareComposer::OnPostThreadPaused() {
  ALOGI("OnPostThreadPaused");
  retire_fence_fds_.clear();
  layers_.clear();

  // Phones create a new composer client on resume and destroy it on pause.
  // Standalones only create the composer client once and then use SetPowerMode
  // to control the screen on pause/resume.
  if (!is_standalone_device_) {
    if (composer_callback_ != nullptr) {
      composer_callback_->SetVsyncService(nullptr);
      composer_callback_ = nullptr;
    }
    composer_.reset(nullptr);
  } else {
    EnableDisplay(*target_display_, false);
  }

  // Trigger target-specific performance mode change.
  property_set(kDvrPerformanceProperty, "idle");
}

bool HardwareComposer::PostThreadCondWait(std::unique_lock<std::mutex>& lock,
                                          int timeout_sec,
                                          const std::function<bool()>& pred) {
  auto pred_with_quit = [&] {
    return pred() || (post_thread_state_ & PostThreadState::Quit);
  };
  if (timeout_sec >= 0) {
    post_thread_wait_.wait_for(lock, std::chrono::seconds(timeout_sec),
                               pred_with_quit);
  } else {
    post_thread_wait_.wait(lock, pred_with_quit);
  }
  if (post_thread_state_ & PostThreadState::Quit) {
    ALOGI("HardwareComposer::PostThread: Quitting.");
    return true;
  }
  return false;
}

HWC::Error HardwareComposer::Validate(hwc2_display_t display) {
  uint32_t num_types;
  uint32_t num_requests;
  HWC::Error error =
      composer_->validateDisplay(display, &num_types, &num_requests);

  if (error == HWC2_ERROR_HAS_CHANGES) {
    ALOGE("Hardware composer has requested composition changes, "
          "which we don't support.");
    // Accept the changes anyway and see if we can get something on the screen.
    error = composer_->acceptDisplayChanges(display);
  }

  return error;
}

bool HardwareComposer::EnableVsync(const DisplayParams& display, bool enabled) {
  HWC::Error error = composer_->setVsyncEnabled(display.id,
      (Hwc2::IComposerClient::Vsync)(enabled ? HWC2_VSYNC_ENABLE
                                             : HWC2_VSYNC_DISABLE));
  if (error != HWC::Error::None) {
    ALOGE("Error attempting to %s vsync on %s display: %s",
        enabled ? "enable" : "disable", GetDisplayName(display.is_primary),
        error.to_string().c_str());
  }
  return error == HWC::Error::None;
}

bool HardwareComposer::SetPowerMode(const DisplayParams& display, bool active) {
  ALOGI("Turning %s display %s", GetDisplayName(display.is_primary),
      active ? "on" : "off");
  HWC::PowerMode power_mode = active ? HWC::PowerMode::On : HWC::PowerMode::Off;
  HWC::Error error = composer_->setPowerMode(display.id,
      power_mode.cast<Hwc2::IComposerClient::PowerMode>());
  if (error != HWC::Error::None) {
    ALOGE("Error attempting to turn %s display %s: %s",
          GetDisplayName(display.is_primary), active ? "on" : "off",
        error.to_string().c_str());
  }
  return error == HWC::Error::None;
}

bool HardwareComposer::EnableDisplay(const DisplayParams& display,
                                     bool enabled) {
  bool power_result;
  bool vsync_result;
  // When turning a display on, we set the power state then set vsync. When
  // turning a display off we do it in the opposite order.
  if (enabled) {
    power_result = SetPowerMode(display, enabled);
    vsync_result = EnableVsync(display, enabled);
  } else {
    vsync_result = EnableVsync(display, enabled);
    power_result = SetPowerMode(display, enabled);
  }
  return power_result && vsync_result;
}

HWC::Error HardwareComposer::Present(hwc2_display_t display) {
  int32_t present_fence;
  HWC::Error error = composer_->presentDisplay(display, &present_fence);

  // According to the documentation, this fence is signaled at the time of
  // vsync/DMA for physical displays.
  if (error == HWC::Error::None) {
    retire_fence_fds_.emplace_back(present_fence);
  } else {
    ATRACE_INT("HardwareComposer: PresentResult", error);
  }

  return error;
}

DisplayParams HardwareComposer::GetDisplayParams(
    Hwc2::Composer* composer, hwc2_display_t display, bool is_primary) {
  DisplayParams params;
  params.id = display;
  params.is_primary = is_primary;

  Hwc2::Config config;
  HWC::Error error = composer->getActiveConfig(display, &config);

  if (error == HWC::Error::None) {
    auto get_attr = [&](hwc2_attribute_t attr, const char* attr_name)
        -> std::optional<int32_t> {
      int32_t val;
      HWC::Error error = composer->getDisplayAttribute(
          display, config, (Hwc2::IComposerClient::Attribute)attr, &val);
      if (error != HWC::Error::None) {
        ALOGE("Failed to get %s display attr %s: %s",
            GetDisplayName(is_primary), attr_name,
            error.to_string().c_str());
        return std::nullopt;
      }
      return val;
    };

    auto width = get_attr(HWC2_ATTRIBUTE_WIDTH, "width");
    auto height = get_attr(HWC2_ATTRIBUTE_HEIGHT, "height");

    if (width && height) {
      params.width = *width;
      params.height = *height;
    } else {
      ALOGI("Failed to get width and/or height for %s display. Using default"
          " size %dx%d.", GetDisplayName(is_primary), kDefaultDisplayWidth,
          kDefaultDisplayHeight);
      params.width = kDefaultDisplayWidth;
      params.height = kDefaultDisplayHeight;
    }

    auto vsync_period = get_attr(HWC2_ATTRIBUTE_VSYNC_PERIOD, "vsync period");
    if (vsync_period) {
      params.vsync_period_ns = *vsync_period;
    } else {
      ALOGI("Failed to get vsync period for %s display. Using default vsync"
          " period %.2fms", GetDisplayName(is_primary),
          static_cast<float>(kDefaultVsyncPeriodNs) / 1000000);
      params.vsync_period_ns = kDefaultVsyncPeriodNs;
    }

    auto dpi_x = get_attr(HWC2_ATTRIBUTE_DPI_X, "DPI X");
    auto dpi_y = get_attr(HWC2_ATTRIBUTE_DPI_Y, "DPI Y");
    if (dpi_x && dpi_y) {
      params.dpi.x = *dpi_x;
      params.dpi.y = *dpi_y;
    } else {
      ALOGI("Failed to get dpi_x and/or dpi_y for %s display. Using default"
          " dpi %d.", GetDisplayName(is_primary), kDefaultDpi);
      params.dpi.x = kDefaultDpi;
      params.dpi.y = kDefaultDpi;
    }
  } else {
    ALOGE("HardwareComposer: Failed to get current %s display config: %d."
        " Using default display values.",
        GetDisplayName(is_primary), error.value);
    params.width = kDefaultDisplayWidth;
    params.height = kDefaultDisplayHeight;
    params.dpi.x = kDefaultDpi;
    params.dpi.y = kDefaultDpi;
    params.vsync_period_ns = kDefaultVsyncPeriodNs;
  }

  ALOGI(
      "HardwareComposer: %s display attributes: width=%d height=%d "
      "vsync_period_ns=%d DPI=%dx%d",
      GetDisplayName(is_primary),
      params.width,
      params.height,
      params.vsync_period_ns,
      params.dpi.x,
      params.dpi.y);

  return params;
}

std::string HardwareComposer::Dump() {
  std::unique_lock<std::mutex> lock(post_thread_mutex_);
  std::ostringstream stream;

  auto print_display_metrics = [&](const DisplayParams& params) {
    stream << GetDisplayName(params.is_primary)
           << " display metrics:     " << params.width << "x"
           << params.height << " " << (params.dpi.x / 1000.0)
           << "x" << (params.dpi.y / 1000.0) << " dpi @ "
           << (1000000000.0 / params.vsync_period_ns) << " Hz"
           << std::endl;
  };

  print_display_metrics(primary_display_);
  if (external_display_)
    print_display_metrics(*external_display_);

  stream << "Post thread resumed: " << post_thread_resumed_ << std::endl;
  stream << "Active layers:       " << layers_.size() << std::endl;
  stream << std::endl;

  for (size_t i = 0; i < layers_.size(); i++) {
    stream << "Layer " << i << ":";
    stream << " type=" << layers_[i].GetCompositionType().to_string();
    stream << " surface_id=" << layers_[i].GetSurfaceId();
    stream << " buffer_id=" << layers_[i].GetBufferId();
    stream << std::endl;
  }
  stream << std::endl;

  if (post_thread_resumed_) {
    stream << "Hardware Composer Debug Info:" << std::endl;
    stream << composer_->dumpDebugInfo();
  }

  return stream.str();
}

void HardwareComposer::PostLayers(hwc2_display_t display) {
  ATRACE_NAME("HardwareComposer::PostLayers");

  // Setup the hardware composer layers with current buffers.
  for (auto& layer : layers_) {
    layer.Prepare();
  }

  // Now that we have taken in a frame from the application, we have a chance
  // to drop the frame before passing the frame along to HWC.
  // If the display driver has become backed up, we detect it here and then
  // react by skipping this frame to catch up latency.
  while (!retire_fence_fds_.empty() &&
         (!retire_fence_fds_.front() ||
          sync_wait(retire_fence_fds_.front().Get(), 0) == 0)) {
    // There are only 2 fences in here, no performance problem to shift the
    // array of ints.
    retire_fence_fds_.erase(retire_fence_fds_.begin());
  }

  const bool is_fence_pending = static_cast<int32_t>(retire_fence_fds_.size()) >
                                post_thread_config_.allowed_pending_fence_count;

  if (is_fence_pending) {
    ATRACE_INT("frame_skip_count", ++frame_skip_count_);

    ALOGW_IF(is_fence_pending,
             "Warning: dropping a frame to catch up with HWC (pending = %zd)",
             retire_fence_fds_.size());

    for (auto& layer : layers_) {
      layer.Drop();
    }
    return;
  } else {
    // Make the transition more obvious in systrace when the frame skip happens
    // above.
    ATRACE_INT("frame_skip_count", 0);
  }

#if TRACE > 1
  for (size_t i = 0; i < layers_.size(); i++) {
    ALOGI("HardwareComposer::PostLayers: layer=%zu buffer_id=%d composition=%s",
          i, layers_[i].GetBufferId(),
          layers_[i].GetCompositionType().to_string().c_str());
  }
#endif

  HWC::Error error = Validate(display);
  if (error != HWC::Error::None) {
    ALOGE("HardwareComposer::PostLayers: Validate failed: %s display=%" PRIu64,
          error.to_string().c_str(), display);
    return;
  }

  error = Present(display);
  if (error != HWC::Error::None) {
    ALOGE("HardwareComposer::PostLayers: Present failed: %s",
          error.to_string().c_str());
    return;
  }

  std::vector<Hwc2::Layer> out_layers;
  std::vector<int> out_fences;
  error = composer_->getReleaseFences(display,
                                      &out_layers, &out_fences);
  ALOGE_IF(error != HWC::Error::None,
           "HardwareComposer::PostLayers: Failed to get release fences: %s",
           error.to_string().c_str());

  // Perform post-frame bookkeeping.
  uint32_t num_elements = out_layers.size();
  for (size_t i = 0; i < num_elements; ++i) {
    for (auto& layer : layers_) {
      if (layer.GetLayerHandle() == out_layers[i]) {
        layer.Finish(out_fences[i]);
      }
    }
  }
}

void HardwareComposer::SetDisplaySurfaces(
    std::vector<std::shared_ptr<DirectDisplaySurface>> surfaces) {
  ALOGI("HardwareComposer::SetDisplaySurfaces: surface count=%zd",
        surfaces.size());
  const bool display_idle = surfaces.size() == 0;
  {
    std::unique_lock<std::mutex> lock(post_thread_mutex_);
    surfaces_ = std::move(surfaces);
    surfaces_changed_ = true;
  }

  if (request_display_callback_ && !is_standalone_device_)
    request_display_callback_(!display_idle);

  // Set idle state based on whether there are any surfaces to handle.
  UpdatePostThreadState(PostThreadState::Idle, display_idle);
}

int HardwareComposer::OnNewGlobalBuffer(DvrGlobalBufferKey key,
                                        IonBuffer& ion_buffer) {
  if (key == DvrGlobalBuffers::kVsyncBuffer) {
    vsync_ring_ = std::make_unique<CPUMappedBroadcastRing<DvrVsyncRing>>(
        &ion_buffer, CPUUsageMode::WRITE_OFTEN);

    if (vsync_ring_->IsMapped() == false) {
      return -EPERM;
    }
  }

  if (key == DvrGlobalBuffers::kVrFlingerConfigBufferKey) {
    return MapConfigBuffer(ion_buffer);
  }

  return 0;
}

void HardwareComposer::OnDeletedGlobalBuffer(DvrGlobalBufferKey key) {
  if (key == DvrGlobalBuffers::kVrFlingerConfigBufferKey) {
    ConfigBufferDeleted();
  }
}

int HardwareComposer::MapConfigBuffer(IonBuffer& ion_buffer) {
  std::lock_guard<std::mutex> lock(shared_config_mutex_);
  shared_config_ring_ = DvrConfigRing();

  if (ion_buffer.width() < DvrConfigRing::MemorySize()) {
    ALOGE("HardwareComposer::MapConfigBuffer: invalid buffer size.");
    return -EINVAL;
  }

  void* buffer_base = 0;
  int result = ion_buffer.Lock(ion_buffer.usage(), 0, 0, ion_buffer.width(),
                               ion_buffer.height(), &buffer_base);
  if (result != 0) {
    ALOGE(
        "HardwareComposer::MapConfigBuffer: Failed to map vrflinger config "
        "buffer.");
    return -EPERM;
  }

  shared_config_ring_ = DvrConfigRing::Create(buffer_base, ion_buffer.width());
  ion_buffer.Unlock();

  return 0;
}

void HardwareComposer::ConfigBufferDeleted() {
  std::lock_guard<std::mutex> lock(shared_config_mutex_);
  shared_config_ring_ = DvrConfigRing();
}

void HardwareComposer::UpdateConfigBuffer() {
  std::lock_guard<std::mutex> lock(shared_config_mutex_);
  if (!shared_config_ring_.is_valid())
    return;
  // Copy from latest record in shared_config_ring_ to local copy.
  DvrConfig record;
  if (shared_config_ring_.GetNewest(&shared_config_ring_sequence_, &record)) {
    ALOGI("DvrConfig updated: sequence %u, post offset %d",
          shared_config_ring_sequence_, record.frame_post_offset_ns);
    ++shared_config_ring_sequence_;
    post_thread_config_ = record;
  }
}

int HardwareComposer::PostThreadPollInterruptible(
    const pdx::LocalHandle& event_fd, int requested_events, int timeout_ms) {
  pollfd pfd[2] = {
      {
          .fd = event_fd.Get(),
          .events = static_cast<short>(requested_events),
          .revents = 0,
      },
      {
          .fd = post_thread_event_fd_.Get(),
          .events = POLLPRI | POLLIN,
          .revents = 0,
      },
  };
  int ret, error;
  do {
    ret = poll(pfd, 2, timeout_ms);
    error = errno;
    ALOGW_IF(ret < 0,
             "HardwareComposer::PostThreadPollInterruptible: Error during "
             "poll(): %s (%d)",
             strerror(error), error);
  } while (ret < 0 && error == EINTR);

  if (ret < 0) {
    return -error;
  } else if (ret == 0) {
    return -ETIMEDOUT;
  } else if (pfd[0].revents != 0) {
    return 0;
  } else if (pfd[1].revents != 0) {
    ALOGI("VrHwcPost thread interrupted: revents=%x", pfd[1].revents);
    return kPostThreadInterrupted;
  } else {
    return 0;
  }
}

// Sleep until the next predicted vsync, returning the predicted vsync
// timestamp.
Status<int64_t> HardwareComposer::WaitForPredictedVSync() {
  const int64_t predicted_vsync_time = last_vsync_timestamp_ +
      (target_display_->vsync_period_ns * vsync_prediction_interval_);
  const int error = SleepUntil(predicted_vsync_time);
  if (error < 0) {
    ALOGE("HardwareComposer::WaifForVSync:: Failed to sleep: %s",
          strerror(-error));
    return error;
  }
  return {predicted_vsync_time};
}

int HardwareComposer::SleepUntil(int64_t wakeup_timestamp) {
  const int timer_fd = vsync_sleep_timer_fd_.Get();
  const itimerspec wakeup_itimerspec = {
      .it_interval = {.tv_sec = 0, .tv_nsec = 0},
      .it_value = NsToTimespec(wakeup_timestamp),
  };
  int ret =
      timerfd_settime(timer_fd, TFD_TIMER_ABSTIME, &wakeup_itimerspec, nullptr);
  int error = errno;
  if (ret < 0) {
    ALOGE("HardwareComposer::SleepUntil: Failed to set timerfd: %s",
          strerror(error));
    return -error;
  }

  return PostThreadPollInterruptible(vsync_sleep_timer_fd_, POLLIN,
                                     /*timeout_ms*/ -1);
}

void HardwareComposer::PostThread() {
  // NOLINTNEXTLINE(runtime/int)
  prctl(PR_SET_NAME, reinterpret_cast<unsigned long>("VrHwcPost"), 0, 0, 0);

  // Set the scheduler to SCHED_FIFO with high priority. If this fails here
  // there may have been a startup timing issue between this thread and
  // performanced. Try again later when this thread becomes active.
  bool thread_policy_setup =
      SetThreadPolicy("graphics:high", "/system/performance");

  // Create a timerfd based on CLOCK_MONOTINIC.
  vsync_sleep_timer_fd_.Reset(timerfd_create(CLOCK_MONOTONIC, 0));
  LOG_ALWAYS_FATAL_IF(
      !vsync_sleep_timer_fd_,
      "HardwareComposer: Failed to create vsync sleep timerfd: %s",
      strerror(errno));

  struct VsyncEyeOffsets { int64_t left_ns, right_ns; };
  bool was_running = false;

  auto get_vsync_eye_offsets = [this]() -> VsyncEyeOffsets {
    VsyncEyeOffsets offsets;
    offsets.left_ns =
        GetPosePredictionTimeOffset(target_display_->vsync_period_ns);

    // TODO(jbates) Query vblank time from device, when such an API is
    // available. This value (6.3%) was measured on A00 in low persistence mode.
    int64_t vblank_ns = target_display_->vsync_period_ns * 63 / 1000;
    offsets.right_ns = (target_display_->vsync_period_ns - vblank_ns) / 2;

    // Check property for overriding right eye offset value.
    offsets.right_ns =
        property_get_int64(kRightEyeOffsetProperty, offsets.right_ns);

    return offsets;
  };

  VsyncEyeOffsets vsync_eye_offsets = get_vsync_eye_offsets();

  if (is_standalone_device_) {
    // First, wait until boot finishes.
    std::unique_lock<std::mutex> lock(post_thread_mutex_);
    if (PostThreadCondWait(lock, -1, [this] { return boot_finished_; })) {
      return;
    }

    // Then, wait until we're either leaving the quiescent state, or the boot
    // finished display off timeout expires.
    if (PostThreadCondWait(lock, kBootFinishedDisplayOffTimeoutSec,
                           [this] { return !post_thread_quiescent_; })) {
      return;
    }

    LOG_ALWAYS_FATAL_IF(post_thread_state_ & PostThreadState::Suspended,
                        "Vr flinger should own the display by now.");
    post_thread_resumed_ = true;
    post_thread_ready_.notify_all();
    if (!composer_)
      CreateComposer();
  }

  while (1) {
    ATRACE_NAME("HardwareComposer::PostThread");

    // Check for updated config once per vsync.
    UpdateConfigBuffer();

    while (post_thread_quiescent_) {
      std::unique_lock<std::mutex> lock(post_thread_mutex_);
      ALOGI("HardwareComposer::PostThread: Entering quiescent state.");

      if (was_running) {
        vsync_trace_parity_ = false;
        ATRACE_INT(kVsyncTraceEventName, 0);
      }

      // Tear down resources.
      OnPostThreadPaused();
      was_running = false;
      post_thread_resumed_ = false;
      post_thread_ready_.notify_all();

      if (PostThreadCondWait(lock, -1,
                             [this] { return !post_thread_quiescent_; })) {
        // A true return value means we've been asked to quit.
        return;
      }

      post_thread_resumed_ = true;
      post_thread_ready_.notify_all();

      ALOGI("HardwareComposer::PostThread: Exiting quiescent state.");
    }

    if (!composer_)
      CreateComposer();

    bool target_display_changed = UpdateTargetDisplay();
    bool just_resumed_running = !was_running;
    was_running = true;

    if (target_display_changed)
      vsync_eye_offsets = get_vsync_eye_offsets();

    if (just_resumed_running) {
      OnPostThreadResumed();

      // Try to setup the scheduler policy if it failed during startup. Only
      // attempt to do this on transitions from inactive to active to avoid
      // spamming the system with RPCs and log messages.
      if (!thread_policy_setup) {
        thread_policy_setup =
            SetThreadPolicy("graphics:high", "/system/performance");
      }
    }

    if (target_display_changed || just_resumed_running) {
      // Initialize the last vsync timestamp with the current time. The
      // predictor below uses this time + the vsync interval in absolute time
      // units for the initial delay. Once the driver starts reporting vsync the
      // predictor will sync up with the real vsync.
      last_vsync_timestamp_ = GetSystemClockNs();
      vsync_prediction_interval_ = 1;
      retire_fence_fds_.clear();
    }

    int64_t vsync_timestamp = 0;
    {
      TRACE_FORMAT("wait_vsync|vsync=%u;last_timestamp=%" PRId64
                   ";prediction_interval=%d|",
                   vsync_count_ + 1, last_vsync_timestamp_,
                   vsync_prediction_interval_);

      auto status = WaitForPredictedVSync();
      ALOGE_IF(
          !status,
          "HardwareComposer::PostThread: Failed to wait for vsync event: %s",
          status.GetErrorMessage().c_str());

      // If there was an error either sleeping was interrupted due to pausing or
      // there was an error getting the latest timestamp.
      if (!status)
        continue;

      // Predicted vsync timestamp for this interval. This is stable because we
      // use absolute time for the wakeup timer.
      vsync_timestamp = status.get();
    }

    vsync_trace_parity_ = !vsync_trace_parity_;
    ATRACE_INT(kVsyncTraceEventName, vsync_trace_parity_ ? 1 : 0);

    // Advance the vsync counter only if the system is keeping up with hardware
    // vsync to give clients an indication of the delays.
    if (vsync_prediction_interval_ == 1)
      ++vsync_count_;

    UpdateLayerConfig();

    // Publish the vsync event.
    if (vsync_ring_) {
      DvrVsync vsync;
      vsync.vsync_count = vsync_count_;
      vsync.vsync_timestamp_ns = vsync_timestamp;
      vsync.vsync_left_eye_offset_ns = vsync_eye_offsets.left_ns;
      vsync.vsync_right_eye_offset_ns = vsync_eye_offsets.right_ns;
      vsync.vsync_period_ns = target_display_->vsync_period_ns;

      vsync_ring_->Publish(vsync);
    }

    {
      // Sleep until shortly before vsync.
      ATRACE_NAME("sleep");

      const int64_t display_time_est_ns =
          vsync_timestamp + target_display_->vsync_period_ns;
      const int64_t now_ns = GetSystemClockNs();
      const int64_t sleep_time_ns = display_time_est_ns - now_ns -
                                    post_thread_config_.frame_post_offset_ns;
      const int64_t wakeup_time_ns =
          display_time_est_ns - post_thread_config_.frame_post_offset_ns;

      ATRACE_INT64("sleep_time_ns", sleep_time_ns);
      if (sleep_time_ns > 0) {
        int error = SleepUntil(wakeup_time_ns);
        ALOGE_IF(error < 0 && error != kPostThreadInterrupted,
                 "HardwareComposer::PostThread: Failed to sleep: %s",
                 strerror(-error));
        // If the sleep was interrupted (error == kPostThreadInterrupted),
        // we still go through and present this frame because we may have set
        // layers earlier and we want to flush the Composer's internal command
        // buffer by continuing through to validate and present.
      }
    }

    {
      auto status = composer_callback_->GetVsyncTime(target_display_->id);

      // If we failed to read vsync there might be a problem with the driver.
      // Since there's nothing we can do just behave as though we didn't get an
      // updated vsync time and let the prediction continue.
      const int64_t current_vsync_timestamp =
          status ? status.get() : last_vsync_timestamp_;

      const bool vsync_delayed =
          last_vsync_timestamp_ == current_vsync_timestamp;
      ATRACE_INT("vsync_delayed", vsync_delayed);

      // If vsync was delayed advance the prediction interval and allow the
      // fence logic in PostLayers() to skip the frame.
      if (vsync_delayed) {
        ALOGW(
            "HardwareComposer::PostThread: VSYNC timestamp did not advance "
            "since last frame: timestamp=%" PRId64 " prediction_interval=%d",
            current_vsync_timestamp, vsync_prediction_interval_);
        vsync_prediction_interval_++;
      } else {
        // We have an updated vsync timestamp, reset the prediction interval.
        last_vsync_timestamp_ = current_vsync_timestamp;
        vsync_prediction_interval_ = 1;
      }
    }

    PostLayers(target_display_->id);
  }
}

bool HardwareComposer::UpdateTargetDisplay() {
  bool target_display_changed = false;
  auto displays = composer_callback_->GetDisplays();
  if (displays.external_display_was_hotplugged) {
    bool was_using_external_display = !target_display_->is_primary;
    if (was_using_external_display) {
      // The external display was hotplugged, so make sure to ignore any bad
      // display errors as we destroy the layers.
      for (auto& layer: layers_)
        layer.IgnoreBadDisplayErrorsOnDestroy(true);
    }

    if (displays.external_display) {
      // External display was connected
      external_display_ = GetDisplayParams(composer_.get(),
          *displays.external_display, /*is_primary*/ false);

      if (property_get_bool(kUseExternalDisplayProperty, false)) {
        ALOGI("External display connected. Switching to external display.");
        target_display_ = &(*external_display_);
        target_display_changed = true;
      } else {
        ALOGI("External display connected, but sysprop %s is unset, so"
              " using primary display.", kUseExternalDisplayProperty);
        if (was_using_external_display) {
          target_display_ = &primary_display_;
          target_display_changed = true;
        }
      }
    } else {
      // External display was disconnected
      external_display_ = std::nullopt;
      if (was_using_external_display) {
        ALOGI("External display disconnected. Switching to primary display.");
        target_display_ = &primary_display_;
        target_display_changed = true;
      }
    }
  }

  if (target_display_changed) {
    // If we're switching to the external display, turn the primary display off.
    if (!target_display_->is_primary) {
      EnableDisplay(primary_display_, false);
    }
    // If we're switching to the primary display, and the external display is
    // still connected, turn the external display off.
    else if (target_display_->is_primary && external_display_) {
      EnableDisplay(*external_display_, false);
    }

    // Turn the new target display on.
    EnableDisplay(*target_display_, true);

    // When we switch displays we need to recreate all the layers, so clear the
    // current list, which will trigger layer recreation.
    layers_.clear();
  }

  return target_display_changed;
}

// Checks for changes in the surface stack and updates the layer config to
// accomodate the new stack.
void HardwareComposer::UpdateLayerConfig() {
  std::vector<std::shared_ptr<DirectDisplaySurface>> surfaces;
  {
    std::unique_lock<std::mutex> lock(post_thread_mutex_);

    if (!surfaces_changed_ && (!layers_.empty() || surfaces_.empty()))
      return;

    surfaces = surfaces_;
    surfaces_changed_ = false;
  }

  ATRACE_NAME("UpdateLayerConfig_HwLayers");

  // Sort the new direct surface list by z-order to determine the relative order
  // of the surfaces. This relative order is used for the HWC z-order value to
  // insulate VrFlinger and HWC z-order semantics from each other.
  std::sort(surfaces.begin(), surfaces.end(), [](const auto& a, const auto& b) {
    return a->z_order() < b->z_order();
  });

  // Prepare a new layer stack, pulling in layers from the previous
  // layer stack that are still active and updating their attributes.
  std::vector<Layer> layers;
  size_t layer_index = 0;
  for (const auto& surface : surfaces) {
    // The bottom layer is opaque, other layers blend.
    HWC::BlendMode blending =
        layer_index == 0 ? HWC::BlendMode::None : HWC::BlendMode::Coverage;

    // Try to find a layer for this surface in the set of active layers.
    auto search =
        std::lower_bound(layers_.begin(), layers_.end(), surface->surface_id());
    const bool found = search != layers_.end() &&
                       search->GetSurfaceId() == surface->surface_id();
    if (found) {
      // Update the attributes of the layer that may have changed.
      search->SetBlending(blending);
      search->SetZOrder(layer_index);  // Relative z-order.

      // Move the existing layer to the new layer set and remove the empty layer
      // object from the current set.
      layers.push_back(std::move(*search));
      layers_.erase(search);
    } else {
      // Insert a layer for the new surface.
      layers.emplace_back(composer_.get(), *target_display_, surface, blending,
          HWC::Composition::Device, layer_index);
    }

    ALOGI_IF(
        TRACE,
        "HardwareComposer::UpdateLayerConfig: layer_index=%zu surface_id=%d",
        layer_index, layers[layer_index].GetSurfaceId());

    layer_index++;
  }

  // Sort the new layer stack by ascending surface id.
  std::sort(layers.begin(), layers.end());

  // Replace the previous layer set with the new layer set. The destructor of
  // the previous set will clean up the remaining Layers that are not moved to
  // the new layer set.
  layers_ = std::move(layers);

  ALOGD_IF(TRACE, "HardwareComposer::UpdateLayerConfig: %zd active layers",
           layers_.size());
}

std::vector<sp<IVsyncCallback>>::const_iterator
HardwareComposer::VsyncService::FindCallback(
    const sp<IVsyncCallback>& callback) const {
  sp<IBinder> binder = IInterface::asBinder(callback);
  return std::find_if(callbacks_.cbegin(), callbacks_.cend(),
                      [&](const sp<IVsyncCallback>& callback) {
                        return IInterface::asBinder(callback) == binder;
                      });
}

status_t HardwareComposer::VsyncService::registerCallback(
    const sp<IVsyncCallback> callback) {
  std::lock_guard<std::mutex> autolock(mutex_);
  if (FindCallback(callback) == callbacks_.cend()) {
    callbacks_.push_back(callback);
  }
  return OK;
}

status_t HardwareComposer::VsyncService::unregisterCallback(
    const sp<IVsyncCallback> callback) {
  std::lock_guard<std::mutex> autolock(mutex_);
  auto iter = FindCallback(callback);
  if (iter != callbacks_.cend()) {
    callbacks_.erase(iter);
  }
  return OK;
}

void HardwareComposer::VsyncService::OnVsync(int64_t vsync_timestamp) {
  ATRACE_NAME("VsyncService::OnVsync");
  std::lock_guard<std::mutex> autolock(mutex_);
  for (auto iter = callbacks_.begin(); iter != callbacks_.end();) {
    if ((*iter)->onVsync(vsync_timestamp) == android::DEAD_OBJECT) {
      iter = callbacks_.erase(iter);
    } else {
      ++iter;
    }
  }
}

Return<void> HardwareComposer::ComposerCallback::onHotplug(
    Hwc2::Display display, IComposerCallback::Connection conn) {
  std::lock_guard<std::mutex> lock(mutex_);
  ALOGI("onHotplug display=%" PRIu64 " conn=%d", display, conn);

  bool is_primary = !got_first_hotplug_ || display == primary_display_.id;

  // Our first onHotplug callback is always for the primary display.
  if (!got_first_hotplug_) {
    LOG_ALWAYS_FATAL_IF(conn != IComposerCallback::Connection::CONNECTED,
        "Initial onHotplug callback should be primary display connected");
    got_first_hotplug_ = true;
  } else if (is_primary) {
    ALOGE("Ignoring unexpected onHotplug() call for primary display");
    return Void();
  }

  if (conn == IComposerCallback::Connection::CONNECTED) {
    if (!is_primary)
      external_display_ = DisplayInfo();
    DisplayInfo& display_info = is_primary ?
        primary_display_ : *external_display_;
    display_info.id = display;

    std::array<char, 1024> buffer;
    snprintf(buffer.data(), buffer.size(),
             "/sys/class/graphics/fb%" PRIu64 "/vsync_event", display);
    if (LocalHandle handle{buffer.data(), O_RDONLY}) {
      ALOGI(
          "HardwareComposer::ComposerCallback::onHotplug: Driver supports "
          "vsync_event node for display %" PRIu64,
          display);
      display_info.driver_vsync_event_fd = std::move(handle);
    } else {
      ALOGI(
          "HardwareComposer::ComposerCallback::onHotplug: Driver does not "
          "support vsync_event node for display %" PRIu64,
          display);
    }
  } else if (conn == IComposerCallback::Connection::DISCONNECTED) {
    external_display_ = std::nullopt;
  }

  if (!is_primary)
    external_display_was_hotplugged_ = true;

  return Void();
}

Return<void> HardwareComposer::ComposerCallback::onRefresh(
    Hwc2::Display /*display*/) {
  return hardware::Void();
}

Return<void> HardwareComposer::ComposerCallback::onVsync(Hwc2::Display display,
                                                         int64_t timestamp) {
  TRACE_FORMAT("vsync_callback|display=%" PRIu64 ";timestamp=%" PRId64 "|",
               display, timestamp);
  std::lock_guard<std::mutex> lock(mutex_);
  DisplayInfo* display_info = GetDisplayInfo(display);
  if (display_info) {
    display_info->callback_vsync_timestamp = timestamp;
  }
  if (primary_display_.id == display && vsync_service_ != nullptr) {
    vsync_service_->OnVsync(timestamp);
  }

  return Void();
}

void HardwareComposer::ComposerCallback::SetVsyncService(
    const sp<VsyncService>& vsync_service) {
  std::lock_guard<std::mutex> lock(mutex_);
  vsync_service_ = vsync_service;
}

HardwareComposer::ComposerCallback::Displays
HardwareComposer::ComposerCallback::GetDisplays() {
  std::lock_guard<std::mutex> lock(mutex_);
  Displays displays;
  displays.primary_display = primary_display_.id;
  if (external_display_)
    displays.external_display = external_display_->id;
  if (external_display_was_hotplugged_) {
    external_display_was_hotplugged_ = false;
    displays.external_display_was_hotplugged = true;
  }
  return displays;
}

Status<int64_t> HardwareComposer::ComposerCallback::GetVsyncTime(
    hwc2_display_t display) {
  std::lock_guard<std::mutex> autolock(mutex_);
  DisplayInfo* display_info = GetDisplayInfo(display);
  if (!display_info) {
    ALOGW("Attempt to get vsync time for unknown display %" PRIu64, display);
    return ErrorStatus(EINVAL);
  }

  // See if the driver supports direct vsync events.
  LocalHandle& event_fd = display_info->driver_vsync_event_fd;
  if (!event_fd) {
    // Fall back to returning the last timestamp returned by the vsync
    // callback.
    return display_info->callback_vsync_timestamp;
  }

  // When the driver supports the vsync_event sysfs node we can use it to
  // determine the latest vsync timestamp, even if the HWC callback has been
  // delayed.

  // The driver returns data in the form "VSYNC=<timestamp ns>".
  std::array<char, 32> data;
  data.fill('\0');

  // Seek back to the beginning of the event file.
  int ret = lseek(event_fd.Get(), 0, SEEK_SET);
  if (ret < 0) {
    const int error = errno;
    ALOGE(
        "HardwareComposer::ComposerCallback::GetVsyncTime: Failed to seek "
        "vsync event fd: %s",
        strerror(error));
    return ErrorStatus(error);
  }

  // Read the vsync event timestamp.
  ret = read(event_fd.Get(), data.data(), data.size());
  if (ret < 0) {
    const int error = errno;
    ALOGE_IF(error != EAGAIN,
             "HardwareComposer::ComposerCallback::GetVsyncTime: Error "
             "while reading timestamp: %s",
             strerror(error));
    return ErrorStatus(error);
  }

  int64_t timestamp;
  ret = sscanf(data.data(), "VSYNC=%" PRIu64,
               reinterpret_cast<uint64_t*>(&timestamp));
  if (ret < 0) {
    const int error = errno;
    ALOGE(
        "HardwareComposer::ComposerCallback::GetVsyncTime: Error while "
        "parsing timestamp: %s",
        strerror(error));
    return ErrorStatus(error);
  }

  return {timestamp};
}

HardwareComposer::ComposerCallback::DisplayInfo*
HardwareComposer::ComposerCallback::GetDisplayInfo(hwc2_display_t display) {
  if (display == primary_display_.id) {
    return &primary_display_;
  } else if (external_display_ && display == external_display_->id) {
    return &(*external_display_);
  }
  return nullptr;
}

void Layer::Reset() {
  if (hardware_composer_layer_) {
    HWC::Error error =
        composer_->destroyLayer(display_params_.id, hardware_composer_layer_);
    if (error != HWC::Error::None &&
        (!ignore_bad_display_errors_on_destroy_ ||
         error != HWC::Error::BadDisplay)) {
      ALOGE("destroyLayer() failed for display %" PRIu64 ", layer %" PRIu64
          ". error: %s", display_params_.id, hardware_composer_layer_,
          error.to_string().c_str());
    }
    hardware_composer_layer_ = 0;
  }

  z_order_ = 0;
  blending_ = HWC::BlendMode::None;
  composition_type_ = HWC::Composition::Invalid;
  target_composition_type_ = composition_type_;
  source_ = EmptyVariant{};
  acquire_fence_.Close();
  surface_rect_functions_applied_ = false;
  pending_visibility_settings_ = true;
  cached_buffer_map_.clear();
  ignore_bad_display_errors_on_destroy_ = false;
}

Layer::Layer(Hwc2::Composer* composer, const DisplayParams& display_params,
             const std::shared_ptr<DirectDisplaySurface>& surface,
             HWC::BlendMode blending, HWC::Composition composition_type,
             size_t z_order)
    : composer_(composer),
      display_params_(display_params),
      z_order_{z_order},
      blending_{blending},
      target_composition_type_{composition_type},
      source_{SourceSurface{surface}} {
  CommonLayerSetup();
}

Layer::Layer(Hwc2::Composer* composer, const DisplayParams& display_params,
             const std::shared_ptr<IonBuffer>& buffer, HWC::BlendMode blending,
             HWC::Composition composition_type, size_t z_order)
    : composer_(composer),
      display_params_(display_params),
      z_order_{z_order},
      blending_{blending},
      target_composition_type_{composition_type},
      source_{SourceBuffer{buffer}} {
  CommonLayerSetup();
}

Layer::~Layer() { Reset(); }

Layer::Layer(Layer&& other) noexcept { *this = std::move(other); }

Layer& Layer::operator=(Layer&& other) noexcept {
  if (this != &other) {
    Reset();
    using std::swap;
    swap(composer_, other.composer_);
    swap(display_params_, other.display_params_);
    swap(hardware_composer_layer_, other.hardware_composer_layer_);
    swap(z_order_, other.z_order_);
    swap(blending_, other.blending_);
    swap(composition_type_, other.composition_type_);
    swap(target_composition_type_, other.target_composition_type_);
    swap(source_, other.source_);
    swap(acquire_fence_, other.acquire_fence_);
    swap(surface_rect_functions_applied_,
         other.surface_rect_functions_applied_);
    swap(pending_visibility_settings_, other.pending_visibility_settings_);
    swap(cached_buffer_map_, other.cached_buffer_map_);
    swap(ignore_bad_display_errors_on_destroy_,
         other.ignore_bad_display_errors_on_destroy_);
  }
  return *this;
}

void Layer::UpdateBuffer(const std::shared_ptr<IonBuffer>& buffer) {
  if (source_.is<SourceBuffer>())
    std::get<SourceBuffer>(source_) = {buffer};
}

void Layer::SetBlending(HWC::BlendMode blending) {
  if (blending_ != blending) {
    blending_ = blending;
    pending_visibility_settings_ = true;
  }
}

void Layer::SetZOrder(size_t z_order) {
  if (z_order_ != z_order) {
    z_order_ = z_order;
    pending_visibility_settings_ = true;
  }
}

IonBuffer* Layer::GetBuffer() {
  struct Visitor {
    IonBuffer* operator()(SourceSurface& source) { return source.GetBuffer(); }
    IonBuffer* operator()(SourceBuffer& source) { return source.GetBuffer(); }
    IonBuffer* operator()(EmptyVariant) { return nullptr; }
  };
  return source_.Visit(Visitor{});
}

void Layer::UpdateVisibilitySettings() {
  if (pending_visibility_settings_) {
    pending_visibility_settings_ = false;

    HWC::Error error;

    error = composer_->setLayerBlendMode(
        display_params_.id, hardware_composer_layer_,
        blending_.cast<Hwc2::IComposerClient::BlendMode>());
    ALOGE_IF(error != HWC::Error::None,
             "Layer::UpdateLayerSettings: Error setting layer blend mode: %s",
             error.to_string().c_str());

    error = composer_->setLayerZOrder(display_params_.id,
        hardware_composer_layer_, z_order_);
    ALOGE_IF(error != HWC::Error::None,
             "Layer::UpdateLayerSettings: Error setting z_ order: %s",
             error.to_string().c_str());
  }
}

void Layer::UpdateLayerSettings() {
  HWC::Error error;

  UpdateVisibilitySettings();

  // TODO(eieio): Use surface attributes or some other mechanism to control
  // the layer display frame.
  error = composer_->setLayerDisplayFrame(
      display_params_.id, hardware_composer_layer_,
      {0, 0, display_params_.width, display_params_.height});
  ALOGE_IF(error != HWC::Error::None,
           "Layer::UpdateLayerSettings: Error setting layer display frame: %s",
           error.to_string().c_str());

  error = composer_->setLayerVisibleRegion(
      display_params_.id, hardware_composer_layer_,
      {{0, 0, display_params_.width, display_params_.height}});
  ALOGE_IF(error != HWC::Error::None,
           "Layer::UpdateLayerSettings: Error setting layer visible region: %s",
           error.to_string().c_str());

  error = composer_->setLayerPlaneAlpha(display_params_.id,
      hardware_composer_layer_, 1.0f);
  ALOGE_IF(error != HWC::Error::None,
           "Layer::UpdateLayerSettings: Error setting layer plane alpha: %s",
           error.to_string().c_str());
}

void Layer::CommonLayerSetup() {
  HWC::Error error = composer_->createLayer(display_params_.id,
                                            &hardware_composer_layer_);
  ALOGE_IF(error != HWC::Error::None,
           "Layer::CommonLayerSetup: Failed to create layer on primary "
           "display: %s",
           error.to_string().c_str());
  UpdateLayerSettings();
}

bool Layer::CheckAndUpdateCachedBuffer(std::size_t slot, int buffer_id) {
  auto search = cached_buffer_map_.find(slot);
  if (search != cached_buffer_map_.end() && search->second == buffer_id)
    return true;

  // Assign or update the buffer slot.
  if (buffer_id >= 0)
    cached_buffer_map_[slot] = buffer_id;
  return false;
}

void Layer::Prepare() {
  int right, bottom, id;
  sp<GraphicBuffer> handle;
  std::size_t slot;

  // Acquire the next buffer according to the type of source.
  IfAnyOf<SourceSurface, SourceBuffer>::Call(&source_, [&](auto& source) {
    std::tie(right, bottom, id, handle, acquire_fence_, slot) =
        source.Acquire();
  });

  TRACE_FORMAT("Layer::Prepare|buffer_id=%d;slot=%zu|", id, slot);

  // Update any visibility (blending, z-order) changes that occurred since
  // last prepare.
  UpdateVisibilitySettings();

  // When a layer is first setup there may be some time before the first
  // buffer arrives. Setup the HWC layer as a solid color to stall for time
  // until the first buffer arrives. Once the first buffer arrives there will
  // always be a buffer for the frame even if it is old.
  if (!handle.get()) {
    if (composition_type_ == HWC::Composition::Invalid) {
      composition_type_ = HWC::Composition::SolidColor;
      composer_->setLayerCompositionType(
          display_params_.id, hardware_composer_layer_,
          composition_type_.cast<Hwc2::IComposerClient::Composition>());
      Hwc2::IComposerClient::Color layer_color = {0, 0, 0, 0};
      composer_->setLayerColor(display_params_.id, hardware_composer_layer_,
                               layer_color);
    } else {
      // The composition type is already set. Nothing else to do until a
      // buffer arrives.
    }
  } else {
    if (composition_type_ != target_composition_type_) {
      composition_type_ = target_composition_type_;
      composer_->setLayerCompositionType(
          display_params_.id, hardware_composer_layer_,
          composition_type_.cast<Hwc2::IComposerClient::Composition>());
    }

    // See if the HWC cache already has this buffer.
    const bool cached = CheckAndUpdateCachedBuffer(slot, id);
    if (cached)
      handle = nullptr;

    HWC::Error error{HWC::Error::None};
    error =
        composer_->setLayerBuffer(display_params_.id, hardware_composer_layer_,
                                  slot, handle, acquire_fence_.Get());

    ALOGE_IF(error != HWC::Error::None,
             "Layer::Prepare: Error setting layer buffer: %s",
             error.to_string().c_str());

    if (!surface_rect_functions_applied_) {
      const float float_right = right;
      const float float_bottom = bottom;
      error = composer_->setLayerSourceCrop(display_params_.id,
                                            hardware_composer_layer_,
                                            {0, 0, float_right, float_bottom});

      ALOGE_IF(error != HWC::Error::None,
               "Layer::Prepare: Error setting layer source crop: %s",
               error.to_string().c_str());

      surface_rect_functions_applied_ = true;
    }
  }
}

void Layer::Finish(int release_fence_fd) {
  IfAnyOf<SourceSurface, SourceBuffer>::Call(
      &source_, [release_fence_fd](auto& source) {
        source.Finish(LocalHandle(release_fence_fd));
      });
}

void Layer::Drop() { acquire_fence_.Close(); }

}  // namespace dvr
}  // namespace android
