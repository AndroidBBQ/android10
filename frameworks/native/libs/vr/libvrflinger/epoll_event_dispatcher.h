#ifndef ANDROID_DVR_SERVICES_DISPLAYD_EPOLL_EVENT_DISPATCHER_H_
#define ANDROID_DVR_SERVICES_DISPLAYD_EPOLL_EVENT_DISPATCHER_H_

#include <sys/epoll.h>

#include <atomic>
#include <functional>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include <pdx/file_handle.h>
#include <pdx/status.h>

namespace android {
namespace dvr {

class EpollEventDispatcher {
 public:
  // Function type for event handlers. The handler receives a bitmask of the
  // epoll events that occurred on the file descriptor associated with the
  // handler.
  using Handler = std::function<void(int)>;

  EpollEventDispatcher();
  ~EpollEventDispatcher();

  // |handler| is called on the internal dispatch thread when |fd| is signaled
  // by events in |event_mask|.
  pdx::Status<void> AddEventHandler(int fd, int event_mask, Handler handler);
  pdx::Status<void> RemoveEventHandler(int fd);

  void Stop();

 private:
  void EventThread();

  std::thread thread_;
  std::atomic<bool> exit_thread_{false};

  // Protects handlers_ and removed_handlers_ and serializes operations on
  // epoll_fd_ and event_fd_.
  std::mutex lock_;

  // Maintains a map of fds to event handlers. This is primarily to keep any
  // references alive that may be bound in the std::function instances. It is
  // not used at dispatch time to avoid performance problems with different
  // versions of std::unordered_map.
  std::unordered_map<int, Handler> handlers_;

  // List of fds to be removed from the map. The actual removal is performed
  // by the event dispatch thread to avoid races.
  std::vector<int> removed_handlers_;

  pdx::LocalHandle epoll_fd_;
  pdx::LocalHandle event_fd_;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_SERVICES_DISPLAYD_EPOLL_EVENT_DISPATCHER_H_
