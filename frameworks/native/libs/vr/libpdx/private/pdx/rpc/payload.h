#ifndef ANDROID_PDX_RPC_PAYLOAD_H_
#define ANDROID_PDX_RPC_PAYLOAD_H_

#include <iterator>

#include <pdx/client.h>
#include <pdx/rpc/message_buffer.h>
#include <pdx/service.h>

namespace android {
namespace pdx {
namespace rpc {

// Implements the payload interface, required by Serialize/Deserialize, on top
// of a thread-local MessageBuffer.
template <typename Slot>
class MessagePayload {
 public:
  using BufferType = typename MessageBuffer<Slot>::BufferType;
  using ValueType = typename MessageBuffer<Slot>::ValueType;

  // Constructs a MessagePayload with an empty TLS buffer.
  MessagePayload()
      : buffer_(MessageBuffer<Slot>::GetEmptyBuffer()),
        cursor_(buffer_.begin()),
        const_cursor_(buffer_.cbegin()) {}

  // Returns a reference to the cursor iterator to be used during serialization
  // into the underlying MessageBuffer.
  typename BufferType::iterator& Cursor() { return cursor_; }

  // Returns a reference to the const cursor iterator at the beginning of the
  // underlying MessageBuffer.
  typename BufferType::const_iterator& ConstCursor() { return const_cursor_; }

  // Returns a const iterator marking the end of the underlying MessageBuffer.
  typename BufferType::const_iterator ConstEnd() { return buffer_.cend(); }

  // Resizes the underlying MessageBuffer and sets the cursor to the beginning.
  void Resize(std::size_t size) {
    buffer_.resize(size);
    cursor_ = buffer_.begin();
    const_cursor_ = buffer_.cbegin();
  }

  // Resets the read cursor so that data can be read from the buffer again.
  void Rewind() { const_cursor_ = buffer_.cbegin(); }

  // Adds |size| bytes to the size of the underlying MessageBuffer and positions
  // the cursor at the beginning of the extended region.
  void Extend(std::size_t size) {
    const std::size_t offset = buffer_.size();
    buffer_.resize(offset + size);
    cursor_ = buffer_.begin() + offset;
    const_cursor_ = buffer_.cbegin() + offset;
  }

  // Clears the underlying MessageBuffer and sets the cursor to the beginning.
  void Clear() {
    buffer_.clear();
    cursor_ = buffer_.begin();
    const_cursor_ = buffer_.cbegin();
  }

  ValueType* Data() { return buffer_.data(); }
  const ValueType* Data() const { return buffer_.data(); }
  std::size_t Size() const { return buffer_.size(); }
  std::size_t Capacity() const { return buffer_.capacity(); }

 private:
  BufferType& buffer_;
  typename BufferType::iterator cursor_;
  typename BufferType::const_iterator const_cursor_;

  MessagePayload(const MessagePayload<Slot>&) = delete;
  void operator=(const MessagePayload<Slot>&) = delete;
};

// Implements the payload interface for service-side RPCs. Handles translating
// between remote and local handle spaces automatically.
template <typename Slot>
class ServicePayload : public MessagePayload<Slot>,
                       public MessageWriter,
                       public MessageReader {
 public:
  explicit ServicePayload(Message& message) : message_(message) {}

  // MessageWriter
  void* GetNextWriteBufferSection(size_t size) override {
    this->Extend(size);
    return &*this->Cursor();
  }

  OutputResourceMapper* GetOutputResourceMapper() override { return &message_; }

  // MessageReader
  BufferSection GetNextReadBufferSection() override {
    return {&*this->ConstCursor(), &*this->ConstEnd()};
  }

  void ConsumeReadBufferSectionData(const void* new_start) override {
    std::advance(this->ConstCursor(),
                 PointerDistance(new_start, &*this->ConstCursor()));
  }

  InputResourceMapper* GetInputResourceMapper() override { return &message_; }

 private:
  Message& message_;
};

// Implements the payload interface for client-side RPCs. Handles gathering file
// handles to be sent over IPC automatically.
template <typename Slot>
class ClientPayload : public MessagePayload<Slot>,
                      public MessageWriter,
                      public MessageReader {
 public:
  using ContainerType =
      MessageBuffer<ThreadLocalTypeSlot<ClientPayload<Slot>>, 1024u, int>;
  using BufferType = typename ContainerType::BufferType;

  explicit ClientPayload(Transaction& transaction)
      : transaction_{transaction} {}

  // MessageWriter
  void* GetNextWriteBufferSection(size_t size) override {
    this->Extend(size);
    return &*this->Cursor();
  }

  OutputResourceMapper* GetOutputResourceMapper() override {
    return &transaction_;
  }

  // MessageReader
  BufferSection GetNextReadBufferSection() override {
    return {&*this->ConstCursor(), &*this->ConstEnd()};
  }

  void ConsumeReadBufferSectionData(const void* new_start) override {
    std::advance(this->ConstCursor(),
                 PointerDistance(new_start, &*this->ConstCursor()));
  }

  InputResourceMapper* GetInputResourceMapper() override {
    return &transaction_;
  }

 private:
  Transaction& transaction_;
};

}  // namespace rpc
}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_RPC_PAYLOAD_H_
