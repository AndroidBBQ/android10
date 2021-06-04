#ifndef ANDROID_PDX_MOCK_MESSAGE_READER_H_
#define ANDROID_PDX_MOCK_MESSAGE_READER_H_

#include <gmock/gmock.h>
#include <pdx/message_reader.h>

namespace android {
namespace pdx {

class MockInputResourceMapper : public InputResourceMapper {
 public:
  MOCK_METHOD2(GetFileHandle, bool(FileReference ref, LocalHandle* handle));
  MOCK_METHOD2(GetChannelHandle,
               bool(ChannelReference ref, LocalChannelHandle* handle));
};

class MockMessageReader : public MessageReader {
 public:
  MOCK_METHOD0(GetNextReadBufferSection, BufferSection());
  MOCK_METHOD1(ConsumeReadBufferSectionData, void(const void* new_start));
  MOCK_METHOD0(GetInputResourceMapper, InputResourceMapper*());
};

}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_MOCK_MESSAGE_READER_H_
