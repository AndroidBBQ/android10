/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef CAS_API_H_
#define CAS_API_H_

#include <vector>
#include <utils/String8.h>

//  Loadable CasPlugin shared libraries should define the entry points
//  as shown below:
//
//  extern "C" {
//      extern android::CasFactory *createCasFactory();
//      extern android::DescramblerFactory *createDescramblerFactory();
//  }

namespace android {

struct CasPlugin;

struct CasPluginDescriptor {
    int32_t CA_system_id;
    String8 name;
};

typedef std::vector<uint8_t> CasData;
typedef std::vector<uint8_t> CasSessionId;
typedef std::vector<uint8_t> CasEmm;
typedef std::vector<uint8_t> CasEcm;
typedef void (*CasPluginCallback)(
        void *appData,
        int32_t event,
        int32_t arg,
        uint8_t *data,
        size_t size);

typedef void (*CasPluginCallbackExt)(
        void *appData,
        int32_t event,
        int32_t arg,
        uint8_t *data,
        size_t size,
        const CasSessionId *sessionId);

struct CasFactory {
    CasFactory() {}
    virtual ~CasFactory() {}

    // Determine if the plugin can handle the CA scheme identified by CA_system_id.
    virtual bool isSystemIdSupported(
            int32_t CA_system_id) const = 0;

    // Get a list of the CA schemes supported by the plugin.
    virtual status_t queryPlugins(
            std::vector<CasPluginDescriptor> *descriptors) const = 0;

    // Construct a new instance of a CasPlugin given a CA_system_id
    virtual status_t createPlugin(
            int32_t CA_system_id,
            void *appData,
            CasPluginCallback callback,
            CasPlugin **plugin) = 0;

    // Construct a new extend instance of a CasPlugin given a CA_system_id
    virtual status_t createPlugin(
            int32_t CA_system_id,
            void *appData,
            CasPluginCallbackExt callback,
            CasPlugin **plugin) = 0;

private:
    CasFactory(const CasFactory &);
    CasFactory &operator=(const CasFactory &); /* NOLINT */
};

struct CasPlugin {
    CasPlugin() {}
    virtual ~CasPlugin() {}

    // Provide the CA private data from a CA_descriptor in the conditional
    // access table to a CasPlugin.
    virtual status_t setPrivateData(
            const CasData &privateData) = 0;

    // Open a session for descrambling a program, or one or more elementary
    // streams.
    virtual status_t openSession(CasSessionId *sessionId) = 0;

    // Close a previously opened session.
    virtual status_t closeSession(const CasSessionId &sessionId) = 0;

    // Provide the CA private data from a CA_descriptor in the program map
    // table to a CasPlugin.
    virtual status_t setSessionPrivateData(
            const CasSessionId &sessionId,
            const CasData &privateData) = 0;

    // Process an ECM from the ECM stream for this session’s elementary stream.
    virtual status_t processEcm(
            const CasSessionId &sessionId,
            const CasEcm &ecm) = 0;

    // Process an in-band EMM from the EMM stream.
    virtual status_t processEmm(
            const CasEmm &emm) = 0;

    // Deliver an event to the CasPlugin. The format of the event is specific
    // to the CA scheme and is opaque to the framework.
    virtual status_t sendEvent(
            int32_t event,
            int32_t arg,
            const CasData &eventData) = 0;

    // Deliver an session event to the CasPlugin. The format of the event is
    // specific to the CA scheme and is opaque to the framework.
    virtual status_t sendSessionEvent(
            const CasSessionId &sessionId,
            int32_t event,
            int32_t arg,
            const CasData &eventData) = 0;

   // Native implementation of the MediaCas Java API provision method.
    virtual status_t provision(
            const String8 &provisionString) = 0;

    // Native implementation of the MediaCas Java API refreshEntitlements method
    virtual status_t refreshEntitlements(
            int32_t refreshType,
            const CasData &refreshData) = 0;

private:
    CasPlugin(const CasPlugin &);
    CasPlugin &operator=(const CasPlugin &); /* NOLINT */
};

extern "C" {
    extern android::CasFactory *createCasFactory();
}

} // namespace android

#endif // CAS_API_H_
