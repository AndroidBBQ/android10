# GLES Layers

## EGL Loader Initialization
After standard entrypoints have all been populated unmodified, a GLES LayerLoader will be instantiated.  If debug layers are enabled, the LayerLoader will scan specified directories for layers, just like the Vulkan loader does.

If layering is enabled, the loader will search for and enumerate a specified layer list.  The layer list will be specified by colon separated filenames (see [Enabling layers](#Enabling-layers) below).

The layers will be traversed in the order they are specified, so the first layer will be directly below the application.  For each layer, it will track two entrypoints from the layer.  `AndroidGLESLayer_Initialize` and `AndroidGLESLayer_GetProcAddress`.
```cpp
typedef void* (*PFNEGLGETNEXTLAYERPROCADDRESSPROC)(void*, const char*);
void* AndroidGLESLayer_Initialize(void* layer_id, PFNEGLGETNEXTLAYERPROCADDRESSPROC get_next_layer_proc_address))
```

`AndroidGLESLayer_Initialize` is a new function that provides an identifier for the layer to use (layer_id) and an entrypoint that can be called to look up functions below the layer.  The entrypoint can be used like so:
```cpp
const char* func = "eglFoo";
void* gpa = get_next_layer_proc_address(layer_id, func);
```

Note that only GLES2+ entrypoints will be provided. If a layer tries to make independent GLES 1.x calls, they will be routed to GLES2+ libraries, which may not behave as expected.  Application calls to 1.x will not be affected.

AndroidGLESLayer_GetProcAddress is a new function designed for this layering system.  It takes the address of the next call in the chain that the layer should call when finished.  If there is only one layer, next will point directly to the driver for most functions.
```cpp
void* AndroidGLESLayer_GetProcAddress(const char *funcName, EGLFuncPointer next)
```

For each layer found, the GLES LayerLoader will call `AndroidGLESLayer_Initialize`, and then walk libEGL’s function lists and call `AndroidGLESLayer_GetProcAddress` for all known functions.   The layer can track that next address with any means it wants.  If the layer does not intercept the function, `AndroidGLESLayer_GetProcAddress` must return the same function address it was passed.  The LayerLoader will then update the function hook list to point to the layer’s entrypoint.

The layers are not required to do anything with the info provided by `AndroidGLESLayer_Initialize` or get_next_layer_proc_address, but providing them makes it easier for existing layers (like GAPID and RenderDoc) to support Android.  That way a layer can look up functions independently (i.e. not wait for calls to `AndroidGLESLayer_GetProcAddress`).  Layers must be sure to use gen_next_layer_proc_address if they look up function calls instead of eglGetProcAddress or they will not get an accurate answer.  eglGetProcAddress must be passed down the chain to the platform.

## Placing layers

Where layers can be found, in order of priority
 1. System location for root
    This requires root access
    ```bash
    adb root
    adb disable-verity
    adb reboot
    adb root
    adb shell setenforce 0
    adb shell mkdir -p /data/local/debug/gles
    adb push <layer>.so /data/local/debug/gles/
    ```

 2. Application's base directory
    Target application must be debuggable, or you must have root access:
     ```bash
     adb push libGLTrace.so /data/local/tmp
     adb shell run-as com.android.gl2jni cp /data/local/tmp/libGLTrace.so .
     adb shell run-as com.android.gl2jni ls | grep libGLTrace
      libGLTrace.so
     ```

 3. External APK
    Determine the ABI of your target application, then install an APK containing the layers you wish to load:
    ```bash
    adb install --abi armeabi-v7a layers.apk
    ```

 4. In the target application's APK

## Enabling layers

### Per application
Note these settings will persist across reboots:
```bash
# Enable layers
adb shell settings put global enable_gpu_debug_layers 1

# Specify target application
adb shell settings put global gpu_debug_app <package_name>

# Specify layer list (from top to bottom)
adb shell settings put global gpu_debug_layers_gles <layer1:layer2:layerN>

# Specify a package to search for layers
adb shell settings put global gpu_debug_layer_app <layer_package>
```
To disable the per-app layers:
```
adb shell settings delete global enable_gpu_debug_layers
adb shell settings delete global gpu_debug_app
adb shell settings delete global gpu_debug_layers_gles
adb shell settings delete global gpu_debug_layer_app
```

### Globally
These will be cleared on reboot:
```bash
# This will attempt to load layers for all applications, including native executables
adb shell setprop debug.gles.layers <layer1:layer2:layerN>
```


## Creating a layer

Layers must expose the following two functions described above:
```cpp
AndroidGLESLayer_Initialize
AndroidGLESLayer_GetProcAddress
```

For a simple layer that just wants to intercept a handful of functions, a passively initialized layer is the way to go.  It can simply wait for the EGL Loader to initialize the function it cares about.  See below for an example of creating a passive layer.

For more formalized layers that need to fully initialize up front, or layers that needs to look up extensions not known to the EGL loader, active layer initialization is the way to go.  The layer can utilize get_next_layer_proc_address provided by `AndroidGLESLayer_Initialize` to look up a function at any time.  The layer must still respond to `AndroidGLESLayer_GetProcAddress` requests from the loader so the platform knows where to route calls. See below for an example of creating an active layer.

### Example Passive Layer Initialization
```cpp
namespace {

std::unordered_map<std::string, EGLFuncPointer> funcMap;

EGLAPI EGLBoolean EGLAPIENTRY glesLayer_eglChooseConfig (
  EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size,
  EGLint *num_config) {

  EGLFuncPointer entry = funcMap["eglChooseConfig"];

  typedef EGLBoolean (*PFNEGLCHOOSECONFIGPROC)(
    EGLDisplay, const EGLint*, EGLConfig*, EGLint, EGLint*);

  PFNEGLCHOOSECONFIGPROC next = reinterpret_cast<PFNEGLCHOOSECONFIGPROC>(entry);

  return next(dpy, attrib_list, configs, config_size, num_config);
}

EGLAPI EGLFuncPointer EGLAPIENTRY eglGPA(const char* funcName) {

  #define GETPROCADDR(func) if(!strcmp(funcName, #func)) { \
    return (EGLFuncPointer)glesLayer_##func; }

  GETPROCADDR(eglChooseConfig);

  // Don't return anything for unrecognized functions
  return nullptr;
}

EGLAPI void EGLAPIENTRY glesLayer_InitializeLayer(
  void* layer_id, PFNEGLGETNEXTLAYERPROCADDRESSPROC get_next_layer_proc_address) {
     // This function is purposefully empty, since this layer does not proactively
     // look up any entrypoints
  }

EGLAPI EGLFuncPointer EGLAPIENTRY glesLayer_GetLayerProcAddress(
  const char* funcName, EGLFuncPointer next) {
  EGLFuncPointer entry = eglGPA(funcName);
  if (entry != nullptr) {
    funcMap[std::string(funcName)] = next;
    return entry;
  }
  return next;
}

}  // namespace

extern "C" {
  __attribute((visibility("default"))) EGLAPI void AndroidGLESLayer_Initialize(
    void* layer_id, PFNEGLGETNEXTLAYERPROCADDRESSPROC get_next_layer_proc_address) {
    return (void)glesLayer_InitializeLayer(layer_id, get_next_layer_proc_address);
  }
  __attribute((visibility("default"))) EGLAPI void* AndroidGLESLayer_GetProcAddres(
    const char *funcName, EGLFuncPointer next) {
    return (void*)glesLayer_GetLayerProcAddress(funcName, next);
  }
}
```

### Example Active Layer Initialization
```cpp
namespace {

std::unordered_map<std::string, EGLFuncPointer> funcMap;

EGLAPI EGLBoolean EGLAPIENTRY glesLayer_eglChooseConfig (
  EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size,
  EGLint *num_config) {

  EGLFuncPointer entry = funcMap["eglChooseConfig"];

  typedef EGLBoolean (*PFNEGLCHOOSECONFIGPROC)(
    EGLDisplay, const EGLint*, EGLConfig*, EGLint, EGLint*);

  PFNEGLCHOOSECONFIGPROC next = reinterpret_cast<PFNEGLCHOOSECONFIGPROC>(entry);

  return next(dpy, attrib_list, configs, config_size, num_config);
}

EGLAPI EGLFuncPointer EGLAPIENTRY eglGPA(const char* funcName) {

  #define GETPROCADDR(func) if(!strcmp(funcName, #func)) { \
    return (EGLFuncPointer)glesLayer_##func; }

  GETPROCADDR(eglChooseConfig);

  // Don't return anything for unrecognized functions
  return nullptr;
}

EGLAPI void EGLAPIENTRY glesLayer_InitializeLayer(
  void* layer_id, PFNEGLGETNEXTLAYERPROCADDRESSPROC get_next_layer_proc_address) {

  // Note: This is where the layer would populate its function map with all the
  // functions it cares about
  const char* func = “eglChooseConfig”;
  funcMap[func] = get_next_layer_proc_address(layer_id, func);
}

EGLAPI EGLFuncPointer EGLAPIENTRY glesLayer_GetLayerProcAddress(
  const char* funcName, EGLFuncPointer next) {
  EGLFuncPointer entry = eglGPA(funcName);
  if (entry != nullptr) {
    return entry;
  }

  return next;
}

}  // namespace

extern "C" {
  __attribute((visibility("default"))) EGLAPI void AndroidGLESLayer_Initialize(
    void* layer_id, PFNEGLGETNEXTLAYERPROCADDRESSPROC get_next_layer_proc_address) {
    return (void)glesLayer_InitializeLayer(layer_id, get_next_layer_proc_address);
  }
  __attribute((visibility("default"))) EGLAPI void* AndroidGLESLayer_GetProcAddres(
    const char *funcName, EGLFuncPointer next) {
    return (void*)glesLayer_GetLayerProcAddress(funcName, next);
  }
}
```

## Caveats
Only supports GLES 2.0+.

When layering is enabled, GLES 1.x exclusive functions will continue to route to GLES 1.x drivers.  But functions shared with GLES 2.0+ (like glGetString) will be routed to 2.0+ drivers, which can cause confusion.

## FAQ
 - Who can use layers?
   - GLES Layers can be loaded by any debuggable application, or for any application if you have root access
 - How do we know if layers are working on a device?
   - This feature is backed by Android CTS, so you can run `atest CtsGpuToolsHostTestCases`
 - How does a app determine if this feature is supported?
   - There are two ways.  First you can check against the version of Android.
     ```bash
     # Q is the first that will support this, so look for `Q` or 10 for release
     adb shell getprop ro.build.version.sdk 
     # Or look for the SDK version, which should be 29 for Q
     adb shell getprop ro.build.version.sdk
     ```
   - Secondly, if you want to determine from an application that can't call out to ADB for this, you can check for the [EGL_ANDROID_GLES_layers](../../specs/EGL_ANDROID_GLES_layers.txt). It simply indicates support of this layering system:
     ```cpp
     std::string display_extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
     if (display_extension.find("EGL_ANDROID_GLES_layers") != std::string::npos)
     {
        // Layers are supported!
     }
     ```
