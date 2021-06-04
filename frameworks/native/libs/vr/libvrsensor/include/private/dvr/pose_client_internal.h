#ifndef ANDROID_DVR_POSE_CLIENT_INTERNAL_H_
#define ANDROID_DVR_POSE_CLIENT_INTERNAL_H_

#include <private/dvr/buffer_hub_queue_client.h>

using android::dvr::ConsumerQueue;

typedef struct DvrPoseClient DvrPoseClient;

namespace android {
namespace dvr {

int dvrPoseClientGetDataReaderHandle(DvrPoseClient *client, uint64_t data_type,
                                     ConsumerQueue **queue_out);

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_POSE_CLIENT_INTERNAL_H_
