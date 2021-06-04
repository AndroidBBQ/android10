#ifndef ANDROID_PDX_FILE_HANDLE_H_
#define ANDROID_PDX_FILE_HANDLE_H_

#include <fcntl.h>
#include <unistd.h>

#include <string>

namespace android {
namespace pdx {

enum class FileHandleMode {
  Local,
  Remote,
  Borrowed,
};

// Manages ownership, sharing, and lifetime of file descriptors.
template <FileHandleMode Mode>
class FileHandle {
 public:
  static constexpr int kEmptyFileHandle = -1;

  // Constructs an empty FileHandle object.
  FileHandle() : fd_(kEmptyFileHandle) {}

  // Constructs a FileHandle from an integer file descriptor and takes
  // ownership.
  explicit FileHandle(int fd) : fd_(fd) {}

  // Constructs a FileHandle by opening |path|. The arguments follow the
  // semantics of open().
  FileHandle(const std::string& path, int flags, mode_t mode = 0) {
    fd_ = open(path.c_str(), flags, mode);
  }

  // Constructs a FileHandle by opening |path| relative to |dir_fd|, following
  // the semantics of openat().
  FileHandle(const int directory_fd, const std::string& path, int flags,
             mode_t mode = 0) {
    fd_ = openat(directory_fd, path.c_str(), flags, mode);
  }

  // Move constructor that assumes ownership of the file descriptor, leaving the
  // other FileHandle object empty.
  FileHandle(FileHandle&& other) noexcept {
    fd_ = other.fd_;
    other.fd_ = kEmptyFileHandle;
  }

  // Returns a FileHandle as a duplicate handle of |fd|. Borrowed handles and
  // handles in remote handle space are not duplicated.
  static FileHandle AsDuplicate(const int fd) {
    if (Mode == FileHandleMode::Local)
      return FileHandle(dup(fd));
    else
      return FileHandle(fd);
  }

  // Destructor closes the file descriptor when non-empty.
  ~FileHandle() { Close(); }

  // Move assignment operator that assumes ownership of the underlying file
  // descriptor, leaving the other FileHandle object empty.
  FileHandle& operator=(FileHandle&& other) noexcept {
    if (this != &other) {
      Reset(other.fd_);
      other.fd_ = kEmptyFileHandle;
    }
    return *this;
  }

  // Resets the underlying file handle to |fd|.
  void Reset(int fd) {
    Close();
    fd_ = fd;
  }

  // Closes the underlying file descriptor when non-empty.
  void Close() {
    if (IsValid() && Mode == FileHandleMode::Local)
      close(fd_);
    fd_ = kEmptyFileHandle;
  }

  // Return the internal fd, passing ownership to the caller.
  int Release() {
    int release_fd = fd_;
    fd_ = kEmptyFileHandle;
    return release_fd;
  }

  // Duplicates the underlying file descriptor and returns a FileHandle that
  // owns the new file descriptor. File descriptors are not duplicated when Mode
  // is Remote or Borrowed.
  FileHandle Duplicate() const {
    return FileHandle(Mode == FileHandleMode::Local ? dup(fd_) : fd_);
  }

  FileHandle<FileHandleMode::Borrowed> Borrow() const {
    return FileHandle<FileHandleMode::Borrowed>(Get());
  }

  // Gets the underlying file descriptor value.
  int Get() const { return fd_; }
  bool IsValid() const { return fd_ >= 0; }
  explicit operator bool() const { return IsValid(); }

 private:
  int fd_;

  FileHandle(const FileHandle&) = delete;
  void operator=(const FileHandle&) = delete;
};

// Alias for a file handle in the local process' handle space.
using LocalHandle = FileHandle<FileHandleMode::Local>;

// Alias for a file handle in another process' handle space. Handles returned
// from pushing a file object or channel must be stored in this type of handle
// class, which doesn't close the underlying file descriptor. The underlying
// file descriptor in this wrapper should not be passed to close() because doing
// so would close an unrelated file descriptor in the local handle space.
using RemoteHandle = FileHandle<FileHandleMode::Remote>;

// Alias for borrowed handles in the local process' handle space. A borrowed
// file handle is not close() because this wrapper does not own the underlying
// file descriptor. Care must be take to ensure that a borrowed file handle
// remains valid during use.
using BorrowedHandle = FileHandle<FileHandleMode::Borrowed>;

// FileReference is a 16 bit integer used in IPC serialization to be
// transferred across processes. You can convert this value to a local file
// handle by calling Transaction.GetFileHandle() on client side and
// Message.GetFileHandle() on service side.
using FileReference = int16_t;

}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_FILE_HANDLE_H_
