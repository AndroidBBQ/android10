#include <errno.h>
#include <ftw.h>
#include <getopt.h>
#include <pdx/client.h>
#include <pdx/service.h>
#include <sys/stat.h>

#include <algorithm>
#include <vector>

#include <pdx/default_transport/client_channel_factory.h>

using android::pdx::default_transport::ClientChannelFactory;

namespace {

constexpr long kClientTimeoutMs = 0;  // Don't wait for non-existent services.
constexpr int kDumpBufferSize = 2 * 4096;  // Two pages.

class ControlClient : public android::pdx::ClientBase<ControlClient> {
 public:
  explicit ControlClient(const std::string& service_path, long timeout_ms);

  void Reload();
  std::string Dump();

 private:
  friend BASE;

  ControlClient(const ControlClient&) = delete;
  void operator=(const ControlClient&) = delete;
};

bool option_verbose = false;

static struct option long_options[] = {
    {"reload", required_argument, 0, 0},
    {"dump", required_argument, 0, 0},
    {"verbose", no_argument, 0, 0},
    {0, 0, 0, 0},
};

#define printf_verbose(fmt, ... /*args*/) \
  do {                                    \
    if (option_verbose)                   \
      printf(fmt, ##__VA_ARGS__);         \
  } while (0)

void HexDump(const void* pointer, size_t length);

ControlClient::ControlClient(const std::string& service_path, long timeout_ms)
    : BASE{ClientChannelFactory::Create(service_path), timeout_ms} {}

void ControlClient::Reload() {
  android::pdx::Transaction trans{*this};
  auto status = trans.Send<void>(android::pdx::opcodes::REPORT_SYSPROP_CHANGE,
                                 nullptr, 0, nullptr, 0);
  if (!status) {
    fprintf(stderr, "Failed to send reload: %s\n",
            status.GetErrorMessage().c_str());
  }
}

std::string ControlClient::Dump() {
  android::pdx::Transaction trans{*this};
  std::vector<char> buffer(kDumpBufferSize);
  auto status = trans.Send<int>(android::pdx::opcodes::DUMP_STATE, nullptr, 0,
                                buffer.data(), buffer.size());

  printf_verbose("ControlClient::Dump: ret=%d\n", ReturnStatusOrError(status));

  if (!status) {
    fprintf(stderr, "Failed to send dump request: %s\n",
            status.GetErrorMessage().c_str());
    return "";
  } else if (status.get() > static_cast<ssize_t>(buffer.capacity())) {
    fprintf(stderr, "Service returned a larger size than requested: %d\n",
            status.get());
    return "";
  }

  if (option_verbose)
    HexDump(buffer.data(), status.get());

  return std::string(buffer.data(), status.get());
}

int Usage(const std::string& command_name) {
  printf("Usage: %s [options]\n", command_name.c_str());
  printf("\t--verbose                      : Use verbose messages.\n");
  printf(
      "\t--reload <all | service path>  : Ask service(s) to reload system "
      "properties.\n");
  printf("\t--dump <all | service path>    : Dump service(s) state.\n");
  return -1;
}

typedef int (*CallbackType)(const char* path, const struct stat* sb,
                            int type_flag, FTW* ftw_buffer);

int ReloadCommandCallback(const char* path, const struct stat* sb,
                          int type_flag, FTW* ftw_buffer);
int DumpCommandCallback(const char* path, const struct stat* sb, int type_flag,
                        FTW* ftw_buffer);

void CallOnAllFiles(CallbackType callback, const std::string& base_path) {
  const int kMaxDepth = 32;
  nftw(base_path.c_str(), callback, kMaxDepth, FTW_PHYS);
}

int ReloadCommand(const std::string& service_path) {
  printf_verbose("ReloadCommand: service_path=%s\n", service_path.c_str());

  if (service_path == "" || service_path == "all") {
    CallOnAllFiles(ReloadCommandCallback,
                   ClientChannelFactory::GetRootEndpointPath());
    return 0;
  } else {
    auto client = ControlClient::Create(service_path, kClientTimeoutMs);
    if (!client) {
      fprintf(stderr, "Failed to open service at \"%s\".\n",
              service_path.c_str());
      return -1;
    }

    client->Reload();
    return 0;
  }
}

int DumpCommand(const std::string& service_path) {
  printf_verbose("DumpCommand: service_path=%s\n", service_path.c_str());

  if (service_path == "" || service_path == "all") {
    CallOnAllFiles(DumpCommandCallback,
                   ClientChannelFactory::GetRootEndpointPath());
    return 0;
  } else {
    auto client = ControlClient::Create(service_path, kClientTimeoutMs);
    if (!client) {
      fprintf(stderr, "Failed to open service at \"%s\".\n",
              service_path.c_str());
      return -1;
    }

    std::string response = client->Dump();
    if (!response.empty()) {
      printf(
          "--------------------------------------------------------------------"
          "---\n");
      printf("%s:\n", service_path.c_str());
      printf("%s\n", response.c_str());
    }
    return 0;
  }
}

int ReloadCommandCallback(const char* path, const struct stat*, int type_flag,
                          FTW*) {
  if (type_flag == FTW_F)
    ReloadCommand(path);
  return 0;
}

int DumpCommandCallback(const char* path, const struct stat*, int type_flag,
                        FTW*) {
  if (type_flag == FTW_F)
    DumpCommand(path);
  return 0;
}

void HexDump(const void* pointer, size_t length) {
  uintptr_t address = reinterpret_cast<uintptr_t>(pointer);

  for (size_t count = 0; count < length; count += 16, address += 16) {
    printf("0x%08lx: ", static_cast<unsigned long>(address));

    for (size_t i = 0; i < 16u; i++) {
      if (i < std::min(length - count, static_cast<size_t>(16))) {
        printf("%02x ", *reinterpret_cast<const uint8_t*>(address + i));
      } else {
        printf("   ");
      }
    }

    printf("|");

    for (size_t i = 0; i < 16u; i++) {
      if (i < std::min(length - count, static_cast<size_t>(16))) {
        char c = *reinterpret_cast<const char*>(address + i);
        if (isalnum(c) || c == ' ') {
          printf("%c", c);
        } else {
          printf(".");
        }
      } else {
        printf(" ");
      }
    }

    printf("|\n");
  }
}

}  // anonymous namespace

int main(int argc, char** argv) {
  int getopt_code;
  int option_index;
  std::string option = "";
  std::string command = "";
  std::string command_argument = "";

  // Process command line options.
  while ((getopt_code =
              getopt_long(argc, argv, "", long_options, &option_index)) != -1) {
    option = long_options[option_index].name;
    printf_verbose("option=%s\n", option.c_str());
    switch (getopt_code) {
      case 0:
        if (option == "verbose") {
          option_verbose = true;
        } else {
          command = option;
          if (optarg)
            command_argument = optarg;
        }
        break;
    }
  }

  printf_verbose("command=%s command_argument=%s\n", command.c_str(),
                 command_argument.c_str());

  if (command == "") {
    return Usage(argv[0]);
  } else if (command == "reload") {
    return ReloadCommand(command_argument);
  } else if (command == "dump") {
    return DumpCommand(command_argument);
  } else {
    return Usage(argv[0]);
  }
}
