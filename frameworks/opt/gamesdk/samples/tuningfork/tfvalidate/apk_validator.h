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

#pragma once

#include <string>
#include <map>
#include <memory>

#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/compiler/importer.h>

#include "androidfw/ZipFileRO.h"

#include "apk_errors.h"

namespace tuningfork {

namespace pb = google::protobuf;

class ErrorCollector : public pb::compiler::MultiFileErrorCollector {
 public:
  void AddError(const std::string& file_name, int line, int column,
                const std::string& message) override;
};

class ZipSourceTree : public pb::compiler::SourceTree {
  std::shared_ptr<android::ZipFileRO> zip_file_;
  std::string include_path_;
  std::string last_error_msg_;
  int last_error_code_;
  std::map<std::string, std::vector<char>> file_cache_;

 public:
  ZipSourceTree(const std::shared_ptr<android::ZipFileRO>& zip_file,
                const std::string& include_path);
  pb::io::ZeroCopyInputStream* Open(const std::string& relative_path) override;
  std::string GetLastErrorMessage() override;
  int GetLastErrorCode();
  const std::shared_ptr<android::ZipFileRO>& ZipFile() const;
  std::string IncludePath() const;
};

class ApkValidator {
  ZipSourceTree source_tree_;
  ErrorCollector error_collector_;
  std::unique_ptr<pb::compiler::Importer> importer_;
  pb::DynamicMessageFactory factory_;
  bool debug_;

 public:
  ApkValidator(const std::shared_ptr<android::ZipFileRO>& zip_file,
               const std::string& asset_path, bool debug);
  int Validate(bool enforce_enums_in_annotations,
               bool check_dev_fidelity_params);

 private:
  int ValidateAnnotation(const pb::Descriptor* desc,
                         bool enforce_enums_in_annotations,
                         std::vector<int>& enum_sizes);
  int ValidateFidelityParams(const pb::Descriptor* desc);
  int ValidateSettings(const pb::Descriptor* sdesc,
                       const std::vector<int>& def_ann_enum_size);
  int ValidateDevFidelityParams();
  const pb::Descriptor* FindMessageIgnoringScope(
      const pb::FileDescriptor* fdesc,
      const std::string& name);
  void DebugFileDesc(const pb::FileDescriptor* fdesc);
  void DebugDesc(const pb::Descriptor* desc);

};

}
