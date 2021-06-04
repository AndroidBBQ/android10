// Copyright (C) 2019 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "apk_validator.h"

namespace tf = tuningfork;

namespace {

int validateApk(const char *apk_path, const std::string asset_path,
                bool enforce_enums_in_annotations,
                bool check_dev_fidelityparams, bool debug) {
#undef open
  std::shared_ptr<android::ZipFileRO> zipFile(
      android::ZipFileRO::open(apk_path));
#define open ___xxx_unix_open
  if (zipFile == nullptr) {
    ERROR(ERROR_CANT_FIND_FILE, "Could not open " << apk_path);
  }
  if (debug) {
    std::cout << "Opened " << apk_path << std::endl;
    std::cout << "Searching under " << asset_path << std::endl;
  }
  tf::ApkValidator validator(zipFile, asset_path, debug);
  return validator.Validate(enforce_enums_in_annotations,
                            check_dev_fidelityparams);
}

} // anonymous namespace

int main(int argc, char **argv) {
  std::string asset_path = "assets/tuningfork/";
  bool enforce_enums_in_annotations = true;
  bool check_dev_fidelityparams = true;
  bool debug = false;
  constexpr char usage_string[] =
      "Usage: tfvalidate [-a <asset_path_in_apk>] [-N] [-F] [-d] <apk_path>";
  int opt;
  optind = 1;
  while ((opt=getopt(argc, argv, "+a:NFd")) != -1){
    switch(opt) {
      case 'a':
        asset_path = optarg;
        if ( asset_path.size() > 0 && asset_path.back() != '/' )
          asset_path += '/';
        break;
      case 'N':
        enforce_enums_in_annotations = false;
        break;
      case 'F':
        check_dev_fidelityparams = false;
        break;
      case 'd':
        debug = true;
        break;
      default:
        ERROR(ERROR_BAD_USAGE, usage_string);
    }
  }
  if (optind < argc) {
    for (int i=optind; i<argc; ++i) {
      int ret = validateApk(argv[i], asset_path, enforce_enums_in_annotations,
                            check_dev_fidelityparams, debug);
      if (ret != NO_ERROR) {
        ERROR(ret, "Error validating " << argv[i]);
      } else {
        std::cout << "Validated " << argv[i] << std::endl;
      }
    }
  }
  else
    ERROR(ERROR_BAD_USAGE, usage_string);

  return NO_ERROR;
}
