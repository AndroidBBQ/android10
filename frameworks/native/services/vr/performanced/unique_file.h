#ifndef ANDROID_DVR_PERFORMANCED_UNIQUE_FILE_H_
#define ANDROID_DVR_PERFORMANCED_UNIQUE_FILE_H_

#include <stdio.h>

#include <memory>

namespace android {
namespace dvr {

// Utility to manage the lifetime of a file pointer.
struct FileDeleter {
  void operator()(FILE* fp) { fclose(fp); }
};
using UniqueFile = std::unique_ptr<FILE, FileDeleter>;

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_PERFORMANCED_UNIQUE_FILE_H_
