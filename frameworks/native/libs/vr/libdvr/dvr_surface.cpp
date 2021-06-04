#include "include/dvr/dvr_surface.h"

#include <inttypes.h>

#include <pdx/rpc/variant.h>
#include <private/android/AHardwareBufferHelpers.h>
#include <private/dvr/display_client.h>

#include "dvr_buffer_queue_internal.h"
#include "dvr_internal.h"

using android::AHardwareBuffer_convertToGrallocUsageBits;
using android::dvr::display::DisplayClient;
using android::dvr::display::Surface;
using android::dvr::display::SurfaceAttributes;
using android::dvr::display::SurfaceAttributeValue;
using android::pdx::rpc::EmptyVariant;

namespace {

// Sets the Variant |destination| to the target std::array type and copies the C
// array into it. Unsupported std::array configurations will fail to compile.
template <typename T, std::size_t N>
void ArrayCopy(SurfaceAttributeValue* destination, const T (&source)[N]) {
  using ArrayType = std::array<T, N>;
  *destination = ArrayType{};
  std::copy(std::begin(source), std::end(source),
            std::get<ArrayType>(*destination).begin());
}

bool ConvertSurfaceAttributes(const DvrSurfaceAttribute* attributes,
                              size_t attribute_count,
                              SurfaceAttributes* surface_attributes,
                              size_t* error_index) {
  for (size_t i = 0; i < attribute_count; i++) {
    SurfaceAttributeValue value;
    switch (attributes[i].value.type) {
      case DVR_SURFACE_ATTRIBUTE_TYPE_INT32:
        value = attributes[i].value.int32_value;
        break;
      case DVR_SURFACE_ATTRIBUTE_TYPE_INT64:
        value = attributes[i].value.int64_value;
        break;
      case DVR_SURFACE_ATTRIBUTE_TYPE_BOOL:
        // bool_value is defined in an extern "C" block, which makes it look
        // like an int to C++. Use a cast to assign the correct type to the
        // Variant type SurfaceAttributeValue.
        value = static_cast<bool>(attributes[i].value.bool_value);
        break;
      case DVR_SURFACE_ATTRIBUTE_TYPE_FLOAT:
        value = attributes[i].value.float_value;
        break;
      case DVR_SURFACE_ATTRIBUTE_TYPE_FLOAT2:
        ArrayCopy(&value, attributes[i].value.float2_value);
        break;
      case DVR_SURFACE_ATTRIBUTE_TYPE_FLOAT3:
        ArrayCopy(&value, attributes[i].value.float3_value);
        break;
      case DVR_SURFACE_ATTRIBUTE_TYPE_FLOAT4:
        ArrayCopy(&value, attributes[i].value.float4_value);
        break;
      case DVR_SURFACE_ATTRIBUTE_TYPE_FLOAT8:
        ArrayCopy(&value, attributes[i].value.float8_value);
        break;
      case DVR_SURFACE_ATTRIBUTE_TYPE_FLOAT16:
        ArrayCopy(&value, attributes[i].value.float16_value);
        break;
      case DVR_SURFACE_ATTRIBUTE_TYPE_NONE:
        value = EmptyVariant{};
        break;
      default:
        *error_index = i;
        return false;
    }

    surface_attributes->emplace(attributes[i].key, value);
  }

  return true;
}

}  // anonymous namespace

extern "C" {

struct DvrSurface {
  std::unique_ptr<Surface> surface;
};

int dvrSurfaceCreate(const DvrSurfaceAttribute* attributes,
                     size_t attribute_count, DvrSurface** out_surface) {
  if (out_surface == nullptr) {
    ALOGE("dvrSurfaceCreate: Invalid inputs: out_surface=%p.", out_surface);
    return -EINVAL;
  }

  size_t error_index;
  SurfaceAttributes surface_attributes;
  if (!ConvertSurfaceAttributes(attributes, attribute_count,
                                &surface_attributes, &error_index)) {
    ALOGE("dvrSurfaceCreate: Invalid surface attribute type: %" PRIu64,
          attributes[error_index].value.type);
    return -EINVAL;
  }

  auto status = Surface::CreateSurface(surface_attributes);
  if (!status) {
    ALOGE("dvrSurfaceCreate:: Failed to create display surface: %s",
          status.GetErrorMessage().c_str());
    return -status.error();
  }

  *out_surface = new DvrSurface{status.take()};
  return 0;
}

void dvrSurfaceDestroy(DvrSurface* surface) { delete surface; }

int dvrSurfaceGetId(DvrSurface* surface) {
  return surface->surface->surface_id();
}

int dvrSurfaceSetAttributes(DvrSurface* surface,
                            const DvrSurfaceAttribute* attributes,
                            size_t attribute_count) {
  if (surface == nullptr || attributes == nullptr) {
    ALOGE(
        "dvrSurfaceSetAttributes: Invalid inputs: surface=%p attributes=%p "
        "attribute_count=%zu",
        surface, attributes, attribute_count);
    return -EINVAL;
  }

  size_t error_index;
  SurfaceAttributes surface_attributes;
  if (!ConvertSurfaceAttributes(attributes, attribute_count,
                                &surface_attributes, &error_index)) {
    ALOGE("dvrSurfaceSetAttributes: Invalid surface attribute type: %" PRIu64,
          attributes[error_index].value.type);
    return -EINVAL;
  }

  auto status = surface->surface->SetAttributes(surface_attributes);
  if (!status) {
    ALOGE("dvrSurfaceSetAttributes: Failed to set attributes: %s",
          status.GetErrorMessage().c_str());
    return -status.error();
  }

  return 0;
}

int dvrSurfaceCreateWriteBufferQueue(DvrSurface* surface, uint32_t width,
                                     uint32_t height, uint32_t format,
                                     uint32_t layer_count, uint64_t usage,
                                     size_t capacity, size_t metadata_size,
                                     DvrWriteBufferQueue** out_writer) {
  if (surface == nullptr || out_writer == nullptr) {
    ALOGE(
        "dvrSurfaceCreateWriteBufferQueue: Invalid inputs: surface=%p, "
        "out_writer=%p.",
        surface, out_writer);
    return -EINVAL;
  }

  auto status = surface->surface->CreateQueue(
      width, height, layer_count, format, usage, capacity, metadata_size);
  if (!status) {
    ALOGE("dvrSurfaceCreateWriteBufferQueue: Failed to create queue: %s",
          status.GetErrorMessage().c_str());
    return -status.error();
  }

  *out_writer = new DvrWriteBufferQueue(status.take());
  return 0;
}

int dvrSetupGlobalBuffer(DvrGlobalBufferKey key, size_t size, uint64_t usage,
                         DvrBuffer** buffer_out) {
  if (!buffer_out)
    return -EINVAL;

  int error;
  auto client = DisplayClient::Create(&error);
  if (!client) {
    ALOGE("dvrSetupGlobalBuffer: Failed to create display client: %s",
          strerror(-error));
    return error;
  }

  uint64_t gralloc_usage = AHardwareBuffer_convertToGrallocUsageBits(usage);

  auto buffer_status = client->SetupGlobalBuffer(key, size, gralloc_usage);
  if (!buffer_status) {
    ALOGE("dvrSetupGlobalBuffer: Failed to setup global buffer: %s",
          buffer_status.GetErrorMessage().c_str());
    return -buffer_status.error();
  }

  *buffer_out = CreateDvrBufferFromIonBuffer(buffer_status.take());
  return 0;
}

int dvrDeleteGlobalBuffer(DvrGlobalBufferKey key) {
  int error;
  auto client = DisplayClient::Create(&error);
  if (!client) {
    ALOGE("dvrDeleteGlobalBuffer: Failed to create display client: %s",
          strerror(-error));
    return error;
  }

  auto buffer_status = client->DeleteGlobalBuffer(key);
  if (!buffer_status) {
    ALOGE("dvrDeleteGlobalBuffer: Failed to delete named buffer: %s",
          buffer_status.GetErrorMessage().c_str());
    return -buffer_status.error();
  }

  return 0;
}

int dvrGetGlobalBuffer(DvrGlobalBufferKey key, DvrBuffer** out_buffer) {
  if (!out_buffer)
    return -EINVAL;

  int error;
  auto client = DisplayClient::Create(&error);
  if (!client) {
    ALOGE("dvrGetGlobalBuffer: Failed to create display client: %s",
          strerror(-error));
    return error;
  }

  auto status = client->GetGlobalBuffer(key);
  if (!status) {
    return -status.error();
  }
  *out_buffer = CreateDvrBufferFromIonBuffer(status.take());
  return 0;
}

int dvrGetNativeDisplayMetrics(size_t sizeof_metrics,
                               DvrNativeDisplayMetrics* metrics) {
  ALOGE_IF(sizeof_metrics != sizeof(DvrNativeDisplayMetrics),
           "dvrGetNativeDisplayMetrics: metrics struct mismatch, your dvr api "
           "header is out of date.");

  auto client = DisplayClient::Create();
  if (!client) {
    ALOGE("dvrGetNativeDisplayMetrics: Failed to create display client!");
    return -ECOMM;
  }

  if (metrics == nullptr) {
    ALOGE("dvrGetNativeDisplayMetrics: output metrics buffer must be non-null");
    return -EINVAL;
  }

  auto status = client->GetDisplayMetrics();

  if (!status) {
    return -status.error();
  }

  if (sizeof_metrics >= 20) {
    metrics->display_width = status.get().display_width;
    metrics->display_height = status.get().display_height;
    metrics->display_x_dpi = status.get().display_x_dpi;
    metrics->display_y_dpi = status.get().display_y_dpi;
    metrics->vsync_period_ns = status.get().vsync_period_ns;
  }

  return 0;
}

}  // extern "C"
