#ifndef ANDROID_PDX_MESSAGE_READER_H_
#define ANDROID_PDX_MESSAGE_READER_H_

#include <memory>

#include <pdx/channel_handle.h>
#include <pdx/file_handle.h>

namespace android {
namespace pdx {

class InputResourceMapper {
 public:
  virtual bool GetFileHandle(FileReference ref, LocalHandle* handle) = 0;
  virtual bool GetChannelHandle(ChannelReference ref,
                                LocalChannelHandle* handle) = 0;

 protected:
  virtual ~InputResourceMapper() = default;
};

class MessageReader {
 public:
  // Pointers to start/end of the region in the read buffer.
  using BufferSection = std::pair<const void*, const void*>;

  virtual BufferSection GetNextReadBufferSection() = 0;
  virtual void ConsumeReadBufferSectionData(const void* new_start) = 0;
  virtual InputResourceMapper* GetInputResourceMapper() = 0;

 protected:
  virtual ~MessageReader() = default;
};

}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_MESSAGE_READER_H_
