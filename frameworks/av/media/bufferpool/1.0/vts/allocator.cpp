/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cutils/ashmem.h>
#include <sys/mman.h>
#include "allocator.h"

union Params {
  struct {
    uint32_t capacity;
  } data;
  uint8_t array[0];
  Params() : data{0} {}
  Params(uint32_t size)
      : data{size} {}
};


namespace {

struct HandleAshmem : public native_handle_t {
  HandleAshmem(int ashmemFd, size_t size)
    : native_handle_t(cHeader),
    mFds{ ashmemFd },
    mInts{ int (size & 0xFFFFFFFF), int((uint64_t(size) >> 32) & 0xFFFFFFFF), kMagic } {}

  int ashmemFd() const { return mFds.mAshmem; }
  size_t size() const {
    return size_t(unsigned(mInts.mSizeLo))
        | size_t(uint64_t(unsigned(mInts.mSizeHi)) << 32);
  }

  static bool isValid(const native_handle_t * const o);

protected:
  struct {
    int mAshmem;
  } mFds;
  struct {
    int mSizeLo;
    int mSizeHi;
    int mMagic;
  } mInts;

private:
  enum {
    kMagic = 'ahm\x00',
    numFds = sizeof(mFds) / sizeof(int),
    numInts = sizeof(mInts) / sizeof(int),
    version = sizeof(native_handle_t)
  };
  const static native_handle_t cHeader;
};

const native_handle_t HandleAshmem::cHeader = {
  HandleAshmem::version,
  HandleAshmem::numFds,
  HandleAshmem::numInts,
  {}
};

bool HandleAshmem::isValid(const native_handle_t * const o) {
  if (!o || memcmp(o, &cHeader, sizeof(cHeader))) {
    return false;
  }
  const HandleAshmem *other = static_cast<const HandleAshmem*>(o);
  return other->mInts.mMagic == kMagic;
}

class AllocationAshmem {
private:
  AllocationAshmem(int ashmemFd, size_t capacity, bool res)
    : mHandle(ashmemFd, capacity),
      mInit(res) {}

public:
  static AllocationAshmem *Alloc(size_t size) {
    constexpr static const char *kAllocationTag = "bufferpool_test";
    int ashmemFd = ashmem_create_region(kAllocationTag, size);
    return new AllocationAshmem(ashmemFd, size, ashmemFd >= 0);
  }

  ~AllocationAshmem() {
    if (mInit) {
      native_handle_close(&mHandle);
    }
  }

  const HandleAshmem *handle() {
    return &mHandle;
  }

private:
  HandleAshmem mHandle;
  bool mInit;
  // TODO: mapping and map fd
};

struct AllocationDtor {
  AllocationDtor(const std::shared_ptr<AllocationAshmem> &alloc)
      : mAlloc(alloc) {}

  void operator()(BufferPoolAllocation *poolAlloc) { delete poolAlloc; }

  const std::shared_ptr<AllocationAshmem> mAlloc;
};

}


ResultStatus TestBufferPoolAllocator::allocate(
    const std::vector<uint8_t> &params,
    std::shared_ptr<BufferPoolAllocation> *alloc,
    size_t *allocSize) {
  Params ashmemParams;
  memcpy(&ashmemParams, params.data(), std::min(sizeof(Params), params.size()));

  std::shared_ptr<AllocationAshmem> ashmemAlloc =
      std::shared_ptr<AllocationAshmem>(
          AllocationAshmem::Alloc(ashmemParams.data.capacity));
  if (ashmemAlloc) {
    BufferPoolAllocation *ptr = new BufferPoolAllocation(ashmemAlloc->handle());
    if (ptr) {
      *alloc = std::shared_ptr<BufferPoolAllocation>(ptr, AllocationDtor(ashmemAlloc));
      if (*alloc) {
          *allocSize = ashmemParams.data.capacity;
          return ResultStatus::OK;
      }
      delete ptr;
      return ResultStatus::NO_MEMORY;
    }
  }
  return ResultStatus::CRITICAL_ERROR;
}

bool TestBufferPoolAllocator::compatible(const std::vector<uint8_t> &newParams,
                                        const std::vector<uint8_t> &oldParams) {
  size_t newSize = newParams.size();
  size_t oldSize = oldParams.size();
  if (newSize == oldSize) {
    for (size_t i = 0; i < newSize; ++i) {
      if (newParams[i] != oldParams[i]) {
        return false;
      }
    }
    return true;
  }
  return false;
}

bool TestBufferPoolAllocator::Fill(const native_handle_t *handle, const unsigned char val) {
  if (!HandleAshmem::isValid(handle)) {
    return false;
  }
  const HandleAshmem *o = static_cast<const HandleAshmem*>(handle);
  unsigned char *ptr = (unsigned char *)mmap(
      NULL, o->size(), PROT_READ|PROT_WRITE, MAP_SHARED, o->ashmemFd(), 0);

  if (ptr != MAP_FAILED) {
    for (size_t i = 0; i < o->size(); ++i) {
      ptr[i] = val;
    }
    munmap(ptr, o->size());
    return true;
  }
  return false;
}

bool TestBufferPoolAllocator::Verify(const native_handle_t *handle, const unsigned char val) {
  if (!HandleAshmem::isValid(handle)) {
    return false;
  }
  const HandleAshmem *o = static_cast<const HandleAshmem*>(handle);
  unsigned char *ptr = (unsigned char *)mmap(
      NULL, o->size(), PROT_READ, MAP_SHARED, o->ashmemFd(), 0);

  if (ptr != MAP_FAILED) {
    bool res = true;
    for (size_t i = 0; i < o->size(); ++i) {
      if (ptr[i] != val) {
        res = false;
        break;
      }
    }
    munmap(ptr, o->size());
    return res;
  }
  return false;
}

void getTestAllocatorParams(std::vector<uint8_t> *params) {
  constexpr static int kAllocationSize = 1024 * 10;
  Params ashmemParams(kAllocationSize);

  params->assign(ashmemParams.array, ashmemParams.array + sizeof(ashmemParams));
}
