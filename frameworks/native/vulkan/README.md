# frameworks/native/vulkan

This subdirectory contains Android's Vulkan loader, as well as some Vulkan-related tools useful to platform developers.

## Coding Style

We follow the [Chromium coding style](https://www.chromium.org/developers/coding-style) for naming and formatting, except with four-space indentation instead of two spaces. In general, any C++ features supported by the prebuilt platform toolchain are allowed.

Use "clang-format -style=file" to format all C/C++ code, except code imported verbatim from elsewhere. Setting up git-clang-format in your environment is recommended.

## Code Generation

We generate several parts of the loader and tools from a Vulkan API description file, stored in `api/vulkan.api`. Code generation must be done manually because the generator tools aren't part of the platform toolchain (yet?). Files named `foo_gen.*` are generated from the API file and a template file named `foo.tmpl`.

 To run the generator:

### One-time setup
- Install [golang](https://golang.org/), if you don't have it already.
- Create a directory (e.g. `$HOME/lib/go`) for local go sources and binaries and add it to `$GOPATH`.
- `$ git clone https://android.googlesource.com/platform/tools/gpu $GOPATH/src/android.googlesource.com/platform/tools/gpu`
- `$ go get android.googlesource.com/platform/tools/gpu/api/...`
- You should now have `$GOPATH/bin/apic`. You might want to add `$GOPATH/bin` to your `$PATH`.

### Generating code
To generate `libvulkan/*_gen.*`,
- `$ cd libvulkan`
- `$ apic template ../api/vulkan.api code-generator.tmpl`
Similar for `nulldrv/null_driver_gen.*`.
