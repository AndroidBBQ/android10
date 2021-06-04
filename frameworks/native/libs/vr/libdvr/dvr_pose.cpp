#include "include/dvr/dvr_pose.h"

#include <memory>

#include <private/dvr/buffer_hub_queue_client.h>
#include <private/dvr/pose_client_internal.h>

#include "dvr_buffer_queue_internal.h"

using android::dvr::ConsumerQueue;

int dvrPoseClientGetDataReader(DvrPoseClient* client, uint64_t data_type,
                               DvrReadBufferQueue** queue_out) {
  if (!client || !queue_out)
    return -EINVAL;

  ConsumerQueue* consumer_queue;
  int status = android::dvr::dvrPoseClientGetDataReaderHandle(client,
                                                              data_type,
                                                              &consumer_queue);
  if (status != 0) {
    ALOGE("dvrPoseClientGetDataReader: Failed to get queue: %d", status);
    return status;
  }

  std::shared_ptr<ConsumerQueue> consumer_queue_ptr{consumer_queue};
  *queue_out = new DvrReadBufferQueue(consumer_queue_ptr);
  return 0;
}
