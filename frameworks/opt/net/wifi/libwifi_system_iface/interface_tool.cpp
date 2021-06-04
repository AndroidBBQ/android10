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

#include "wifi_system/interface_tool.h"

#include <net/if_arp.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <linux/ethtool.h>
/* We need linux/if.h for flags like IFF_UP.  Sadly, it forward declares
   struct sockaddr and must be included after sys/socket.h. */
#include <linux/if.h>

#include <android-base/logging.h>
#include <android-base/unique_fd.h>

namespace android {
namespace wifi_system {
namespace {

const char kWlan0InterfaceName[] = "wlan0";

bool GetIfState(const char* if_name, int sock, struct ifreq* ifr) {
  memset(ifr, 0, sizeof(*ifr));
  if (strlcpy(ifr->ifr_name, if_name, sizeof(ifr->ifr_name)) >=
      sizeof(ifr->ifr_name)) {
    LOG(ERROR) << "Interface name is too long: " << if_name;
    return false;
  }

  if (TEMP_FAILURE_RETRY(ioctl(sock, SIOCGIFFLAGS, ifr)) != 0) {
    LOG(ERROR) << "Could not read interface state for " << if_name
               << " (" << strerror(errno) << ")";
    return false;
  }

  return true;
}

}  // namespace

bool InterfaceTool::GetUpState(const char* if_name) {
  base::unique_fd sock(socket(PF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0));
  if (sock.get() < 0) {
    LOG(ERROR) << "Failed to open socket to set up/down state ("
               << strerror(errno) << ")";
    return false;
  }

  struct ifreq ifr;
  if (!GetIfState(if_name, sock.get(), &ifr)) {
    return false;  // logging done internally
  }

  return ifr.ifr_flags & IFF_UP;
}

bool InterfaceTool::SetUpState(const char* if_name, bool request_up) {
  base::unique_fd sock(socket(PF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0));
  if (sock.get() < 0) {
    LOG(ERROR) << "Failed to open socket to set up/down state ("
               << strerror(errno) << ")";
    return false;
  }

  struct ifreq ifr;
  if (!GetIfState(if_name, sock.get(), &ifr)) {
    return false;  // logging done internally
  }

  const bool currently_up = ifr.ifr_flags & IFF_UP;
  if (currently_up == request_up) {
    return true;
  }

  if (request_up) {
    ifr.ifr_flags |= IFF_UP;
  } else {
    ifr.ifr_flags &= ~IFF_UP;
  }

  if (TEMP_FAILURE_RETRY(ioctl(sock.get(), SIOCSIFFLAGS, &ifr)) != 0) {
    LOG(ERROR) << "Could not set interface flags for " << if_name
               << " (" << strerror(errno) << ")";
    return false;
  }

  return true;
}

bool InterfaceTool::SetWifiUpState(bool request_up) {
  return SetUpState(kWlan0InterfaceName, request_up);
}

bool InterfaceTool::SetMacAddress(const char* if_name,
    const std::array<uint8_t, ETH_ALEN>& new_address) {
  struct ifreq ifr;
  static_assert(ETH_ALEN <= sizeof(ifr.ifr_hwaddr.sa_data),
    "new address is too long");

  base::unique_fd sock(socket(PF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0));
  if (sock.get() < 0) {
    LOG(ERROR) << "Failed to open socket to set MAC address ("
               << strerror(errno) << ")";
    return false;
  }

  if (!GetIfState(if_name, sock.get(), &ifr)) {
    return false;  // logging done internally
  }

  memset(&ifr.ifr_hwaddr, 0, sizeof(ifr.ifr_hwaddr));
  ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
  memcpy(ifr.ifr_hwaddr.sa_data, new_address.data(), new_address.size());
  if (TEMP_FAILURE_RETRY(ioctl(sock.get(), SIOCSIFHWADDR, &ifr)) != 0) {
    LOG(ERROR) << "Could not set interface MAC address for " << if_name
               << " (" << strerror(errno) << ")";
    return false;
  }

  return true;
}

std::array<uint8_t, ETH_ALEN> InterfaceTool::GetFactoryMacAddress(const char* if_name) {
  std::array<uint8_t, ETH_ALEN> paddr = {};
  struct ifreq ifr;
  struct ethtool_perm_addr *epaddr;

  base::unique_fd sock(socket(PF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0));
  if (sock.get() < 0) {
    LOG(ERROR) << "Failed to open socket to get factory MAC address ("
               << strerror(errno) << ")";
    return paddr;
  }

  if (!GetIfState(if_name, sock.get(), &ifr)) {
    return paddr;  // logging done internally
  }

  epaddr = (ethtool_perm_addr*) malloc(sizeof(struct ethtool_perm_addr) + ETH_ALEN);
  if (!epaddr) {
    LOG(ERROR) << "Failed to set memory for mac address ("
               << strerror(errno) << ")";
    return paddr;
  }

  epaddr->cmd = ETHTOOL_GPERMADDR;
  epaddr->size = ETH_ALEN;
  ifr.ifr_data = epaddr;

  if (TEMP_FAILURE_RETRY(ioctl(sock.get(), SIOCETHTOOL, &ifr)) != 0) {
    LOG(ERROR) << "Could not get factory address MAC for " << if_name
               << " (" << strerror(errno) << ")";
  } else if (epaddr->size == ETH_ALEN) {
    memcpy(paddr.data(), epaddr->data, ETH_ALEN);
  }
  return paddr;
}

}  // namespace wifi_system
}  // namespace android
