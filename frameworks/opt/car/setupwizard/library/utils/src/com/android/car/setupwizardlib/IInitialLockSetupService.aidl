/*
 * Copyright (C) 2019 The Android Open Source Project
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

package com.android.car.setupwizardlib;

import src.com.android.car.setupwizardlib.LockConfig;

interface IInitialLockSetupService {

    /**
     * Returns the version of the library that the service was built on.
     */
    int getServiceVersion() = 0;

    /**
     * Returns a {@link LockConfig} with the lock configuration for the given
     * {@link InitialLockSetupConstants#LockTypes} type.
     */
    LockConfig getLockConfig(in int lockType) = 1;


    /**
     * Returns the error flags for the entered password. If there is no error and the password is
     * valid then it will return 0.
     */
    int checkValidLock(in int lockType, in byte[] password) = 2;

    /**
     * Sets the lock password according to the lock type. This lock should be
     * serialized based on the methods provided by the library to ensure it is
     * deserializable by the service.
     */
    int setLock(in int lockType, in byte[] password) = 3;
}

