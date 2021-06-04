/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include "wifi_hal/hal_tool.h"

#include <android-base/logging.h>

namespace android {
namespace wifi_system {
namespace {

wifi_error wifi_initialize_stub(wifi_handle* handle) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_wait_for_driver_ready_stub(void) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

void wifi_cleanup_stub(wifi_handle handle, wifi_cleaned_up_handler handler) {}

void wifi_event_loop_stub(wifi_handle handle) {}

void wifi_get_error_info_stub(wifi_error err, const char** msg) { *msg = NULL; }

wifi_error wifi_get_supported_feature_set_stub(wifi_interface_handle handle,
                                               feature_set* set) {
  return WIFI_ERROR_UNINITIALIZED;
}

wifi_error wifi_get_concurrency_matrix_stub(wifi_interface_handle handle,
                                            int max_size, feature_set* matrix,
                                            int* size) {
  return WIFI_ERROR_UNINITIALIZED;
}

wifi_error wifi_set_scanning_mac_oui_stub(wifi_interface_handle handle,
                                          unsigned char* oui_data) {
  return WIFI_ERROR_UNINITIALIZED;
}

/* List of all supported channels, including 5GHz channels */
wifi_error wifi_get_supported_channels_stub(wifi_handle handle, int* size,
                                            wifi_channel* list) {
  return WIFI_ERROR_UNINITIALIZED;
}

/* Enhanced power reporting */
wifi_error wifi_is_epr_supported_stub(wifi_handle handle) {
  return WIFI_ERROR_UNINITIALIZED;
}

/* multiple interface support */
wifi_error wifi_get_ifaces_stub(wifi_handle handle, int* num_ifaces,
                                wifi_interface_handle** ifaces) {
  return WIFI_ERROR_UNINITIALIZED;
}

wifi_error wifi_get_iface_name_stub(wifi_interface_handle iface, char* name,
                                    size_t size) {
  return WIFI_ERROR_UNINITIALIZED;
}

wifi_error wifi_set_iface_event_handler_stub(wifi_request_id id,
                                             wifi_interface_handle iface,
                                             wifi_event_handler eh) {
  return WIFI_ERROR_UNINITIALIZED;
}

wifi_error wifi_reset_iface_event_handler_stub(wifi_request_id id,
                                               wifi_interface_handle iface) {
  return WIFI_ERROR_UNINITIALIZED;
}

wifi_error wifi_start_gscan_stub(wifi_request_id id,
                                 wifi_interface_handle iface,
                                 wifi_scan_cmd_params params,
                                 wifi_scan_result_handler handler) {
  return WIFI_ERROR_UNINITIALIZED;
}

wifi_error wifi_stop_gscan_stub(wifi_request_id id,
                                wifi_interface_handle iface) {
  return WIFI_ERROR_UNINITIALIZED;
}

wifi_error wifi_get_cached_gscan_results_stub(wifi_interface_handle iface,
                                              byte flush, int max,
                                              wifi_cached_scan_results* results,
                                              int* num) {
  return WIFI_ERROR_UNINITIALIZED;
}

wifi_error wifi_set_bssid_hotlist_stub(wifi_request_id id,
                                       wifi_interface_handle iface,
                                       wifi_bssid_hotlist_params params,
                                       wifi_hotlist_ap_found_handler handler) {
  return WIFI_ERROR_UNINITIALIZED;
}

wifi_error wifi_reset_bssid_hotlist_stub(wifi_request_id id,
                                         wifi_interface_handle iface) {
  return WIFI_ERROR_UNINITIALIZED;
}

wifi_error wifi_set_significant_change_handler_stub(
    wifi_request_id id, wifi_interface_handle iface,
    wifi_significant_change_params params,
    wifi_significant_change_handler handler) {
  return WIFI_ERROR_UNINITIALIZED;
}

wifi_error wifi_reset_significant_change_handler_stub(
    wifi_request_id id, wifi_interface_handle iface) {
  return WIFI_ERROR_UNINITIALIZED;
}

wifi_error wifi_get_gscan_capabilities_stub(
    wifi_interface_handle handle, wifi_gscan_capabilities* capabilities) {
  return WIFI_ERROR_UNINITIALIZED;
}

wifi_error wifi_set_link_stats_stub(wifi_interface_handle iface,
                                    wifi_link_layer_params params) {
  return WIFI_ERROR_UNINITIALIZED;
}

wifi_error wifi_get_link_stats_stub(wifi_request_id id,
                                    wifi_interface_handle iface,
                                    wifi_stats_result_handler handler) {
  return WIFI_ERROR_UNINITIALIZED;
}

wifi_error wifi_clear_link_stats_stub(wifi_interface_handle iface,
                                      u32 stats_clear_req_mask,
                                      u32* stats_clear_rsp_mask, u8 stop_req,
                                      u8* stop_rsp) {
  return WIFI_ERROR_UNINITIALIZED;
}

wifi_error wifi_get_valid_channels_stub(wifi_interface_handle handle, int band,
                                        int max_channels,
                                        wifi_channel* channels,
                                        int* num_channels) {
  return WIFI_ERROR_UNINITIALIZED;
}

/* API to request RTT measurement */
wifi_error wifi_rtt_range_request_stub(wifi_request_id id,
                                       wifi_interface_handle iface,
                                       unsigned num_rtt_config,
                                       wifi_rtt_config rtt_config[],
                                       wifi_rtt_event_handler handler) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

/* API to cancel RTT measurements */
wifi_error wifi_rtt_range_cancel_stub(wifi_request_id id,
                                      wifi_interface_handle iface,
                                      unsigned num_devices, mac_addr addr[]) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

/* API to get RTT capability */
wifi_error wifi_get_rtt_capabilities_stub(wifi_interface_handle iface,
                                          wifi_rtt_capabilities* capabilities) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

/* API to enable RTT responder role */
wifi_error wifi_enable_responder_stub(wifi_request_id id,
                                      wifi_interface_handle iface,
                                      wifi_channel_info channel_hint,
                                      unsigned max_duration_seconds,
                                      wifi_channel_info* channel_used) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

/* API to disable RTT responder role */
wifi_error wifi_disable_responder_stub(wifi_request_id id,
                                       wifi_interface_handle iface) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

/* API to get available channel for RTT responder role */
wifi_error wifi_rtt_get_available_channel_stub(wifi_interface_handle iface,
                                               wifi_channel_info* channel) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_set_nodfs_flag_stub(wifi_interface_handle iface, u32 nodfs) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_start_logging_stub(wifi_interface_handle iface,
                                   u32 verbose_level, u32 flags,
                                   u32 max_interval_sec, u32 min_data_size,
                                   char* buffer_name) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_set_epno_list_stub(int id, wifi_interface_info* iface,
                                   const wifi_epno_params* params,
                                   wifi_epno_handler handler) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_reset_epno_list_stub(int id, wifi_interface_info* iface) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_set_country_code_stub(wifi_interface_handle iface,
                                      const char* code) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_get_firmware_memory_dump_stub(
    wifi_interface_handle iface, wifi_firmware_memory_dump_handler handler) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_set_log_handler_stub(wifi_request_id id,
                                     wifi_interface_handle iface,
                                     wifi_ring_buffer_data_handler handler) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_reset_log_handler_stub(wifi_request_id id,
                                       wifi_interface_handle iface) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_set_alert_handler_stub(wifi_request_id id,
                                       wifi_interface_handle iface,
                                       wifi_alert_handler handler) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_reset_alert_handler_stub(wifi_request_id id,
                                         wifi_interface_handle iface) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_get_firmware_version_stub(wifi_interface_handle iface,
                                          char* buffer, int buffer_size) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_get_ring_buffers_status_stub(wifi_interface_handle iface,
                                             u32* num_rings,
                                             wifi_ring_buffer_status* status) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_get_logger_supported_feature_set_stub(
    wifi_interface_handle iface, unsigned int* support) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_get_ring_data_stub(wifi_interface_handle iface,
                                   char* ring_name) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_get_driver_version_stub(wifi_interface_handle iface,
                                        char* buffer, int buffer_size) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_enable_tdls_stub(wifi_interface_handle iface, mac_addr addr,
                                 wifi_tdls_params* params,
                                 wifi_tdls_handler handler) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_disable_tdls_stub(wifi_interface_handle iface, mac_addr addr) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_get_tdls_status_stub(wifi_interface_handle iface, mac_addr addr,
                                     wifi_tdls_status* status) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_get_tdls_capabilities_stub(
    wifi_interface_handle iface, wifi_tdls_capabilities* capabilities) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_start_sending_offloaded_packet_stub(
    wifi_request_id id, wifi_interface_handle iface, u16 ether_type,
    u8* ip_packet, u16 ip_packet_len, u8* src_mac_addr, u8* dst_mac_addr,
    u32 period_msec) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_stop_sending_offloaded_packet_stub(
    wifi_request_id id, wifi_interface_handle iface) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_get_wake_reason_stats_stub(
    wifi_interface_handle iface,
    WLAN_DRIVER_WAKE_REASON_CNT* wifi_wake_reason_cnt) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_configure_nd_offload_stub(wifi_interface_handle iface,
                                          u8 enable) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_get_driver_memory_dump_stub(
    wifi_interface_handle iface, wifi_driver_memory_dump_callbacks callbacks) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_start_pkt_fate_monitoring_stub(wifi_interface_handle iface) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_get_tx_pkt_fates_stub(wifi_interface_handle handle,
                                      wifi_tx_report* tx_report_bufs,
                                      size_t n_requested_fates,
                                      size_t* n_provided_fates) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_get_rx_pkt_fates_stub(wifi_interface_handle handle,
                                      wifi_rx_report* rx_report_bufs,
                                      size_t n_requested_fates,
                                      size_t* n_provided_fates) {
  return WIFI_ERROR_NOT_SUPPORTED;
}
wifi_error wifi_nan_enable_request_stub(transaction_id id,
                                        wifi_interface_handle iface,
                                        NanEnableRequest* msg) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_nan_disable_request_stub(transaction_id id,
                                         wifi_interface_handle iface) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_nan_publish_request_stub(transaction_id id,
                                         wifi_interface_handle iface,
                                         NanPublishRequest* msg) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_nan_publish_cancel_request_stub(transaction_id id,
                                                wifi_interface_handle iface,
                                                NanPublishCancelRequest* msg) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_nan_subscribe_request_stub(transaction_id id,
                                           wifi_interface_handle iface,
                                           NanSubscribeRequest* msg) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_nan_subscribe_cancel_request_stub(
    transaction_id id, wifi_interface_handle iface,
    NanSubscribeCancelRequest* msg) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_nan_transmit_followup_request_stub(
    transaction_id id, wifi_interface_handle iface,
    NanTransmitFollowupRequest* msg) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_nan_stats_request_stub(transaction_id id,
                                       wifi_interface_handle iface,
                                       NanStatsRequest* msg) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_nan_config_request_stub(transaction_id id,
                                        wifi_interface_handle iface,
                                        NanConfigRequest* msg) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_nan_tca_request_stub(transaction_id id,
                                     wifi_interface_handle iface,
                                     NanTCARequest* msg) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_nan_beacon_sdf_payload_request_stub(
    transaction_id id, wifi_interface_handle iface,
    NanBeaconSdfPayloadRequest* msg) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_nan_register_handler_stub(wifi_interface_handle iface,
                                          NanCallbackHandler handlers) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_nan_get_version_stub(wifi_handle handle, NanVersion* version) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_nan_get_capabilities_stub(transaction_id id,
                                          wifi_interface_handle iface) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_nan_data_interface_create_stub(transaction_id id,
                                               wifi_interface_handle iface,
                                               char* iface_name) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_nan_data_interface_delete_stub(transaction_id id,
                                               wifi_interface_handle iface,
                                               char* iface_name) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_nan_data_request_initiator_stub(
    transaction_id id, wifi_interface_handle iface,
    NanDataPathInitiatorRequest* msg) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_nan_data_indication_response_stub(
    transaction_id id, wifi_interface_handle iface,
    NanDataPathIndicationResponse* msg) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_nan_data_end_stub(transaction_id id,
                                  wifi_interface_handle iface,
                                  NanDataPathEndRequest* msg) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_get_packet_filter_capabilities_stub(
    wifi_interface_handle handle, u32* version, u32* max_len) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

wifi_error wifi_set_packet_filter_stub(wifi_interface_handle handle,
                                       const u8* program, u32 len) {
  return WIFI_ERROR_NOT_SUPPORTED;
}

bool init_wifi_stub_hal_func_table(wifi_hal_fn* hal_fn) {
  if (hal_fn == NULL) {
    return false;
  }
  hal_fn->wifi_initialize = wifi_initialize_stub;
  hal_fn->wifi_wait_for_driver_ready = wifi_wait_for_driver_ready_stub;
  hal_fn->wifi_cleanup = wifi_cleanup_stub;
  hal_fn->wifi_event_loop = wifi_event_loop_stub;
  hal_fn->wifi_get_error_info = wifi_get_error_info_stub;
  hal_fn->wifi_get_supported_feature_set = wifi_get_supported_feature_set_stub;
  hal_fn->wifi_get_concurrency_matrix = wifi_get_concurrency_matrix_stub;
  hal_fn->wifi_set_scanning_mac_oui = wifi_set_scanning_mac_oui_stub;
  hal_fn->wifi_get_supported_channels = wifi_get_supported_channels_stub;
  hal_fn->wifi_is_epr_supported = wifi_is_epr_supported_stub;
  hal_fn->wifi_get_ifaces = wifi_get_ifaces_stub;
  hal_fn->wifi_get_iface_name = wifi_get_iface_name_stub;
  hal_fn->wifi_reset_iface_event_handler = wifi_reset_iface_event_handler_stub;
  hal_fn->wifi_start_gscan = wifi_start_gscan_stub;
  hal_fn->wifi_stop_gscan = wifi_stop_gscan_stub;
  hal_fn->wifi_get_cached_gscan_results = wifi_get_cached_gscan_results_stub;
  hal_fn->wifi_set_bssid_hotlist = wifi_set_bssid_hotlist_stub;
  hal_fn->wifi_reset_bssid_hotlist = wifi_reset_bssid_hotlist_stub;
  hal_fn->wifi_set_significant_change_handler =
      wifi_set_significant_change_handler_stub;
  hal_fn->wifi_reset_significant_change_handler =
      wifi_reset_significant_change_handler_stub;
  hal_fn->wifi_get_gscan_capabilities = wifi_get_gscan_capabilities_stub;
  hal_fn->wifi_set_link_stats = wifi_set_link_stats_stub;
  hal_fn->wifi_get_link_stats = wifi_get_link_stats_stub;
  hal_fn->wifi_clear_link_stats = wifi_clear_link_stats_stub;
  hal_fn->wifi_get_valid_channels = wifi_get_valid_channels_stub;
  hal_fn->wifi_rtt_range_request = wifi_rtt_range_request_stub;
  hal_fn->wifi_rtt_range_cancel = wifi_rtt_range_cancel_stub;
  hal_fn->wifi_get_rtt_capabilities = wifi_get_rtt_capabilities_stub;
  hal_fn->wifi_start_logging = wifi_start_logging_stub;
  hal_fn->wifi_set_epno_list = wifi_set_epno_list_stub;
  hal_fn->wifi_set_country_code = wifi_set_country_code_stub;
  hal_fn->wifi_enable_tdls = wifi_enable_tdls_stub;
  hal_fn->wifi_disable_tdls = wifi_disable_tdls_stub;
  hal_fn->wifi_get_tdls_status = wifi_get_tdls_status_stub;
  hal_fn->wifi_get_tdls_capabilities = wifi_get_tdls_capabilities_stub;
  hal_fn->wifi_set_nodfs_flag = wifi_set_nodfs_flag_stub;
  hal_fn->wifi_get_firmware_memory_dump = wifi_get_firmware_memory_dump_stub;
  hal_fn->wifi_set_log_handler = wifi_set_log_handler_stub;
  hal_fn->wifi_reset_log_handler = wifi_reset_log_handler_stub;
  hal_fn->wifi_set_alert_handler = wifi_set_alert_handler_stub;
  hal_fn->wifi_reset_alert_handler = wifi_reset_alert_handler_stub;
  hal_fn->wifi_get_firmware_version = wifi_get_firmware_version_stub;
  hal_fn->wifi_get_ring_buffers_status = wifi_get_ring_buffers_status_stub;
  hal_fn->wifi_get_logger_supported_feature_set =
      wifi_get_logger_supported_feature_set_stub;
  hal_fn->wifi_get_ring_data = wifi_get_ring_data_stub;
  hal_fn->wifi_get_driver_version = wifi_get_driver_version_stub;
  hal_fn->wifi_start_sending_offloaded_packet =
      wifi_start_sending_offloaded_packet_stub;
  hal_fn->wifi_stop_sending_offloaded_packet =
      wifi_stop_sending_offloaded_packet_stub;
  hal_fn->wifi_get_wake_reason_stats = wifi_get_wake_reason_stats_stub;
  hal_fn->wifi_configure_nd_offload = wifi_configure_nd_offload_stub;
  hal_fn->wifi_get_driver_memory_dump = wifi_get_driver_memory_dump_stub;
  hal_fn->wifi_start_pkt_fate_monitoring = wifi_start_pkt_fate_monitoring_stub;
  hal_fn->wifi_get_tx_pkt_fates = wifi_get_tx_pkt_fates_stub;
  hal_fn->wifi_get_rx_pkt_fates = wifi_get_rx_pkt_fates_stub;
  hal_fn->wifi_nan_enable_request = wifi_nan_enable_request_stub;
  hal_fn->wifi_nan_disable_request = wifi_nan_disable_request_stub;
  hal_fn->wifi_nan_publish_request = wifi_nan_publish_request_stub;
  hal_fn->wifi_nan_publish_cancel_request =
      wifi_nan_publish_cancel_request_stub;
  hal_fn->wifi_nan_subscribe_request = wifi_nan_subscribe_request_stub;
  hal_fn->wifi_nan_subscribe_cancel_request =
      wifi_nan_subscribe_cancel_request_stub;
  hal_fn->wifi_nan_transmit_followup_request =
      wifi_nan_transmit_followup_request_stub;
  hal_fn->wifi_nan_stats_request = wifi_nan_stats_request_stub;
  hal_fn->wifi_nan_config_request = wifi_nan_config_request_stub;
  hal_fn->wifi_nan_tca_request = wifi_nan_tca_request_stub;
  hal_fn->wifi_nan_beacon_sdf_payload_request =
      wifi_nan_beacon_sdf_payload_request_stub;
  hal_fn->wifi_nan_register_handler = wifi_nan_register_handler_stub;
  hal_fn->wifi_nan_get_version = wifi_nan_get_version_stub;
  hal_fn->wifi_nan_get_capabilities = wifi_nan_get_capabilities_stub;
  hal_fn->wifi_nan_data_interface_create = wifi_nan_data_interface_create_stub;
  hal_fn->wifi_nan_data_interface_delete = wifi_nan_data_interface_delete_stub;
  hal_fn->wifi_nan_data_request_initiator =
      wifi_nan_data_request_initiator_stub;
  hal_fn->wifi_nan_data_indication_response =
      wifi_nan_data_indication_response_stub;
  hal_fn->wifi_nan_data_end = wifi_nan_data_end_stub;
  hal_fn->wifi_get_packet_filter_capabilities =
      wifi_get_packet_filter_capabilities_stub;
  hal_fn->wifi_set_packet_filter = wifi_set_packet_filter_stub;

  return true;
}

}  // namespace

bool HalTool::InitFunctionTable(wifi_hal_fn* hal_fn) {
  if (!init_wifi_stub_hal_func_table(hal_fn)) {
    LOG(ERROR) << "Can not initialize the basic function pointer table";
    return false;
  }

  if (init_wifi_vendor_hal_func_table(hal_fn) != WIFI_SUCCESS) {
    LOG(ERROR) << "Can not initialize the vendor function pointer table";
    return false;
  }

  return true;
}

bool HalTool::CanGetValidChannels(wifi_hal_fn* hal_fn) {
  return hal_fn &&
         (hal_fn->wifi_get_valid_channels != wifi_get_valid_channels_stub);
}

}  // namespace wifi_system
}  // namespace android
