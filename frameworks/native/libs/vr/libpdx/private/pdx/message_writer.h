#ifndef ANDROID_PDX_MESSAGE_WRITER_H_
#define ANDROID_PDX_MESSAGE_WRITER_H_

#include <pdx/channel_handle.h>
#include <pdx/file_handle.h>
#include <pdx/status.h>

namespace android {
namespace pdx {

class OutputResourceMapper {
 public:
  virtual Status<FileReference> PushFileHandle(const LocalHandle& handle) = 0;
  virtual Status<FileReference> PushFileHandle(
      const BorrowedHandle& handle) = 0;
  virtual Status<FileReference> PushFileHandle(const RemoteHandle& handle) = 0;
  virtual Status<ChannelReference> PushChannelHandle(
      const LocalChannelHandle& handle) = 0;
  virtual Status<ChannelReference> PushChannelHandle(
      const BorrowedChannelHandle& handle) = 0;
  virtual Status<ChannelReference> PushChannelHandle(
      const RemoteChannelHandle& handle) = 0;

 protected:
  virtual ~OutputResourceMapper() = default;
};

class MessageWriter {
 public:
  virtual void* GetNextWriteBufferSection(size_t size) = 0;
  virtual OutputResourceMapper* GetOutputResourceMapper() = 0;

 protected:
  virtual ~MessageWriter() = default;
};

}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_MESSAGE_WRITER_H_
