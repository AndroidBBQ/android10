#include "include/dvr/dvr_configuration_data.h"

#include <private/dvr/display_client.h>

using android::dvr::display::ConfigFileType;
using android::dvr::display::DisplayClient;

extern "C" {

int dvrConfigurationDataGet(int config_type, uint8_t** data,
                            size_t* data_size) {
  if (!data || !data_size) {
    return -EINVAL;
  }

  auto client = DisplayClient::Create();
  if (!client) {
    ALOGE("dvrGetGlobalBuffer: Failed to create display client!");
    return -ECOMM;
  }

  ConfigFileType config_file_type = static_cast<ConfigFileType>(config_type);
  auto config_data_status =
      client->GetConfigurationData(config_file_type);

  if (!config_data_status) {
    return -config_data_status.error();
  }

  *data_size = config_data_status.get().size();
  *data = new uint8_t[*data_size];
  std::copy_n(config_data_status.get().begin(), *data_size, *data);
  return 0;
}

void dvrConfigurationDataDestroy(uint8_t* data) {
  delete[] data;
}

}  // extern "C"
