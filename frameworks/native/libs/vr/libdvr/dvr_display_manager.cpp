#include "include/dvr/dvr_display_manager.h"

#include <dvr/dvr_buffer.h>
#include <pdx/rpc/variant.h>
#include <private/dvr/buffer_hub_queue_client.h>
#include <private/dvr/consumer_buffer.h>
#include <private/dvr/display_client.h>
#include <private/dvr/display_manager_client.h>

#include "dvr_internal.h"
#include "dvr_buffer_queue_internal.h"

using android::dvr::ConsumerBuffer;
using android::dvr::display::DisplayManagerClient;
using android::dvr::display::SurfaceAttribute;
using android::dvr::display::SurfaceAttributes;
using android::dvr::display::SurfaceState;
using android::pdx::rpc::EmptyVariant;

namespace {

// Extracts type and value from the attribute Variant and writes them into the
// respective fields of DvrSurfaceAttribute.
struct AttributeVisitor {
  DvrSurfaceAttribute* attribute;

  void operator()(int32_t value) {
    attribute->value.int32_value = value;
    attribute->value.type = DVR_SURFACE_ATTRIBUTE_TYPE_INT32;
  }
  void operator()(int64_t value) {
    attribute->value.int64_value = value;
    attribute->value.type = DVR_SURFACE_ATTRIBUTE_TYPE_INT64;
  }
  void operator()(bool value) {
    attribute->value.bool_value = value;
    attribute->value.type = DVR_SURFACE_ATTRIBUTE_TYPE_BOOL;
  }
  void operator()(float value) {
    attribute->value.float_value = value;
    attribute->value.type = DVR_SURFACE_ATTRIBUTE_TYPE_FLOAT;
  }
  void operator()(const std::array<float, 2>& value) {
    std::copy(value.cbegin(), value.cend(), attribute->value.float2_value);
    attribute->value.type = DVR_SURFACE_ATTRIBUTE_TYPE_FLOAT2;
  }
  void operator()(const std::array<float, 3>& value) {
    std::copy(value.cbegin(), value.cend(), attribute->value.float3_value);
    attribute->value.type = DVR_SURFACE_ATTRIBUTE_TYPE_FLOAT3;
  }
  void operator()(const std::array<float, 4>& value) {
    std::copy(value.cbegin(), value.cend(), attribute->value.float4_value);
    attribute->value.type = DVR_SURFACE_ATTRIBUTE_TYPE_FLOAT4;
  }
  void operator()(const std::array<float, 8>& value) {
    std::copy(value.cbegin(), value.cend(), attribute->value.float8_value);
    attribute->value.type = DVR_SURFACE_ATTRIBUTE_TYPE_FLOAT8;
  }
  void operator()(const std::array<float, 16>& value) {
    std::copy(value.cbegin(), value.cend(), attribute->value.float16_value);
    attribute->value.type = DVR_SURFACE_ATTRIBUTE_TYPE_FLOAT16;
  }
  void operator()(EmptyVariant) {
    attribute->value.type = DVR_SURFACE_ATTRIBUTE_TYPE_NONE;
  }
};

size_t ConvertSurfaceAttributes(const SurfaceAttributes& surface_attributes,
                                DvrSurfaceAttribute* attributes,
                                size_t max_count) {
  size_t count = 0;
  for (const auto& attribute : surface_attributes) {
    if (count >= max_count)
      break;

    // Copy the key and extract the Variant value using a visitor.
    attributes[count].key = attribute.first;
    attribute.second.Visit(AttributeVisitor{&attributes[count]});
    count++;
  }

  return count;
}

}  // anonymous namespace

extern "C" {

struct DvrDisplayManager {
  std::unique_ptr<DisplayManagerClient> client;
};

struct DvrSurfaceState {
  std::vector<SurfaceState> state;
};

int dvrDisplayManagerCreate(DvrDisplayManager** client_out) {
  if (!client_out)
    return -EINVAL;

  auto client = DisplayManagerClient::Create();
  if (!client) {
    ALOGE("dvrDisplayManagerCreate: Failed to create display manager client!");
    return -EIO;
  }

  *client_out = new DvrDisplayManager{std::move(client)};
  return 0;
}

void dvrDisplayManagerDestroy(DvrDisplayManager* client) { delete client; }

int dvrDisplayManagerGetEventFd(DvrDisplayManager* client) {
  if (!client)
    return -EINVAL;

  return client->client->event_fd();
}

int dvrDisplayManagerTranslateEpollEventMask(DvrDisplayManager* client,
                                             int in_events, int* out_events) {
  if (!client || !out_events)
    return -EINVAL;

  auto status = client->client->GetEventMask(in_events);
  if (!status)
    return -status.error();

  *out_events = status.get();
  return 0;
}

int dvrDisplayManagerGetSurfaceState(DvrDisplayManager* client,
                                     DvrSurfaceState* state) {
  if (!client || !state)
    return -EINVAL;

  auto status = client->client->GetSurfaceState();
  if (!status)
    return -status.error();

  state->state = status.take();
  return 0;
}

int dvrDisplayManagerGetReadBufferQueue(DvrDisplayManager* client,
                                        int surface_id, int queue_id,
                                        DvrReadBufferQueue** queue_out) {
  if (!client || !queue_out)
    return -EINVAL;

  auto status = client->client->GetSurfaceQueue(surface_id, queue_id);
  if (!status) {
    ALOGE("dvrDisplayManagerGetReadBufferQueue: Failed to get queue: %s",
          status.GetErrorMessage().c_str());
    return -status.error();
  }

  *queue_out = new DvrReadBufferQueue(status.take());
  return 0;
}

int dvrSurfaceStateCreate(DvrSurfaceState** surface_state_out) {
  if (!surface_state_out)
    return -EINVAL;

  *surface_state_out = new DvrSurfaceState{};
  return 0;
}

void dvrSurfaceStateDestroy(DvrSurfaceState* surface_state) {
  delete surface_state;
}

int dvrSurfaceStateGetSurfaceCount(DvrSurfaceState* surface_state,
                                   size_t* count_out) {
  if (!surface_state)
    return -EINVAL;

  *count_out = surface_state->state.size();
  return 0;
}

int dvrSurfaceStateGetUpdateFlags(DvrSurfaceState* surface_state,
                                  size_t surface_index,
                                  DvrSurfaceUpdateFlags* flags_out) {
  if (!surface_state || surface_index >= surface_state->state.size())
    return -EINVAL;

  *flags_out = surface_state->state[surface_index].update_flags;
  return 0;
}

int dvrSurfaceStateGetSurfaceId(DvrSurfaceState* surface_state,
                                size_t surface_index, int* surface_id_out) {
  if (!surface_state || surface_index >= surface_state->state.size())
    return -EINVAL;

  *surface_id_out = surface_state->state[surface_index].surface_id;
  return 0;
}

int dvrSurfaceStateGetProcessId(DvrSurfaceState* surface_state,
                                size_t surface_index, int* process_id_out) {
  if (!surface_state || surface_index >= surface_state->state.size())
    return -EINVAL;

  *process_id_out = surface_state->state[surface_index].process_id;
  return 0;
}

int dvrSurfaceStateGetQueueCount(DvrSurfaceState* surface_state,
                                 size_t surface_index, size_t* count_out) {
  if (!surface_state || surface_index >= surface_state->state.size())
    return -EINVAL;

  *count_out = surface_state->state[surface_index].queue_ids.size();
  return 0;
}

ssize_t dvrSurfaceStateGetQueueIds(DvrSurfaceState* surface_state,
                                   size_t surface_index, int* queue_ids,
                                   size_t max_count) {
  if (!surface_state || surface_index >= surface_state->state.size())
    return -EINVAL;

  size_t i;
  const auto& state = surface_state->state[surface_index];
  for (i = 0; i < std::min(max_count, state.queue_ids.size()); i++) {
    queue_ids[i] = state.queue_ids[i];
  }

  return i;
}

int dvrSurfaceStateGetZOrder(DvrSurfaceState* surface_state,
                             size_t surface_index, int* z_order_out) {
  if (!surface_state || surface_index >= surface_state->state.size() ||
      !z_order_out) {
    return -EINVAL;
  }

  *z_order_out = surface_state->state[surface_index].GetZOrder();
  return 0;
}

int dvrSurfaceStateGetVisible(DvrSurfaceState* surface_state,
                              size_t surface_index, bool* visible_out) {
  if (!surface_state || surface_index >= surface_state->state.size() ||
      !visible_out) {
    return -EINVAL;
  }

  *visible_out = surface_state->state[surface_index].GetVisible();
  return 0;
}

int dvrSurfaceStateGetAttributeCount(DvrSurfaceState* surface_state,
                                     size_t surface_index, size_t* count_out) {
  if (!surface_state || surface_index >= surface_state->state.size() ||
      !count_out) {
    return -EINVAL;
  }

  *count_out = surface_state->state[surface_index].surface_attributes.size();
  return 0;
}

ssize_t dvrSurfaceStateGetAttributes(DvrSurfaceState* surface_state,
                                     size_t surface_index,
                                     DvrSurfaceAttribute* attributes,
                                     size_t max_count) {
  if (!surface_state || surface_index >= surface_state->state.size() ||
      !attributes) {
    return -EINVAL;
  }

  return ConvertSurfaceAttributes(
      surface_state->state[surface_index].surface_attributes, attributes,
      max_count);
}

}  // extern "C"
