/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef ANDROID_INPUT_DRIVER_H
#define ANDROID_INPUT_DRIVER_H

#include <stdint.h>
#include <sys/types.h>

#include "InputHost.h"

#include <hardware/input.h>
#include <utils/RefBase.h>
#include <utils/String8.h>

// Declare a concrete type for the HAL
struct input_host {
};

namespace android {

class InputDriverInterface : public input_host_t, public virtual RefBase {
protected:
    InputDriverInterface() = default;
    virtual ~InputDriverInterface() = default;

public:
    virtual void init() = 0;

    virtual input_device_identifier_t* createDeviceIdentifier(
            const char* name, int32_t productId, int32_t vendorId,
            input_bus_t bus, const char* uniqueId) = 0;
    virtual input_device_definition_t* createDeviceDefinition() = 0;
    virtual input_report_definition_t* createInputReportDefinition() = 0;
    virtual input_report_definition_t* createOutputReportDefinition() = 0;
    virtual void freeReportDefinition(input_report_definition_t* reportDef) = 0;

    virtual void inputDeviceDefinitionAddReport(input_device_definition_t* d,
            input_report_definition_t* r) = 0;
    virtual void inputReportDefinitionAddCollection(input_report_definition_t* report,
            input_collection_id_t id, int32_t arity) = 0;
    virtual void inputReportDefinitionDeclareUsageInt(input_report_definition_t* report,
            input_collection_id_t id, input_usage_t usage, int32_t min, int32_t max,
            float resolution) = 0;
    virtual void inputReportDefinitionDeclareUsagesBool(input_report_definition_t* report,
            input_collection_id_t id, input_usage_t* usage, size_t usageCount) = 0;

    virtual input_device_handle_t* registerDevice(input_device_identifier_t* id,
            input_device_definition_t* d) = 0;
    virtual void unregisterDevice(input_device_handle_t* handle) = 0;

    virtual input_report_t* inputAllocateReport(input_report_definition_t* r) = 0;
    virtual void inputReportSetUsageInt(input_report_t* r, input_collection_id_t id,
            input_usage_t usage, int32_t value, int32_t arity_index) = 0;
    virtual void inputReportSetUsageBool(input_report_t* r, input_collection_id_t id,
            input_usage_t usage, bool value, int32_t arity_index) = 0;
    virtual void reportEvent(input_device_handle_t* d, input_report_t* report) = 0;

    virtual input_property_map_t* inputGetDevicePropertyMap(input_device_identifier_t* id) = 0;
    virtual input_property_t* inputGetDeviceProperty(input_property_map_t* map,
            const char* key) = 0;
    virtual const char* inputGetPropertyKey(input_property_t* property) = 0;
    virtual const char* inputGetPropertyValue(input_property_t* property) = 0;
    virtual void inputFreeDeviceProperty(input_property_t* property) = 0;
    virtual void inputFreeDevicePropertyMap(input_property_map_t* map) = 0;

    virtual void dump(String8& result) = 0;
};

class InputDriver : public InputDriverInterface {
public:
    explicit InputDriver(const char* name);
    virtual ~InputDriver() = default;

    virtual void init() override;

    virtual input_device_identifier_t* createDeviceIdentifier(
            const char* name, int32_t productId, int32_t vendorId,
            input_bus_t bus, const char* uniqueId) override;
    virtual input_device_definition_t* createDeviceDefinition() override;
    virtual input_report_definition_t* createInputReportDefinition() override;
    virtual input_report_definition_t* createOutputReportDefinition() override;
    virtual void freeReportDefinition(input_report_definition_t* reportDef) override;

    virtual void inputDeviceDefinitionAddReport(input_device_definition_t* d,
            input_report_definition_t* r) override;
    virtual void inputReportDefinitionAddCollection(input_report_definition_t* report,
            input_collection_id_t id, int32_t arity) override;
    virtual void inputReportDefinitionDeclareUsageInt(input_report_definition_t* report,
            input_collection_id_t id, input_usage_t usage, int32_t min, int32_t max,
            float resolution) override;
    virtual void inputReportDefinitionDeclareUsagesBool(input_report_definition_t* report,
            input_collection_id_t id, input_usage_t* usage, size_t usageCount) override;

    virtual input_device_handle_t* registerDevice(input_device_identifier_t* id,
            input_device_definition_t* d) override;
    virtual void unregisterDevice(input_device_handle_t* handle) override;

    virtual input_report_t* inputAllocateReport(input_report_definition_t* r) override;
    virtual void inputReportSetUsageInt(input_report_t* r, input_collection_id_t id,
            input_usage_t usage, int32_t value, int32_t arity_index) override;
    virtual void inputReportSetUsageBool(input_report_t* r, input_collection_id_t id,
            input_usage_t usage, bool value, int32_t arity_index) override;
    virtual void reportEvent(input_device_handle_t* d, input_report_t* report) override;

    virtual input_property_map_t* inputGetDevicePropertyMap(input_device_identifier_t* id) override;
    virtual input_property_t* inputGetDeviceProperty(input_property_map_t* map,
            const char* key) override;
    virtual const char* inputGetPropertyKey(input_property_t* property) override;
    virtual const char* inputGetPropertyValue(input_property_t* property) override;
    virtual void inputFreeDeviceProperty(input_property_t* property) override;
    virtual void inputFreeDevicePropertyMap(input_property_map_t* map) override;

    virtual void dump(String8& result) override;

private:
    String8 mName;
    const input_module_t* mHal;
};


extern "C" {

input_device_identifier_t* create_device_identifier(input_host_t* host,
        const char* name, int32_t product_id, int32_t vendor_id,
        input_bus_t bus, const char* unique_id);

input_device_definition_t* create_device_definition(input_host_t* host);

input_report_definition_t* create_input_report_definition(input_host_t* host);

input_report_definition_t* create_output_report_definition(input_host_t* host);

void free_report_definition(input_host_t* host, input_report_definition_t* report_def);

void input_device_definition_add_report(input_host_t* host,
        input_device_definition_t* d, input_report_definition_t* r);

void input_report_definition_add_collection(input_host_t* host,
        input_report_definition_t* report, input_collection_id_t id, int32_t arity);

void input_report_definition_declare_usage_int(input_host_t* host,
        input_report_definition_t* report, input_collection_id_t id,
        input_usage_t usage, int32_t min, int32_t max, float resolution);

void input_report_definition_declare_usages_bool(input_host_t* host,
        input_report_definition_t* report, input_collection_id_t id,
        input_usage_t* usage, size_t usage_count);


input_device_handle_t* register_device(input_host_t* host,
        input_device_identifier_t* id, input_device_definition_t* d);

void unregister_device(input_host_t* host, input_device_handle_t* handle);

input_report_t* input_allocate_report(input_host_t* host, input_report_definition_t* r);

void input_report_set_usage_int(input_host_t* host, input_report_t* r,
        input_collection_id_t id, input_usage_t usage, int32_t value, int32_t arity_index);

void input_report_set_usage_bool(input_host_t* host, input_report_t* r,
        input_collection_id_t id, input_usage_t usage, bool value, int32_t arity_index);

void report_event(input_host_t* host, input_device_handle_t* d, input_report_t* report);

input_property_map_t* input_get_device_property_map(input_host_t* host,
        input_device_identifier_t* id);

input_property_t* input_get_device_property(input_host_t* host, input_property_map_t* map,
        const char* key);

const char* input_get_property_key(input_host_t* host, input_property_t* property);

const char* input_get_property_value(input_host_t* host, input_property_t* property);

void input_free_device_property(input_host_t* host, input_property_t* property);

void input_free_device_property_map(input_host_t* host, input_property_map_t* map);
}

} // namespace android
#endif // ANDROID_INPUT_DRIVER_H
