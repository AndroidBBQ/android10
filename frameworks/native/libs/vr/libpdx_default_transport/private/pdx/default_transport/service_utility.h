#ifndef ANDROID_PDX_DEFAULT_TRANSPORT_SERVICE_UTILITY_H_
#define ANDROID_PDX_DEFAULT_TRANSPORT_SERVICE_UTILITY_H_

#include <ftw.h>

#include <pdx/client.h>
#include <pdx/default_transport/client_channel_factory.h>
#include <pdx/service.h>
#include <pdx/status.h>

namespace android {
namespace pdx {
namespace default_transport {

class ServiceUtility : public ClientBase<ServiceUtility> {
 public:
  Status<int> ReloadSystemProperties() {
    Transaction transaction{*this};
    return ReturnStatusOrError(
        transaction.Send<int>(opcodes::REPORT_SYSPROP_CHANGE));
  }

  static std::string GetRootEndpointPath() {
    return ClientChannelFactory::GetRootEndpointPath();
  }
  static std::string GetEndpointPath(const std::string& endpoint_path) {
    return ClientChannelFactory::GetEndpointPath(endpoint_path);
  }

  // Traverses the PDX service path space and sends a message to reload system
  // properties to each service endpoint it finds along the way.
  // NOTE: This method is used by atrace to poke PDX services. Please avoid
  // unnecessary changes to this mechanism to minimize impact on atrace.
  static bool PokeServices() {
    const int kMaxDepth = 16;
    const int result =
        nftw(GetRootEndpointPath().c_str(), PokeService, kMaxDepth, FTW_PHYS);
    return result == 0 ? true : false;
  }

 private:
  friend BASE;

  explicit ServiceUtility(const std::string& endpoint_path,
                          int* error = nullptr)
      : BASE(ClientChannelFactory::Create(endpoint_path), 0) {
    if (error)
      *error = Client::error();
  }

  // Sends the sysprop_change message to the service at fpath, so it re-reads
  // its system properties. Returns 0 on success or a negated errno code on
  // failure.
  // NOTE: This method is used by atrace to poke PDX services. Please avoid
  // unnecessary changes to this mechanism to minimize impact on atrace.
  static int PokeService(const char* fpath, const struct stat* /*sb*/,
                         int typeflag, struct FTW* /*ftwbuf*/) {
    const bool kIgnoreErrors = true;

    if (typeflag == FTW_F) {
      int error;
      auto utility = ServiceUtility::Create(fpath, &error);
      if (!utility) {
        if (error != -ECONNREFUSED) {
          ALOGE("ServiceUtility::PokeService: Failed to open %s: %s.", fpath,
                strerror(-error));
        }
        return kIgnoreErrors ? 0 : error;
      }

      auto status = utility->ReloadSystemProperties();
      if (!status) {
        ALOGE(
            "ServiceUtility::PokeService: Failed to send sysprop change to %s: "
            "%s",
            fpath, status.GetErrorMessage().c_str());
        return kIgnoreErrors ? 0 : -status.error();
      }
    }

    return 0;
  }

  ServiceUtility(const ServiceUtility&) = delete;
  void operator=(const ServiceUtility&) = delete;
};

}  // namespace default_transport
}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_DEFAULT_TRANSPORT_SERVICE_UTILITY_H_
