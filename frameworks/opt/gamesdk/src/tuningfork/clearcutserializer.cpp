/*
 * Copyright 2018 The Android Open Source Project
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

#include "clearcutserializer.h"

#include "tuningfork/protobuf_nano_util.h"
#include "nano/tuningfork_clearcut_log.pb.h"

namespace tuningfork {

bool ClearcutSerializer::writeCountArray(pb_ostream_t *stream, const pb_field_t *field,
                                       void *const *arg) {
    const Histogram* h = static_cast<Histogram*>(*arg);
    if(!pb_encode_tag(stream, PB_WT_STRING, logs_proto_tuningfork_TuningForkHistogram_counts_tag))
        return false;
    // Get the length of the data
    pb_ostream_t sizing_stream = PB_OSTREAM_SIZING;
    for (int i = 0; i < h->num_buckets_; ++i)
        pb_encode_varint(&sizing_stream, h->buckets_[i]);
    // Encode the length of the packed array in bytes
    if (!pb_encode_varint(stream, sizing_stream.bytes_written))
        return false;
    // Encode each item, without the type, since it's packed
    for (int i = 0; i < h->num_buckets_; ++i) {
        if(!pb_encode_varint(stream, h->buckets_[i]))
            return false;
    }
    return true;
}
bool ClearcutSerializer::writeCpuFreqs(pb_ostream_t *stream, const pb_field_t *field,
                                         void *const *arg) {
    std::vector<uint64_t>* v = static_cast<std::vector<uint64_t>*>(*arg);
    // Encode each item
    for (int i = 0; i < v->size(); ++i) {
        pb_encode_tag_for_field(stream, field);
        pb_encode_varint(stream, (*v)[i]);
    }
    return true;
}

void ClearcutSerializer::Fill(const Histogram& h, ClearcutHistogram& ch) {
     ch.counts.funcs.encode = writeCountArray;
     ch.counts.arg = (void*)(&h);
}

bool ClearcutSerializer::writeAnnotation(pb_ostream_t* stream, const pb_field_t *field,
                                         void *const *arg) {
    const Prong* p = static_cast<const Prong*>(*arg);
    if(p->annotation_.size()>0) {
        pb_encode_tag_for_field(stream, field);
        pb_encode_string(stream, &p->annotation_[0], p->annotation_.size());
    }
    return true;
}
void ClearcutSerializer::Fill(const Prong& p, ClearcutHistogram& h) {
    h.has_instrument_id = true;
    h.instrument_id = p.instrumentation_key_;
    h.annotation.funcs.encode = writeAnnotation;
    h.annotation.arg = (void*)(&p);
    Fill(p.histogram_, h);
}
void ClearcutSerializer::Fill(const ExtraUploadInfo& tdi, DeviceInfo& di) {
    di.has_total_memory_bytes = true;
    di.total_memory_bytes = tdi.total_memory_bytes;
    di.has_gl_es_version = true;
    di.gl_es_version = tdi.gl_es_version;
    di.build_fingerprint.funcs.encode = writeString;
    di.build_fingerprint.arg = (void*)&tdi.build_fingerprint;
    di.build_version_sdk.funcs.encode = writeString;
    di.build_version_sdk.arg = (void*)&tdi.build_version_sdk;
    di.cpu_max_freq_hz.funcs.encode = writeCpuFreqs;
    di.cpu_max_freq_hz.arg = (void*)&tdi.cpu_max_freq_hz;
}
bool ClearcutSerializer::writeHistograms(pb_ostream_t* stream, const pb_field_t *field,
                                         void *const *arg) {
    const ProngCache* pc =static_cast<const ProngCache*>(*arg);
    for (auto &p: pc->prongs_) {
        if (p->histogram_.Count() > 0) {
            ClearcutHistogram h;
            Fill(*p, h);
            pb_encode_tag_for_field(stream, field);
            // Get size, then fill object
            pb_ostream_t sizing_stream = PB_OSTREAM_SIZING;
            pb_encode(&sizing_stream, logs_proto_tuningfork_TuningForkHistogram_fields, &h);
            pb_encode_varint(stream, sizing_stream.bytes_written);
            pb_encode(stream, logs_proto_tuningfork_TuningForkHistogram_fields, &h);
        }
    }
    return true;
}
bool ClearcutSerializer::writeDeviceInfo(pb_ostream_t* stream, const pb_field_t *field,
                                         void *const *arg) {
    const ExtraUploadInfo* tdi =static_cast<const ExtraUploadInfo*>(*arg);
    DeviceInfo di;
    Fill(*tdi, di);
    pb_encode_tag_for_field(stream, field);
    // Get size, then fill object
    pb_ostream_t sizing_stream = PB_OSTREAM_SIZING;
    pb_encode(&sizing_stream, logs_proto_tuningfork_DeviceInfo_fields, &di);
    pb_encode_varint(stream, sizing_stream.bytes_written);
    pb_encode(stream, logs_proto_tuningfork_DeviceInfo_fields, &di);
    return true;
}

bool ClearcutSerializer::writeString(pb_ostream_t* stream, const pb_field_t *field,
                                           void *const *arg) {

    const std::string* str = static_cast<std::string*>(*arg);
    if(!pb_encode_tag_for_field(stream, field)) return false;
    return pb_encode_string(stream, (uint8_t*) str->data(), str->size());
}

void ClearcutSerializer::FillExtras(const ExtraUploadInfo& info,
                                    TuningForkLogEvent& evt) {
    evt.experiment_id.funcs.encode = writeString;
    evt.experiment_id.arg = (void*)&info.experiment_id;
    evt.session_id.funcs.encode = writeString;
    evt.session_id.arg = (void*)&info.session_id;
    evt.apk_package_name.funcs.encode = writeString;
    evt.apk_package_name.arg = (void*)&info.apk_package_name;
    evt.has_apk_version_code = true;
    evt.apk_version_code = info.apk_version_code;
    evt.has_tuningfork_version = true;
    evt.tuningfork_version = info.tuningfork_version;
}

void ClearcutSerializer::FillHistograms(const ProngCache& pc, TuningForkLogEvent &evt) {
    evt.histograms.funcs.encode = writeHistograms;
    evt.histograms.arg = (void*)&pc;
}

bool ClearcutSerializer::writeFidelityParams(pb_ostream_t* stream, const pb_field_t *field,
                                             void *const *arg) {
    const ProtobufSerialization* fp = static_cast<const ProtobufSerialization*>(*arg);
    if(fp->size()>0) {
        pb_encode_tag_for_field(stream, field);
        pb_encode_string(stream, &(*fp)[0], fp->size());
    }
    return true;
}
void ClearcutSerializer::SerializeEvent(const ProngCache& pc,
                                        const ProtobufSerialization& fidelity_params,
                                        const ExtraUploadInfo& device_info,
                                        ProtobufSerialization& evt_ser) {
    TuningForkLogEvent evt = logs_proto_tuningfork_TuningForkLogEvent_init_default;
    evt.fidelityparams.funcs.encode = writeFidelityParams;
    evt.fidelityparams.arg = (void*)&fidelity_params;
    FillHistograms(pc,evt);
    evt.has_device_info = true;
    Fill(device_info, evt.device_info);
    FillExtras(device_info, evt);
    VectorStream str {&evt_ser, 0};
    pb_ostream_t stream = {VectorStream::Write, &str, SIZE_MAX, 0};
    pb_encode(&stream, logs_proto_tuningfork_TuningForkLogEvent_fields, &evt);
}

} // namespace tuningfork
