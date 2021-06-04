// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.
//
#ifndef CLEARKEY_MEMORY_FILE_SYSTEM_H_
#define CLEARKEY_MEMORY_FILE_SYSTEM_H_

#include <map>
#include <string>

#include "ClearKeyTypes.h"

namespace android {
namespace hardware {
namespace drm {
namespace V1_2 {
namespace clearkey {

// Using android file system requires clearkey plugin to update
// its sepolicy. However, we are unable to update sepolicy for
// older vendor partitions. To provide backward compatibility,
// clearkey plugin implements a very simple file system in memory.
// This memory file system does not support directory structure.
class MemoryFileSystem {
 public:
    struct MemoryFile {
        std::string fileName;  // excludes path
        std::string content;
        size_t fileSize;

        std::string getContent() const { return content; }
        size_t getFileSize() const { return fileSize; }
        void setContent(const std::string& file) { content = file; }
        void setFileName(const std::string& name) { fileName = name; }
        void setFileSize(size_t size) {
            content.resize(size); fileSize = size;
        }
    };

    MemoryFileSystem() {};
    virtual ~MemoryFileSystem() {};

    bool FileExists(const std::string& fileName) const;
    ssize_t GetFileSize(const std::string& fileName) const;
    std::vector<std::string> ListFiles() const;
    size_t Read(const std::string& pathName, std::string* buffer);
    bool RemoveAllFiles();
    bool RemoveFile(const std::string& fileName);
    size_t Write(const std::string& pathName, const MemoryFile& memoryFile);

 private:
    // License file name is made up of a unique keySetId, therefore,
    // the filename can be used as the key to locate licenses in the
    // memory file system.
    std::map<std::string, MemoryFile> mMemoryFileSystem;

    std::string GetFileName(const std::string& path);

    CLEARKEY_DISALLOW_COPY_AND_ASSIGN(MemoryFileSystem);
};

} // namespace clearkey
} // namespace V1_2
} // namespace drm
} // namespace hardware
} // namespace android

#endif  // CLEARKEY_MEMORY_FILE_SYSTEM_H_
