#include <pdx/service_dispatcher.h>

#include <errno.h>
#include <log/log.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>

#include <pdx/service.h>
#include <pdx/service_endpoint.h>

static const int kMaxEventsPerLoop = 128;

namespace android {
namespace pdx {

std::unique_ptr<ServiceDispatcher> ServiceDispatcher::Create() {
  std::unique_ptr<ServiceDispatcher> dispatcher{new ServiceDispatcher()};
  if (!dispatcher->epoll_fd_ || !dispatcher->event_fd_) {
    dispatcher.reset();
  }

  return dispatcher;
}

ServiceDispatcher::ServiceDispatcher() {
  event_fd_.Reset(eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK));
  if (!event_fd_) {
    ALOGE("Failed to create event fd because: %s\n", strerror(errno));
    return;
  }

  epoll_fd_.Reset(epoll_create1(EPOLL_CLOEXEC));
  if (!epoll_fd_) {
    ALOGE("Failed to create epoll fd because: %s\n", strerror(errno));
    return;
  }

  // Use "this" as a unique pointer to distinguish the event fd from all
  // the other entries that point to instances of Service.
  epoll_event event;
  event.events = EPOLLIN;
  event.data.ptr = this;

  if (epoll_ctl(epoll_fd_.Get(), EPOLL_CTL_ADD, event_fd_.Get(), &event) < 0) {
    ALOGE("Failed to add event fd to epoll fd because: %s\n", strerror(errno));

    // Close the fds here and signal failure to the factory method.
    event_fd_.Close();
    epoll_fd_.Close();
  }
}

ServiceDispatcher::~ServiceDispatcher() { SetCanceled(true); }

int ServiceDispatcher::ThreadEnter() {
  std::lock_guard<std::mutex> autolock(mutex_);

  if (canceled_)
    return -EBUSY;

  thread_count_++;
  return 0;
}

void ServiceDispatcher::ThreadExit() {
  std::lock_guard<std::mutex> autolock(mutex_);
  thread_count_--;
  condition_.notify_one();
}

int ServiceDispatcher::AddService(const std::shared_ptr<Service>& service) {
  std::lock_guard<std::mutex> autolock(mutex_);

  epoll_event event;
  event.events = EPOLLIN;
  event.data.ptr = service.get();

  if (epoll_ctl(epoll_fd_.Get(), EPOLL_CTL_ADD, service->endpoint()->epoll_fd(),
                &event) < 0) {
    ALOGE("Failed to add service to dispatcher because: %s\n", strerror(errno));
    return -errno;
  }

  services_.push_back(service);
  return 0;
}

int ServiceDispatcher::RemoveService(const std::shared_ptr<Service>& service) {
  std::lock_guard<std::mutex> autolock(mutex_);

  // It's dangerous to remove a service while other threads may be using it.
  if (thread_count_ > 0)
    return -EBUSY;

  epoll_event dummy;  // See BUGS in man 2 epoll_ctl.
  if (epoll_ctl(epoll_fd_.Get(), EPOLL_CTL_DEL, service->endpoint()->epoll_fd(),
                &dummy) < 0) {
    ALOGE("Failed to remove service from dispatcher because: %s\n",
          strerror(errno));
    return -errno;
  }

  services_.erase(std::remove(services_.begin(), services_.end(), service),
                  services_.end());
  return 0;
}

int ServiceDispatcher::ReceiveAndDispatch() { return ReceiveAndDispatch(-1); }

int ServiceDispatcher::ReceiveAndDispatch(int timeout) {
  int ret = ThreadEnter();
  if (ret < 0)
    return ret;

  epoll_event events[kMaxEventsPerLoop];

  int count = epoll_wait(epoll_fd_.Get(), events, kMaxEventsPerLoop, timeout);
  if (count <= 0) {
    ALOGE_IF(count < 0, "Failed to wait for epoll events because: %s\n",
             strerror(errno));
    ThreadExit();
    return count < 0 ? -errno : -ETIMEDOUT;
  }

  for (int i = 0; i < count; i++) {
    if (events[i].data.ptr == this) {
      ThreadExit();
      return -EBUSY;
    } else {
      Service* service = static_cast<Service*>(events[i].data.ptr);

      ALOGI_IF(TRACE, "Dispatching message: fd=%d\n",
               service->endpoint()->epoll_fd());
      service->ReceiveAndDispatch();
    }
  }

  ThreadExit();
  return 0;
}

int ServiceDispatcher::EnterDispatchLoop() {
  int ret = ThreadEnter();
  if (ret < 0)
    return ret;

  epoll_event events[kMaxEventsPerLoop];

  while (!IsCanceled()) {
    int count = epoll_wait(epoll_fd_.Get(), events, kMaxEventsPerLoop, -1);
    if (count < 0 && errno != EINTR) {
      ALOGE("Failed to wait for epoll events because: %s\n", strerror(errno));
      ThreadExit();
      return -errno;
    }

    for (int i = 0; i < count; i++) {
      if (events[i].data.ptr == this) {
        ThreadExit();
        return -EBUSY;
      } else {
        Service* service = static_cast<Service*>(events[i].data.ptr);

        ALOGI_IF(TRACE, "Dispatching message: fd=%d\n",
                 service->endpoint()->epoll_fd());
        service->ReceiveAndDispatch();
      }
    }
  }

  ThreadExit();
  return 0;
}

void ServiceDispatcher::SetCanceled(bool cancel) {
  std::unique_lock<std::mutex> lock(mutex_);
  canceled_ = cancel;

  if (canceled_ && thread_count_ > 0) {
    eventfd_write(event_fd_.Get(), 1);  // Signal threads to quit.

    condition_.wait(lock, [this] { return !(canceled_ && thread_count_ > 0); });

    eventfd_t value;
    eventfd_read(event_fd_.Get(), &value);  // Unsignal.
  }
}

bool ServiceDispatcher::IsCanceled() const { return canceled_; }

}  // namespace pdx
}  // namespace android
