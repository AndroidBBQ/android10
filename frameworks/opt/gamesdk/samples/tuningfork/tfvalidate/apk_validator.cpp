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

#include <iostream>
#include <sstream>

#include "androidfw/ZipFileRO.h"

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

namespace {

const char kTfProtoFilename[] = "tuningfork.proto";
const char kDevProtoFilename[] = "dev_tuningfork.proto";
const char kSettingsFilename[] = "tuningfork_settings.bin";

int kMaxInstrumentationKeys = 256; // Sanity check

template <typename T>
std::ostream& operator<<(std::ostream& o, const std::vector<T>& vec) {
  o << "[";
  bool first = true;
  for (auto& i : vec) {
    if (!first)
      o << ",";
    else
      first = false;
    o << i;
  }
  o << "]";
  return o;
}

} // anonymous namespace

namespace tuningfork {

void ErrorCollector::AddError(const std::string& file_name, int line, int column,
                              const std::string& message) {
  std::cerr << "Error in: " << file_name << " line: " << line << " column: "
            << column << " message: " << message << std::endl;
}

#define ZERROR(CODE, MSG) {last_error_code_ = CODE; \
  {std::stringstream str; str << MSG; last_error_msg_ = str.str(); } \
  return nullptr;}

ZipSourceTree::ZipSourceTree(const std::shared_ptr<android::ZipFileRO>& zip_file,
                             const std::string& include_path)
    : zip_file_(zip_file), include_path_(include_path), last_error_code_(0) {
}

pb::io::ZeroCopyInputStream*
ZipSourceTree::Open(const std::string& relative_path) {
  std::string path = include_path_ + relative_path;
  auto it = file_cache_.find(path);
  if (file_cache_.end() == it) {
    android::ZipEntryRO entry = zip_file_->findEntryByName(path.c_str());
    if (entry == nullptr) {
      ZERROR(ERROR_NO_DEV_PROTO, "Could not find " << path);
    }
    uint32_t file_len = 0;
    if (!zip_file_->getEntryInfo(entry, NULL, &file_len,
                                 NULL, NULL, NULL, NULL)) {
      ZERROR(ERROR_CANT_READ_PROTO, "Could not read " << path);
    }
    it = file_cache_.insert({path,{}}).first;
    it->second.resize(file_len);
    if (!zip_file_->uncompressEntry(entry, it->second.data(), file_len)) {
      file_cache_.erase(it);
      ZERROR(ERROR_CANT_UNCOMPRESS_PROTO, "Could not uncompress " << path);
    }
  }
  return new pb::io::ArrayInputStream(it->second.data(), it->second.size());
}

std::string ZipSourceTree::GetLastErrorMessage() {
  return last_error_msg_;
}

int ZipSourceTree::GetLastErrorCode() {
  return last_error_code_;
}

const std::shared_ptr<android::ZipFileRO>& ZipSourceTree::ZipFile() const {
  return zip_file_;
}

std::string ZipSourceTree::IncludePath() const {
  return include_path_;
}

ApkValidator::ApkValidator(const std::shared_ptr<android::ZipFileRO>& zip_file,
                           const std::string& asset_path, bool debug)
    : source_tree_(zip_file, asset_path), debug_(debug) {
  importer_ = std::unique_ptr<pb::compiler::Importer>(
      new pb::compiler::Importer(&source_tree_, &error_collector_));
}

int ApkValidator::Validate(bool enforce_enums_in_annotations,
                           bool check_dev_fidelity_params) {
  // Import the dev proto
  const pb::FileDescriptor* fdesc = importer_->Import(kDevProtoFilename);
  if (!fdesc)
    ERROR(source_tree_.GetLastErrorCode(),
          source_tree_.GetLastErrorMessage());
  if (debug_)
    std::cout << "Uncompressed " << kDevProtoFilename << std::endl;
  DebugFileDesc(fdesc);

  // Check Annotations
  auto adesc = FindMessageIgnoringScope(fdesc, "Annotation");
  if (!adesc)
    ERROR(ERROR_GETTING_ANNOTATION, "Error finding Annotation");
  std::vector<int> def_ann_enum_size;
  auto aok = ValidateAnnotation(adesc, enforce_enums_in_annotations,
                                def_ann_enum_size);
  if (aok != NO_ERROR)
    ERROR(aok, "Error in Annotations");

  // Check FidelityParams
  auto fpdesc = FindMessageIgnoringScope(fdesc, "FidelityParams");
  if (!fpdesc)
    ERROR(ERROR_GETTING_FIDELITYPARAMS, "Error finding FidelityParams");
  auto fpok = ValidateFidelityParams(fpdesc);
  if (fpok != NO_ERROR)
    ERROR(fpok, "Error in FidelityParams");

  // Check the settings
  const pb::FileDescriptor* tfdesc = importer_->Import(kTfProtoFilename);
  if (!tfdesc)
    ERROR(source_tree_.GetLastErrorCode(),
          source_tree_.GetLastErrorMessage());
  if (debug_)
    std::cout << "Uncompressed " << kTfProtoFilename << std::endl;
  DebugFileDesc(tfdesc);
  auto sdesc = FindMessageIgnoringScope(tfdesc, "Settings");
  if (!sdesc)
    ERROR(ERROR_GETTING_SETTINGS, "Error finding Settings");
  auto sok = ValidateSettings(sdesc, def_ann_enum_size);
  if (sok != NO_ERROR)
    ERROR(sok, "Error validating settings");

  if (check_dev_fidelity_params) {
    return ValidateDevFidelityParams();
  }
  return NO_ERROR;
}

int ApkValidator::ValidateAnnotation(const pb::Descriptor* desc,
                                     bool enforce_enums_in_annotations,
                                     std::vector<int>& enum_sizes) {
  enum_sizes.clear();
  DebugDesc(desc);
  if (desc->oneof_decl_count() > 0 || desc->nested_type_count() > 0
      || desc->extension_count() > 0 || desc->extension_range_count() > 0)
    ERROR(ERROR_ANNOTATION_TOO_COMPLEX, "Annotation too complex");
  auto n = desc->field_count();
  for (int i=0; i < n; ++i) {
    auto field = desc->field(i);
    if (field->type() == pb::FieldDescriptor::TYPE_ENUM) {
      enum_sizes.push_back(field->enum_type()->value_count());
    }
    else {
      if (enforce_enums_in_annotations) {
        ERROR(ERROR_ANNOTATION_ONLY_ENUMS,
              "Annotation can only contain enums");
      } else {
        // Still check it's an int or bool
        switch (field->type()) {
          case pb::FieldDescriptor::TYPE_INT32:
          case pb::FieldDescriptor::TYPE_UINT32:
            enum_sizes.push_back(-1);
            break;
          case pb::FieldDescriptor::TYPE_BOOL:
            enum_sizes.push_back(2);
            break;
          default:
            ERROR(ERROR_ANNOTATION_ONLY_ENUMS,
                  "Annotation can only contain enums, ints or bools");
        }
      }
    }
  }
  return NO_ERROR;
}

int ApkValidator::ValidateFidelityParams(const pb::Descriptor* desc) {
  DebugDesc(desc);
  if (desc->oneof_decl_count() > 0 || desc->nested_type_count() > 0
      || desc->extension_count() > 0 || desc->extension_range_count() > 0)
    ERROR(ERROR_FIDELITYPARAMS_TOO_COMPLEX, "FidelityParams too complex");
  auto n = desc->field_count();
  for (int i=0; i < n; ++i) {
    auto field = desc->field(i);
    if (field->type() == pb::FieldDescriptor::TYPE_GROUP
        || field->type() == pb::FieldDescriptor::TYPE_MESSAGE
        || field->type() == pb::FieldDescriptor::TYPE_BYTES)
      ERROR(ERROR_FIDELITYPARAMS_BAD_TYPE,
            "FidelityParams can only contain scalars");
  }
  return NO_ERROR;
}

int ApkValidator::ValidateSettings(const pb::Descriptor* sdesc,
                                   const std::vector<int>& def_ann_enum_size) {
  auto settings_proto = factory_.GetPrototype(sdesc);
  if (!settings_proto)
    ERROR(ERROR_GETTING_SETTINGS, "Error making Settings prototype");
  pb::Message* m = settings_proto->New();
  if (!m)
    ERROR(ERROR_GETTING_SETTINGS, "Error making Settings object");
  std::unique_ptr<pb::io::ArrayInputStream> settings_bin(
      dynamic_cast<pb::io::ArrayInputStream*>(
          source_tree_.Open(kSettingsFilename)));
  if (!settings_bin)
    ERROR(ERROR_GETTING_SETTINGS, "Error reading tuningfork_settings.bin");
  if (debug_)
    std::cout << "Uncompressed " << kSettingsFilename << std::endl;
  const void* ptr=0;
  int sz = 0;
  if (!settings_bin->Next(&ptr,&sz))
    ERROR(ERROR_GETTING_SETTINGS, "Error reading tuningfork settings data");
  m->ParseFromString((const char*)ptr);
  const pb::Reflection* refl = m->GetReflection();
  auto aggregation_strategy_field = m->GetDescriptor()
                                    ->FindFieldByName("aggregation_strategy");
  const pb::Message& agg_strat = refl->GetMessage(*m, aggregation_strategy_field);
  const pb::Reflection* as_refl = agg_strat.GetReflection();
  auto max_ikeys_field = agg_strat.GetDescriptor()
                         ->FindFieldByName("max_instrumentation_keys");
  auto max_ikeys = as_refl->GetInt32(agg_strat, max_ikeys_field);
  if (debug_)
    std::cout << "Max instrumentation keys = " << max_ikeys << std::endl;
  if (max_ikeys < 1 || max_ikeys>kMaxInstrumentationKeys)
    ERROR(ERROR_BAD_MAX_INSTRUMENTATION_KEYS, "max_ikeys = " << max_ikeys);
  auto ann_enum_size_field = agg_strat.GetDescriptor()
                             ->FindFieldByName("annotation_enum_size");
  std::vector<int> ann_enum_size;
  int n = as_refl->FieldSize(agg_strat, ann_enum_size_field);
  for (int i=0; i < n; ++i)
    ann_enum_size.push_back(as_refl->GetRepeatedInt32(agg_strat,
                                                      ann_enum_size_field,
                                                      i));
  if (debug_) {
    std::cout << "Deduced annotation enum sizes from Annotation: "
              << def_ann_enum_size << std::endl;
    std::cout << "Annotation enum sizes in settings file: " << ann_enum_size << std::endl;
  }
  if (ann_enum_size.size() != def_ann_enum_size.size())
    ERROR(ERROR_BAD_ANNOTATION_ENUM_SIZE,
          "Annotation enum size length bad: "
          << ann_enum_size.size() << "!=" << def_ann_enum_size.size());
  for (int i=0; i < ann_enum_size.size(); ++i) {
    if (def_ann_enum_size[i] != -1 && ann_enum_size[i] != def_ann_enum_size[i])
      ERROR(ERROR_BAD_ANNOTATION_ENUM_SIZE,
            "Bad annotation enum size: "
            << ann_enum_size[i] << "!=" << def_ann_enum_size[i]);
  }
  return NO_ERROR;
}

int ApkValidator::ValidateDevFidelityParams() {
  void* cookie;
  android::ZipFileRO* zip_file = source_tree_.ZipFile().get();
  std::string prefix = source_tree_.IncludePath() + "dev_tuningfork_fidelityparams_";
  char suffix[] = "bin";
  zip_file->startIteration(&cookie, prefix.c_str(), suffix);
  int fp_count = 0;
  for (auto entry = zip_file->nextEntry(cookie);
       entry; entry=zip_file->nextEntry(cookie)) {
    ++fp_count;
  }
  zip_file->endIteration(cookie);
  if (debug_)
    std::cout << "Found " << fp_count << " files matching "
              << prefix + "*.bin" << std::endl;
  if (fp_count == 0)
    ERROR(ERROR_NO_DEV_FIDELITYPARAMS, "No dev fidelity params present");
  return NO_ERROR;
}

// Find a message type, ignoring the scope
const pb::Descriptor*
ApkValidator::FindMessageIgnoringScope(const pb::FileDescriptor* fdesc,
                                       const std::string& name) {
  int n = fdesc->message_type_count();
  for (int i=0; i<n; ++i) {
    auto desc = fdesc->message_type(i);
    if (desc->name() == name) return desc;
  }
  return nullptr;
}

void ApkValidator::DebugFileDesc(const pb::FileDescriptor* fdesc) {
  if (debug_) {
    std::cout << fdesc->name()
              << ": messages: " << fdesc->message_type_count()
              << ", dependencies: " << fdesc->dependency_count()
              << ", extensions: " << fdesc->extension_count()
              << ", enums: " << fdesc->enum_type_count()
              << std::endl;
  }
}

void ApkValidator::DebugDesc(const pb::Descriptor* desc) {
  if (debug_) {
    std::cout << desc->name()
              << ": fields: " << desc->field_count()
              << ", one-ofs: " << desc->oneof_decl_count()
              << ", nested: " << desc->nested_type_count()
              << ", enums: " << desc->enum_type_count()
              << ", extensions: " << desc->extension_count()
              << ", extension ranges: " << desc->extension_range_count()
              << std::endl;
  }
}

}
