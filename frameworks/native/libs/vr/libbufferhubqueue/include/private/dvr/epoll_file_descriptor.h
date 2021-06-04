#ifndef ANDROID_DVR_EPOLL_FILE_DESCRIPTOR_H_
#define ANDROID_DVR_EPOLL_FILE_DESCRIPTOR_H_

#include <android-base/unique_fd.h>
#include <log/log.h>
#include <sys/epoll.h>

namespace android {
namespace dvr {

class EpollFileDescriptor {
 public:
  static const int CTL_ADD = EPOLL_CTL_ADD;
  static const int CTL_MOD = EPOLL_CTL_MOD;
  static const int CTL_DEL = EPOLL_CTL_DEL;

  EpollFileDescriptor() : fd_(-1) {}

  // Constructs an EpollFileDescriptor from an integer file descriptor and
  // takes ownership.
  explicit EpollFileDescriptor(int fd) : fd_(fd) {}

  bool IsValid() const { return fd_.get() >= 0; }

  int Create() {
    if (IsValid()) {
      ALOGW("epoll fd has already been created.");
      return -EALREADY;
    }

    fd_.reset(epoll_create1(EPOLL_CLOEXEC));

    if (fd_.get() < 0)
      return -errno;
    else
      return 0;
  }

  int Control(int op, int target_fd, epoll_event* ev) {
    if (epoll_ctl(fd_.get(), op, target_fd, ev) < 0)
      return -errno;
    else
      return 0;
  }

  int Wait(epoll_event* events, int maxevents, int timeout) {
    int ret = epoll_wait(fd_.get(), events, maxevents, timeout);

    if (ret < 0)
      return -errno;
    else
      return ret;
  }

  int Get() const { return fd_.get(); }

 private:
  base::unique_fd fd_;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_EPOLL_FILE_DESCRIPTOR_H_
