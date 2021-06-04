#define LOG_TAG "PoseClient"
#include <dvr/dvr_shared_buffers.h>
#include <dvr/pose_client.h>

#include <stdint.h>

#include <log/log.h>
#include <pdx/client.h>
#include <pdx/default_transport/client_channel_factory.h>
#include <pdx/file_handle.h>
#include <private/dvr/buffer_hub_queue_client.h>
#include <private/dvr/consumer_buffer.h>
#include <private/dvr/display_client.h>
#include <private/dvr/pose-ipc.h>
#include <private/dvr/shared_buffer_helpers.h>

using android::dvr::ConsumerQueue;
using android::pdx::LocalHandle;
using android::pdx::LocalChannelHandle;
using android::pdx::Status;
using android::pdx::Transaction;

namespace android {
namespace dvr {
namespace {

typedef CPUMappedBroadcastRing<DvrPoseRing> SensorPoseRing;

constexpr static int32_t MAX_CONTROLLERS = 2;
}  // namespace

// PoseClient is a remote interface to the pose service in sensord.
class PoseClient : public pdx::ClientBase<PoseClient> {
 public:
  ~PoseClient() override {}

  // Casts C handle into an instance of this class.
  static PoseClient* FromC(DvrPoseClient* client) {
    return reinterpret_cast<PoseClient*>(client);
  }

  // Polls the pose service for the current state and stores it in *state.
  // Returns zero on success, a negative error code otherwise.
  int Poll(DvrPose* state) {
    // Allocate the helper class to access the sensor pose buffer.
    if (sensor_pose_buffer_ == nullptr) {
      sensor_pose_buffer_ = std::make_unique<SensorPoseRing>(
          DvrGlobalBuffers::kSensorPoseBuffer, CPUUsageMode::READ_RARELY);
    }

    if (state) {
      if (sensor_pose_buffer_->GetNewest(state)) {
        return 0;
      } else {
        return -EAGAIN;
      }
    }

    return -EINVAL;
  }

  int GetPose(uint32_t vsync_count, DvrPoseAsync* out_pose) {
    const auto vsync_buffer = GetVsyncBuffer();
    if (vsync_buffer) {
      *out_pose =
          vsync_buffer
              ->vsync_poses[vsync_count & DvrVsyncPoseBuffer::kIndexMask];
      return 0;
    } else {
      return -EAGAIN;
    }
  }

  uint32_t GetVsyncCount() {
    const auto vsync_buffer = GetVsyncBuffer();
    if (vsync_buffer) {
      return vsync_buffer->vsync_count;
    }

    return 0;
  }

  int GetControllerPose(int32_t controller_id, uint32_t vsync_count,
                        DvrPoseAsync* out_pose) {
    if (controller_id < 0 || controller_id >= MAX_CONTROLLERS) {
      return -EINVAL;
    }
    if (!controllers_[controller_id].mapped_pose_buffer) {
      int ret = GetControllerRingBuffer(controller_id);
      if (ret < 0)
        return ret;
    }
    *out_pose =
        controllers_[controller_id]
            .mapped_pose_buffer[vsync_count & DvrVsyncPoseBuffer::kIndexMask];
    return 0;
  }

  int LogController(bool enable) {
    Transaction trans{*this};
    Status<int> status = trans.Send<int>(DVR_POSE_LOG_CONTROLLER, &enable,
                                         sizeof(enable), nullptr, 0);
    ALOGE_IF(!status, "Pose LogController() failed because: %s",
             status.GetErrorMessage().c_str());
    return ReturnStatusOrError(status);
  }

  // Freezes the pose to the provided state. Future poll operations will return
  // this state until a different state is frozen or SetMode() is called with a
  // different mode.
  // Returns zero on success, a negative error code otherwise.
  int Freeze(const DvrPose& frozen_state) {
    Transaction trans{*this};
    Status<int> status = trans.Send<int>(DVR_POSE_FREEZE, &frozen_state,
                                         sizeof(frozen_state), nullptr, 0);
    ALOGE_IF(!status, "Pose Freeze() failed because: %s\n",
             status.GetErrorMessage().c_str());
    return ReturnStatusOrError(status);
  }

  // Sets the data mode for the pose service.
  int SetMode(DvrPoseMode mode) {
    Transaction trans{*this};
    Status<int> status =
        trans.Send<int>(DVR_POSE_SET_MODE, &mode, sizeof(mode), nullptr, 0);
    ALOGE_IF(!status, "Pose SetPoseMode() failed because: %s",
             status.GetErrorMessage().c_str());
    return ReturnStatusOrError(status);
  }

  // Gets the data mode for the pose service.
  int GetMode(DvrPoseMode* out_mode) {
    int mode;
    Transaction trans{*this};
    Status<int> status =
        trans.Send<int>(DVR_POSE_GET_MODE, nullptr, 0, &mode, sizeof(mode));
    ALOGE_IF(!status, "Pose GetPoseMode() failed because: %s",
             status.GetErrorMessage().c_str());
    if (status)
      *out_mode = DvrPoseMode(mode);
    return ReturnStatusOrError(status);
  }

  int GetTangoReaderHandle(uint64_t data_type, ConsumerQueue** queue_out) {
    // Get buffer.
    Transaction trans{*this};
    Status<LocalChannelHandle> status = trans.Send<LocalChannelHandle>(
        DVR_POSE_GET_TANGO_READER, &data_type, sizeof(data_type), nullptr, 0);

    if (!status) {
      ALOGE("PoseClient GetTangoReaderHandle() failed because: %s",
            status.GetErrorMessage().c_str());
      *queue_out = nullptr;
      return -status.error();
    }

    std::unique_ptr<ConsumerQueue> consumer_queue =
        ConsumerQueue::Import(status.take());
    *queue_out = consumer_queue.release();
    return 0;
  }

  int DataCapture(const DvrPoseDataCaptureRequest* request) {
    Transaction trans{*this};
    Status<int> status = trans.Send<int>(DVR_POSE_DATA_CAPTURE, request,
                                         sizeof(*request), nullptr, 0);
    ALOGE_IF(!status, "PoseClient DataCapture() failed because: %s\n",
             status.GetErrorMessage().c_str());
    return ReturnStatusOrError(status);
  }

  int DataReaderDestroy(uint64_t data_type) {
    Transaction trans{*this};
    Status<int> status = trans.Send<int>(DVR_POSE_TANGO_READER_DESTROY,
                                         &data_type, sizeof(data_type), nullptr,
                                         0);
    ALOGE_IF(!status, "PoseClient DataReaderDestroy() failed because: %s\n",
             status.GetErrorMessage().c_str());
    return ReturnStatusOrError(status);
  }

  // Enables or disables all pose processing from sensors
  int EnableSensors(bool enabled) {
    Transaction trans{*this};
    Status<int> status = trans.Send<int>(DVR_POSE_SENSORS_ENABLE, &enabled,
                                         sizeof(enabled), nullptr, 0);
    ALOGE_IF(!status, "Pose EnableSensors() failed because: %s\n",
             status.GetErrorMessage().c_str());
    return ReturnStatusOrError(status);
  }

  int GetRingBuffer(DvrPoseRingBufferInfo* out_info) {
    // First time mapping the buffer?
    const auto vsync_buffer = GetVsyncBuffer();
    if (vsync_buffer) {
      if (out_info) {
        out_info->min_future_count = DvrVsyncPoseBuffer::kMinFutureCount;
        out_info->total_count = DvrVsyncPoseBuffer::kSize;
        out_info->buffer = vsync_buffer->vsync_poses;
      }
      return -EINVAL;
    }

    return -EAGAIN;
  }

  int GetControllerRingBuffer(int32_t controller_id) {
    if (controller_id < 0 || controller_id >= MAX_CONTROLLERS) {
      return -EINVAL;
    }
    ControllerClientState& client_state = controllers_[controller_id];
    if (client_state.pose_buffer.get()) {
      return 0;
    }

    Transaction trans{*this};
    Status<LocalChannelHandle> status = trans.Send<LocalChannelHandle>(
        DVR_POSE_GET_CONTROLLER_RING_BUFFER, &controller_id,
        sizeof(controller_id), nullptr, 0);
    if (!status) {
      return -status.error();
    }

    auto buffer = ConsumerBuffer::Import(status.take());
    if (!buffer) {
      ALOGE("Pose failed to import ring buffer");
      return -EIO;
    }
    constexpr size_t size = DvrVsyncPoseBuffer::kSize * sizeof(DvrPoseAsync);
    void* addr = nullptr;
    int ret = buffer->GetBlobReadWritePointer(size, &addr);
    if (ret < 0 || !addr) {
      ALOGE("Pose failed to map ring buffer: ret:%d, addr:%p", ret, addr);
      return -EIO;
    }
    client_state.pose_buffer.swap(buffer);
    client_state.mapped_pose_buffer = static_cast<const DvrPoseAsync*>(addr);
    ALOGI(
        "Mapped controller %d pose data translation %f,%f,%f quat %f,%f,%f,%f",
        controller_id, client_state.mapped_pose_buffer[0].position[0],
        client_state.mapped_pose_buffer[0].position[1],
        client_state.mapped_pose_buffer[0].position[2],
        client_state.mapped_pose_buffer[0].orientation[0],
        client_state.mapped_pose_buffer[0].orientation[1],
        client_state.mapped_pose_buffer[0].orientation[2],
        client_state.mapped_pose_buffer[0].orientation[3]);
    return 0;
  }

 private:
  friend BASE;

  // Set up a channel to the pose service.
  PoseClient()
      : BASE(pdx::default_transport::ClientChannelFactory::Create(
            DVR_POSE_SERVICE_CLIENT)) {
    // TODO(eieio): Cache the pose and make timeout 0 so that the API doesn't
    // block while waiting for the pose service to come back up.
    EnableAutoReconnect(kInfiniteTimeout);
  }

  PoseClient(const PoseClient&) = delete;
  PoseClient& operator=(const PoseClient&) = delete;

  const DvrVsyncPoseBuffer* GetVsyncBuffer() {
    if (mapped_vsync_pose_buffer_ == nullptr) {
      if (vsync_pose_buffer_ == nullptr) {
        // The constructor tries mapping it so we do not need TryMapping after.
        vsync_pose_buffer_ = std::make_unique<CPUMappedBuffer>(
            DvrGlobalBuffers::kVsyncPoseBuffer, CPUUsageMode::READ_OFTEN);
      } else if (vsync_pose_buffer_->IsMapped() == false) {
        vsync_pose_buffer_->TryMapping();
      }

      if (vsync_pose_buffer_->IsMapped()) {
        mapped_vsync_pose_buffer_ =
            static_cast<DvrVsyncPoseBuffer*>(vsync_pose_buffer_->Address());
      }
    }

    return mapped_vsync_pose_buffer_;
  }

  // The vsync pose buffer if already mapped.
  std::unique_ptr<CPUMappedBuffer> vsync_pose_buffer_;

  // The direct sensor pose buffer.
  std::unique_ptr<SensorPoseRing> sensor_pose_buffer_;

  const DvrVsyncPoseBuffer* mapped_vsync_pose_buffer_ = nullptr;

  struct ControllerClientState {
    std::unique_ptr<ConsumerBuffer> pose_buffer;
    const DvrPoseAsync* mapped_pose_buffer = nullptr;
  };
  ControllerClientState controllers_[MAX_CONTROLLERS];
};

int dvrPoseClientGetDataReaderHandle(DvrPoseClient* client, uint64_t type,
                                     ConsumerQueue** queue_out) {
  return PoseClient::FromC(client)->GetTangoReaderHandle(type, queue_out);
}

}  // namespace dvr
}  // namespace android

using android::dvr::PoseClient;

extern "C" {

DvrPoseClient* dvrPoseClientCreate() {
  auto* client = PoseClient::Create().release();
  return reinterpret_cast<DvrPoseClient*>(client);
}

void dvrPoseClientDestroy(DvrPoseClient* client) {
  delete PoseClient::FromC(client);
}

int dvrPoseClientGet(DvrPoseClient* client, uint32_t vsync_count,
                     DvrPoseAsync* out_pose) {
  return PoseClient::FromC(client)->GetPose(vsync_count, out_pose);
}

uint32_t dvrPoseClientGetVsyncCount(DvrPoseClient* client) {
  return PoseClient::FromC(client)->GetVsyncCount();
}

int dvrPoseClientGetController(DvrPoseClient* client, int32_t controller_id,
                               uint32_t vsync_count, DvrPoseAsync* out_pose) {
  return PoseClient::FromC(client)->GetControllerPose(controller_id,
                                                      vsync_count, out_pose);
}

int dvrPoseClientLogController(DvrPoseClient* client, bool enable) {
  return PoseClient::FromC(client)->LogController(enable);
}

int dvrPoseClientPoll(DvrPoseClient* client, DvrPose* state) {
  return PoseClient::FromC(client)->Poll(state);
}

int dvrPoseClientFreeze(DvrPoseClient* client, const DvrPose* frozen_state) {
  return PoseClient::FromC(client)->Freeze(*frozen_state);
}

int dvrPoseClientModeSet(DvrPoseClient* client, DvrPoseMode mode) {
  return PoseClient::FromC(client)->SetMode(mode);
}

int dvrPoseClientModeGet(DvrPoseClient* client, DvrPoseMode* mode) {
  return PoseClient::FromC(client)->GetMode(mode);
}

int dvrPoseClientSensorsEnable(DvrPoseClient* client, bool enabled) {
  return PoseClient::FromC(client)->EnableSensors(enabled);
}

int dvrPoseClientDataCapture(DvrPoseClient* client,
                             const DvrPoseDataCaptureRequest* request) {
  return PoseClient::FromC(client)->DataCapture(request);
}

int dvrPoseClientDataReaderDestroy(DvrPoseClient* client, uint64_t data_type) {
  return PoseClient::FromC(client)->DataReaderDestroy(data_type);
}

}  // extern "C"
