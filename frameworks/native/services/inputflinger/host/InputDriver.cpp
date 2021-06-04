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

#include <functional>
#include <stdint.h>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

#define LOG_TAG "InputDriver"

#define LOG_NDEBUG 0

#include "InputDriver.h"
#include "InputHost.h"

#include <hardware/input.h>
#include <input/InputDevice.h>
#include <utils/Log.h>
#include <utils/PropertyMap.h>
#include <utils/String8.h>

#define INDENT2 "    "

struct input_property_map {
    android::PropertyMap* propertyMap;
};

struct input_property {
    android::String8 key;
    android::String8 value;
};

struct input_device_identifier {
    const char* name;
    const char* uniqueId;
    input_bus_t bus;
    int32_t     vendorId;
    int32_t     productId;
    int32_t     version;
};

struct input_device_definition {
    std::vector<input_report_definition*> reportDefs;
};

struct input_device_handle {
    input_device_identifier_t* id;
    input_device_definition_t* def;
};

struct input_int_usage {
    input_usage_t usage;
    int32_t min;
    int32_t max;
    float   resolution;
};

struct input_collection {
    int32_t arity;
    std::vector<input_int_usage> intUsages;
    std::vector<input_usage_t> boolUsages;
};

struct InputCollectionIdHasher {
    std::size_t operator()(const input_collection_id& id) const {
        return std::hash<int>()(static_cast<int>(id));
    }
};

struct input_report_definition {
    std::unordered_map<input_collection_id_t, input_collection, InputCollectionIdHasher> collections;
};


namespace android {

static input_host_callbacks_t kCallbacks = {
    .create_device_identifier = create_device_identifier,
    .create_device_definition = create_device_definition,
    .create_input_report_definition = create_input_report_definition,
    .create_output_report_definition = create_output_report_definition,
    .free_report_definition = free_report_definition,
    .input_device_definition_add_report = input_device_definition_add_report,
    .input_report_definition_add_collection = input_report_definition_add_collection,
    .input_report_definition_declare_usage_int = input_report_definition_declare_usage_int,
    .input_report_definition_declare_usages_bool = input_report_definition_declare_usages_bool,
    .register_device = register_device,
    .input_allocate_report = input_allocate_report,
    .input_report_set_usage_int = input_report_set_usage_int,
    .input_report_set_usage_bool = input_report_set_usage_bool,
    .report_event = report_event,
    .input_get_device_property_map = input_get_device_property_map,
    .input_get_device_property = input_get_device_property,
    .input_get_property_key = input_get_property_key,
    .input_get_property_value = input_get_property_value,
    .input_free_device_property = input_free_device_property,
    .input_free_device_property_map = input_free_device_property_map,
};

InputDriver::InputDriver(const char* name) : mName(String8(name)) {
    const hw_module_t* module;
    int err = input_open(&module, name);
    LOG_ALWAYS_FATAL_IF(err != 0, "Input module %s not found", name);
    mHal = reinterpret_cast<const input_module_t*>(module);
}

void InputDriver::init() {
    mHal->init(mHal, static_cast<input_host_t*>(this), kCallbacks);
}

input_device_identifier_t* InputDriver::createDeviceIdentifier(
            const char* name, int32_t productId, int32_t vendorId,
            input_bus_t bus, const char* uniqueId) {
    auto identifier = new ::input_device_identifier {
        .name = name,
        .productId = productId,
        .vendorId = vendorId,
        .bus = bus,
        .uniqueId = uniqueId,
    };
    // TODO: store this identifier somewhere
    return identifier;
}

input_device_definition_t* InputDriver::createDeviceDefinition() {
    return new ::input_device_definition;
}

input_report_definition_t* InputDriver::createInputReportDefinition() {
    return new ::input_report_definition;
}

input_report_definition_t* InputDriver::createOutputReportDefinition() {
    return new ::input_report_definition;
}

void InputDriver::freeReportDefinition(input_report_definition_t* reportDef) {
    delete reportDef;
}

void InputDriver::inputDeviceDefinitionAddReport(input_device_definition_t* d,
        input_report_definition_t* r) {
    d->reportDefs.push_back(r);
}

void InputDriver::inputReportDefinitionAddCollection(input_report_definition_t* report,
        input_collection_id_t id, int32_t arity) {
    report->collections[id] = {.arity = arity};
}

void InputDriver::inputReportDefinitionDeclareUsageInt(input_report_definition_t* report,
        input_collection_id_t id, input_usage_t usage, int32_t min, int32_t max,
        float resolution) {
    if (report->collections.find(id) != report->collections.end()) {
        report->collections[id].intUsages.push_back({
                .usage = usage, .min = min, .max = max, .resolution = resolution});
    }
}

void InputDriver::inputReportDefinitionDeclareUsagesBool(input_report_definition_t* report,
        input_collection_id_t id, input_usage_t* usage, size_t usageCount) {
    if (report->collections.find(id) != report->collections.end()) {
        for (size_t i = 0; i < usageCount; ++i) {
            report->collections[id].boolUsages.push_back(usage[i]);
        }
    }
}

input_device_handle_t* InputDriver::registerDevice(input_device_identifier_t* id,
        input_device_definition_t* d) {
    ALOGD("Registering device %s with %zu input reports", id->name, d->reportDefs.size());
    // TODO: save this device handle
    return new input_device_handle{ .id = id, .def = d };
}

void InputDriver::unregisterDevice(input_device_handle_t* handle) {
    delete handle;
}

input_report_t* InputDriver::inputAllocateReport(input_report_definition_t* r) {
    ALOGD("Allocating input report for definition %p", r);
    return nullptr;
}

void InputDriver::inputReportSetUsageInt(input_report_t* r, input_collection_id_t id,
        input_usage_t usage, int32_t value, int32_t arity_index) {
}

void InputDriver::inputReportSetUsageBool(input_report_t* r, input_collection_id_t id,
        input_usage_t usage, bool value, int32_t arity_index) {
}

void InputDriver::reportEvent(input_device_handle_t* d, input_report_t* report) {
    ALOGD("report_event %p for handle %p", report, d);
}

input_property_map_t* InputDriver::inputGetDevicePropertyMap(input_device_identifier_t* id) {
    InputDeviceIdentifier idi;
    idi.name = id->name;
    idi.uniqueId = id->uniqueId;
    idi.bus = id->bus;
    idi.vendor = id->vendorId;
    idi.product = id->productId;
    idi.version = id->version;

    std::string configFile = getInputDeviceConfigurationFilePathByDeviceIdentifier(
            idi, INPUT_DEVICE_CONFIGURATION_FILE_TYPE_CONFIGURATION);
    if (configFile.empty()) {
        ALOGD("No input device configuration file found for device '%s'.",
                idi.name.c_str());
    } else {
        auto propMap = new input_property_map_t();
        status_t status = PropertyMap::load(String8(configFile.c_str()), &propMap->propertyMap);
        if (status) {
            ALOGE("Error loading input device configuration file for device '%s'. "
                    "Using default configuration.",
                    idi.name.c_str());
            delete propMap;
            return nullptr;
        }
        return propMap;
    }
    return nullptr;
}

input_property_t* InputDriver::inputGetDeviceProperty(input_property_map_t* map,
        const char* key) {
    String8 keyString(key);
    if (map != nullptr) {
        if (map->propertyMap->hasProperty(keyString)) {
            auto prop = new input_property_t();
            if (!map->propertyMap->tryGetProperty(keyString, prop->value)) {
                delete prop;
                return nullptr;
            }
            prop->key = keyString;
            return prop;
        }
    }
    return nullptr;
}

const char* InputDriver::inputGetPropertyKey(input_property_t* property) {
    if (property != nullptr) {
        return property->key.string();
    }
    return nullptr;
}

const char* InputDriver::inputGetPropertyValue(input_property_t* property) {
    if (property != nullptr) {
        return property->value.string();
    }
    return nullptr;
}

void InputDriver::inputFreeDeviceProperty(input_property_t* property) {
    if (property != nullptr) {
        delete property;
    }
}

void InputDriver::inputFreeDevicePropertyMap(input_property_map_t* map) {
    if (map != nullptr) {
        delete map->propertyMap;
        delete map;
    }
}

void InputDriver::dump(String8& result) {
    result.appendFormat(INDENT2 "HAL Input Driver (%s)\n", mName.string());
}

} // namespace android

// HAL wrapper functions

namespace android {

::input_device_identifier_t* create_device_identifier(input_host_t* host,
        const char* name, int32_t product_id, int32_t vendor_id,
        input_bus_t bus, const char* unique_id) {
    auto driver = static_cast<InputDriverInterface*>(host);
    return driver->createDeviceIdentifier(name, product_id, vendor_id, bus, unique_id);
}

input_device_definition_t* create_device_definition(input_host_t* host) {
    auto driver = static_cast<InputDriverInterface*>(host);
    return driver->createDeviceDefinition();
}

input_report_definition_t* create_input_report_definition(input_host_t* host) {
    auto driver = static_cast<InputDriverInterface*>(host);
    return driver->createInputReportDefinition();
}

input_report_definition_t* create_output_report_definition(input_host_t* host) {
    auto driver = static_cast<InputDriverInterface*>(host);
    return driver->createOutputReportDefinition();
}

void free_report_definition(input_host_t* host, input_report_definition_t* report_def) {
    auto driver = static_cast<InputDriverInterface*>(host);
    driver->freeReportDefinition(report_def);
}

void input_device_definition_add_report(input_host_t* host,
        input_device_definition_t* d, input_report_definition_t* r) {
    auto driver = static_cast<InputDriverInterface*>(host);
    driver->inputDeviceDefinitionAddReport(d, r);
}

void input_report_definition_add_collection(input_host_t* host,
        input_report_definition_t* report, input_collection_id_t id, int32_t arity) {
    auto driver = static_cast<InputDriverInterface*>(host);
    driver->inputReportDefinitionAddCollection(report, id, arity);
}

void input_report_definition_declare_usage_int(input_host_t* host,
        input_report_definition_t* report, input_collection_id_t id,
        input_usage_t usage, int32_t min, int32_t max, float resolution) {
    auto driver = static_cast<InputDriverInterface*>(host);
    driver->inputReportDefinitionDeclareUsageInt(report, id, usage, min, max, resolution);
}

void input_report_definition_declare_usages_bool(input_host_t* host,
        input_report_definition_t* report, input_collection_id_t id,
        input_usage_t* usage, size_t usage_count) {
    auto driver = static_cast<InputDriverInterface*>(host);
    driver->inputReportDefinitionDeclareUsagesBool(report, id, usage, usage_count);
}

input_device_handle_t* register_device(input_host_t* host,
        input_device_identifier_t* id, input_device_definition_t* d) {
    auto driver = static_cast<InputDriverInterface*>(host);
    return driver->registerDevice(id, d);
}

void unregister_device(input_host_t* host, input_device_handle_t* handle) {
    auto driver = static_cast<InputDriverInterface*>(host);
    driver->unregisterDevice(handle);
}

input_report_t* input_allocate_report(input_host_t* host, input_report_definition_t* r) {
    auto driver = static_cast<InputDriverInterface*>(host);
    return driver->inputAllocateReport(r);
}

void input_report_set_usage_int(input_host_t* host, input_report_t* r,
        input_collection_id_t id, input_usage_t usage, int32_t value, int32_t arity_index) {
    auto driver = static_cast<InputDriverInterface*>(host);
    driver->inputReportSetUsageInt(r, id, usage, value, arity_index);
}

void input_report_set_usage_bool(input_host_t* host, input_report_t* r,
        input_collection_id_t id, input_usage_t usage, bool value, int32_t arity_index) {
    auto driver = static_cast<InputDriverInterface*>(host);
    driver->inputReportSetUsageBool(r, id, usage, value, arity_index);
}

void report_event(input_host_t* host, input_device_handle_t* d, input_report_t* report) {
    auto driver = static_cast<InputDriverInterface*>(host);
    driver->reportEvent(d, report);
}

input_property_map_t* input_get_device_property_map(input_host_t* host,
        input_device_identifier_t* id) {
    auto driver = static_cast<InputDriverInterface*>(host);
    return driver->inputGetDevicePropertyMap(id);
}

input_property_t* input_get_device_property(input_host_t* host, input_property_map_t* map,
        const char* key) {
    auto driver = static_cast<InputDriverInterface*>(host);
    return driver->inputGetDeviceProperty(map, key);
}

const char* input_get_property_key(input_host_t* host, input_property_t* property) {
    auto driver = static_cast<InputDriverInterface*>(host);
    return driver->inputGetPropertyKey(property);
}

const char* input_get_property_value(input_host_t* host, input_property_t* property) {
    auto driver = static_cast<InputDriverInterface*>(host);
    return driver->inputGetPropertyValue(property);
}

void input_free_device_property(input_host_t* host, input_property_t* property) {
    auto driver = static_cast<InputDriverInterface*>(host);
    driver->inputFreeDeviceProperty(property);
}

void input_free_device_property_map(input_host_t* host, input_property_map_t* map) {
    auto driver = static_cast<InputDriverInterface*>(host);
    driver->inputFreeDevicePropertyMap(map);
}

} // namespace android
