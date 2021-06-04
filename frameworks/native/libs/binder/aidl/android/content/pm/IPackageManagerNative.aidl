/*
**
** Copyright 2017, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

package android.content.pm;

/**
 * Parallel implementation of certain {@link PackageManager} APIs that need to
 * be exposed to native code.
 * <p>These APIs are a parallel definition to the APIs in PackageManager, so,
 * they can technically diverge. However, it's good practice to keep these
 * APIs in sync with each other.
 * <p>Because these APIs are exposed to native code, it's possible they will
 * be exposed to privileged components [such as UID 0]. Care should be taken
 * to avoid exposing potential security holes for methods where permission
 * checks are bypassed based upon UID alone.
 *
 * @hide
 */
interface IPackageManagerNative {
    /**
     * Returns a set of names for the given UIDs.
     * IMPORTANT: Unlike the Java version of this API, unknown UIDs are
     * not represented by 'null's. Instead, they are represented by empty
     * strings.
     */
    @utf8InCpp String[] getNamesForUids(in int[] uids);

    /**
     * Returns the name of the installer (a package) which installed the named
     * package. Preloaded packages return the string "preload". Sideloaded packages
     * return an empty string. Unknown or unknowable are returned as empty strings.
     */

    @utf8InCpp String getInstallerForPackage(in String packageName);

    /**
     * Returns the version code of the named package.
     * Unknown or unknowable versions are returned as 0.
     */

    long getVersionCodeForPackage(in String packageName);

    /**
     * Return if each app, identified by its package name allows its audio to be recorded.
     * Unknown packages are mapped to false.
     */
    boolean[] isAudioPlaybackCaptureAllowed(in @utf8InCpp String[] packageNames);

    /*  ApplicationInfo.isSystemApp() == true */
    const int LOCATION_SYSTEM = 0x1;
    /*  ApplicationInfo.isVendor() == true */
    const int LOCATION_VENDOR = 0x2;
    /*  ApplicationInfo.isProduct() == true */
    const int LOCATION_PRODUCT = 0x4;

    /**
     * Returns a set of bitflags about package location.
     * LOCATION_SYSTEM: getApplicationInfo(packageName).isSystemApp()
     * LOCATION_VENDOR: getApplicationInfo(packageName).isVendor()
     * LOCATION_PRODUCT: getApplicationInfo(packageName).isProduct()
     */
    int getLocationFlags(in @utf8InCpp String packageName);

    /**
     * Returns the target SDK version for the given package.
     * Unknown packages will cause the call to fail. The caller must check the
     * returned Status before using the result of this function.
     */
    int getTargetSdkVersionForPackage(in String packageName);

    /**
     * Returns the name of module metadata package, or empty string if device doesn't have such
     * package.
     */
    @utf8InCpp String getModuleMetadataPackageName();
}
