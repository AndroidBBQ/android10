/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef STAGEFRIGHT_CODEC2_COMPONENT_FACTORY_H_
#define STAGEFRIGHT_CODEC2_COMPONENT_FACTORY_H_

#include <C2Component.h>

#include <functional>
#include <memory>

/**
 * Component factory object that enables to create a component and/or interface from a dynamically
 * linked library. This is needed because the component/interfaces are managed objects, but we
 * cannot safely create a managed object and pass it in C.
 *
 * Components/interfaces typically inherit from std::enable_shared_from_this, but C requires
 * passing simple pointer, and shared_ptr constructor needs to know the class to be constructed
 * derives from enable_shared_from_this.
 *
 */
class C2ComponentFactory {
public:
    typedef std::function<void(::C2Component*)> ComponentDeleter;
    typedef std::function<void(::C2ComponentInterface*)> InterfaceDeleter;

    /**
     * Creates a component.
     *
     * This method SHALL return within 100ms.
     *
     * \param id        component ID for the created component
     * \param component shared pointer where the created component is stored. Cleared on
     *                  failure and updated on success.
     *
     * \retval C2_OK        the component was created successfully
     * \retval C2_TIMED_OUT could not create the component within the time limit (unexpected)
     * \retval C2_CORRUPTED some unknown error prevented the creation of the component (unexpected)
     *
     * \retval C2_NO_MEMORY not enough memory to create the component
     */
    virtual c2_status_t createComponent(
            c2_node_id_t id, std::shared_ptr<C2Component>* const component,
            ComponentDeleter deleter = std::default_delete<C2Component>()) = 0;

    /**
     * Creates a component interface.
     *
     * This method SHALL return within 100ms.
     *
     * \param id        component interface ID for the created interface
     * \param interface shared pointer where the created interface is stored. Cleared on
     *                  failure and updated on success.
     *
     * \retval C2_OK        the component interface was created successfully
     * \retval C2_TIMED_OUT could not create the component interface within the time limit
     *                      (unexpected)
     * \retval C2_CORRUPTED some unknown error prevented the creation of the component interface
     *                      (unexpected)
     *
     * \retval C2_NO_MEMORY not enough memory to create the component interface
     */
    virtual c2_status_t createInterface(
            c2_node_id_t id, std::shared_ptr<C2ComponentInterface>* const interface,
            InterfaceDeleter deleter = std::default_delete<C2ComponentInterface>()) = 0;

    virtual ~C2ComponentFactory() = default;

    typedef ::C2ComponentFactory* (*CreateCodec2FactoryFunc)(void);
    typedef void (*DestroyCodec2FactoryFunc)(::C2ComponentFactory*);
};


#endif // STAGEFRIGHT_CODEC2_COMPONENT_FACTORY_H_
