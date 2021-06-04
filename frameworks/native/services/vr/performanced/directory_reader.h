#ifndef ANDROID_DVR_PERFORMANCED_DIRECTORY_READER_H_
#define ANDROID_DVR_PERFORMANCED_DIRECTORY_READER_H_

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <android-base/unique_fd.h>

namespace android {
namespace dvr {

// Utility class around readdir() that handles automatic cleanup.
class DirectoryReader {
 public:
  explicit DirectoryReader(base::unique_fd directory_fd) {
    int fd = directory_fd.release();
    directory_ = fdopendir(fd);
    error_ = errno;
    if (directory_ == nullptr)
      close(fd);
  }

  ~DirectoryReader() {
    if (directory_)
      closedir(directory_);
  }

  bool IsValid() const { return directory_ != nullptr; }
  explicit operator bool() const { return IsValid(); }
  int GetError() const { return error_; }

  // Returns a pointer to a dirent describing the next directory entry. The
  // pointer is only valid unitl the next call to Next() or the DirectoryReader
  // is destroyed. Returns nullptr when the end of the directory is reached.
  dirent* Next() {
    if (directory_)
      return readdir(directory_);
    else
      return nullptr;
  }

 private:
  DIR* directory_;
  int error_;

  DirectoryReader(const DirectoryReader&) = delete;
  void operator=(const DirectoryReader&) = delete;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_PERFORMANCED_DIRECTORY_READER_H_
