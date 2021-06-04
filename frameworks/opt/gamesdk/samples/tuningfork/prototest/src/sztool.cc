#include "dynamicproto.h"

#include "proto/tuningfork.pb.h"
#include "proto/tuningfork_clearcut_log.pb.h"
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <iostream>
#include <fstream>
#include <numeric>
#include <functional>
#include <algorithm>
#include <random>
#include <sstream>

using namespace google::protobuf;
using namespace google::protobuf::io;

using ::com::google::tuningfork::FidelityParams;
using ::com::google::tuningfork::Annotation;
using ::com::google::tuningfork::Settings;
using ::com::google::tuningfork::Settings_Histogram;

using ::logs::proto::tuningfork::TuningForkLogEvent;
using ::logs::proto::tuningfork::TuningForkHistogram;

namespace {

std::default_random_engine generator;
std::uniform_real_distribution<double> zero_one_random_double_dist(0,1);

double ZeroToOneRandomDouble() {
  return zero_one_random_double_dist(generator);
}

std::uniform_int_distribution<int32_t> int32_dist;
int32_t RandInt32() {
  return int32_dist(generator);
}

void Usage() {
  std::cerr << "Usage: sztool <your_tf_proto_name> <your_settings>\n";
}

constexpr size_t ERROR = -1;

size_t FieldSize(const FieldDescriptor* fd, bool outputTypes) {
  switch(fd->type()) {
    case FieldDescriptor::TYPE_BOOL:
      if(outputTypes)
        std::cout << fd->name() << " " << fd->type_name() << std::endl;
      // Do we want this?
      return 2;
    case FieldDescriptor::TYPE_ENUM:
      if(outputTypes)
        std::cout << fd->name() << " ";
      dynamicproto::print(fd->enum_type(), std::cerr);
      if(outputTypes)
        std::cerr << std::endl;
      // NB each field is optional, so there is another possibility,
      //  with the value unknown.
      return fd->enum_type()->value_count() + 1;
    default:
      return ERROR;
    }
}

string RandomId() {
  const int ID_LEN = 32;
  char id[ID_LEN];
  for(int i=0;i<ID_LEN-1;++i)
    id[i] = 32+(RandInt32()%90);
  id[ID_LEN-1]=0;
  return id;
}

const string ANNOTATION_MESSAGE_NAME = "Annotation";
const string FIDELITYPARAMS_MESSAGE_NAME = "FidelityParams";
void NextAnnotation(const std::vector<const FieldDescriptor*>& ann_extensions,
                    std::vector<int>& ann_values) {
  for(int i=0; i<ann_extensions.size(); ++i) {
    auto fd = ann_extensions[i];
    if(fd->type()==FieldDescriptor::TYPE_BOOL) {
      if(ann_values[i]==0) {
        ann_values[i]++;
        return;
      }
    }
    else {
      if(ann_values[i]<fd->enum_type()->value_count()) {
        ann_values[i]++;
        return;
      }
    }
    ann_values[i] = 0;
  }
}

Annotation
MakeAnnotation(const FileDescriptor* fdesc,
               const std::vector<const FieldDescriptor*>& ann_extensions,
               const std::vector<int>& ann_values) {
  Annotation a;
  Message* m = dynamicproto::newMessage(fdesc, ANNOTATION_MESSAGE_NAME);
  if(m) {
    const Reflection* r = m->GetReflection();
    for(int i=0; i<ann_extensions.size(); ++i) {
      auto fd = ann_extensions[i];
      if(fd->type()==FieldDescriptor::TYPE_BOOL) {
        r->SetBool(m, fd, ann_values[i]!=0);
      }
      else {
        // 0 means it's not set
        if(ann_values[i]>0)
          r->SetEnumValue(m, fd, ann_values[i]);
      }
    }
    std::string s;
    m->SerializeToString(&s);
    a.ParseFromString(s);
    delete m;
  }
  else
    std::cerr << "Warning, could not find " << ANNOTATION_MESSAGE_NAME
              << " in " << fdesc->name() << std::endl;
  return a;
}

FidelityParams
RandomFidelityParams(const FileDescriptor* fdesc,
                     const std::vector<const FieldDescriptor*>& fpExtensions) {
  FidelityParams p;
  Message* m = dynamicproto::newMessage(fdesc, FIDELITYPARAMS_MESSAGE_NAME);
  if(m) {
    const Reflection* r = m->GetReflection();
    for(int i=0; i<fpExtensions.size(); ++i) {
      auto fd = fpExtensions[i];
      switch(fd->type()) {
        case FieldDescriptor::TYPE_BOOL:
          r->SetBool(m, fd, RandInt32()%2);
          break;
        case FieldDescriptor::TYPE_ENUM:
          r->SetEnumValue(m, fd,
                          1 + (RandInt32() % fd->enum_type()->value_count()));
          break;
        case FieldDescriptor::TYPE_INT32:
          r->SetInt32(m, fd, RandInt32());
          break;
        case FieldDescriptor::TYPE_INT64:
          r->SetInt64(m, fd, RandInt32());
          break;
        case FieldDescriptor::TYPE_STRING:
          r->SetString(m, fd, RandomId());
          break;
        default:
          // Ignore the rest
          break;
      }
    }
    std::string s;
    m->SerializeToString(&s);
    p.ParseFromString(s);
    delete m;
  }
  else
    std::cerr << "Warning, could not find " << FIDELITYPARAMS_MESSAGE_NAME
              << " in " << fdesc->name() << std::endl;
  return p;
}

} // namespace

int main(int argc, char *argv[]) {

  using namespace std::placeholders;  // for _1, _2, _3...

  if(argc!=3) {
    Usage();
    return 1;
  }
  dynamicproto::init({".", "proto"});
  auto fdescs = dynamicproto::fileDescriptors(argv[1]);
  if(fdescs.size()<2) {
    std::cerr << "The proto file has errors: it must include one import "
              << "of tuningfork.proto" << std::endl;
    return -2;
  }

  // Get the annotation extensions and count the combinations
  std::vector<const FieldDescriptor*> ann_extensions;
  dynamicproto::extensionsOf(fdescs.back(), "com.google.tuningfork.Annotation",
                             ann_extensions);
  auto enums = dynamicproto::enums(fdescs.back());
  std::vector<size_t> ann_extensionsSize(ann_extensions.size());
  std::cout << "Annotation extensions:" << std::endl;
  std::transform(ann_extensions.begin(), ann_extensions.end(),
                 ann_extensionsSize.begin(),
                 std::bind(FieldSize, _1, true));
  for(auto a: ann_extensionsSize) {
    if(a==ERROR) {
      std::cerr << "Bad annotation (only bool and enum are supported)"
                << std::endl;
      return 2;
    }
  }
  size_t nAnnotationCombinations =
      std::accumulate(ann_extensionsSize.begin(),
                      ann_extensionsSize.end(),
                      1,
                      std::multiplies<size_t>());
  std::cerr << "Total number of annotation combinations = "
            << nAnnotationCombinations << std::endl;

  // FidelityParams extensions
  std::vector<const FieldDescriptor*> fpExtensions;
  dynamicproto::extensionsOf(fdescs.back(),
                             "com.google.tuningfork.FidelityParams",
                             fpExtensions);

  // Get the settings
  std::string settings_file_name = argv[2];
  ifstream settings_file(settings_file_name);
  IstreamInputStream settings_stream(&settings_file);
  if(!settings_file.good()) {
    std::cerr << "Bad file: " << argv[2] << std::endl;
    return 2;
  }
  Settings settings;
  TextFormat::Parse(&settings_stream, &settings);
  std::cout << "Read settings file " << settings_file_name << " :\n";
  size_t maxInstrumentationKeys =
      settings.aggregation_strategy().max_instrumentation_keys();

  const double fraction_of_annotations_explored = 0.2;
  // Fill in a clearcut log event and get its size
  TuningForkLogEvent ev;
  ev.set_device_id(RandomId());
  ev.set_apk_id(RandomId());
  *ev.mutable_fidelityparams() = RandomFidelityParams(fdescs.front(),
                                                      fpExtensions);
  ev.set_experiment_id(RandomId());
  // This assumes we have histogram settings for each instrument key, i.e. not
  //   relying on defaults
  int n = settings.histograms_size();
  int nset = 0;
  for(int i=0;i<n;++i) {
    std::vector<int> ann_values(ann_extensions.size(),0);
    const Settings_Histogram& h = settings.histograms(i);
    for(int c=0;c<nAnnotationCombinations; ++c) {
      TuningForkHistogram* live_h = ev.add_histograms();
      live_h->set_instrument_id(i);
      *live_h->mutable_annotation() = MakeAnnotation(fdescs.front(),
                                                     ann_extensions,
                                                     ann_values);
      NextAnnotation(ann_extensions, ann_values);
      // Only fill in some of the histograms
      if(ZeroToOneRandomDouble()<fraction_of_annotations_explored) {
        for(int i=0; i<h.n_buckets(); ++i)
          live_h->add_counts(RandInt32());
        nset++;
      }
    }
  }
  std::cout << "Assuming " << nset << " histograms are non-zero out of "
            << n*nAnnotationCombinations << std::endl;
  const std::string outfname = "clearcut_evt.bin";
  const std::string dbgoutfname = "clearcut_evt.txt";
  std::ofstream dbgfout(dbgoutfname);
  dbgfout << ev.DebugString() << std::endl;
  std::ofstream fout(outfname);
  ev.SerializeToOstream(&fout);
  std::cout << "Wrote example clearcut message to " << outfname << " and "
            << dbgoutfname <<std::endl;
  std::stringstream sout;
  ev.SerializeToOstream(&sout);
  std::cout << "Example message size = " << sout.str().length() << " bytes"
            << std::endl;
  return 0;
}
