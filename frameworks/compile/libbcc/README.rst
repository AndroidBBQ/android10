===============================================================
libbcc: A Versatile Bitcode Execution Engine for Mobile Devices
===============================================================


Introduction
------------

libbcc is an LLVM bitcode execution engine that compiles the bitcode
to an in-memory executable. libbcc is versatile because:

* it implements both AOT (Ahead-of-Time) and JIT (Just-in-Time)
  compilation.

* Android devices demand fast start-up time, small size, and high
  performance *at the same time*. libbcc attempts to address these
  design constraints.

* it supports on-device linking. Each device vendor can supply his or
  her own runtime bitcode library (lib*.bc) that differentiates his or
  her system. Specialization becomes ecosystem-friendly.

libbcc provides:

* a *just-in-time bitcode compiler*, which translates the LLVM bitcode
  into machine code

* a *caching mechanism*, which can:

  * after each compilation, serialize the in-memory executable into a
    cache file.  Note that the compilation is triggered by a cache
    miss.
  * load from the cache file upon cache-hit.

Highlights of libbcc are:

* libbcc supports bitcode from various language frontends, such as
  Renderscript, GLSL (pixelflinger2).

* libbcc strives to balance between library size, launch time and
  steady-state performance:

  * The size of libbcc is aggressively reduced for mobile devices. We
    customize and improve upon the default Execution Engine from
    upstream. Otherwise, libbcc's execution engine can easily become
    at least 2 times bigger.

  * To reduce launch time, we support caching of
    binaries. Just-in-Time compilation are oftentimes Just-too-Late,
    if the given apps are performance-sensitive. Thus, we implemented
    AOT to get the best of both worlds: Fast launch time and high
    steady-state performance.

    AOT is also important for projects such as NDK on LLVM with
    portability enhancement. Launch time reduction after we
    implemented AOT is signficant::


     Apps          libbcc without AOT       libbcc with AOT
                   launch time in libbcc    launch time in libbcc
     App_1            1218ms                   9ms
     App_2            842ms                    4ms
     Wallpaper:
       MagicSmoke     182ms                    3ms
       Halo           127ms                    3ms
     Balls            149ms                    3ms
     SceneGraph       146ms                    90ms
     Model            104ms                    4ms
     Fountain         57ms                     3ms

    AOT also masks the launching time overhead of on-device linking
    and helps it become reality.

  * For steady-state performance, we enable VFP3 and aggressive
    optimizations.

* Currently we disable Lazy JITting.



API
---

**Basic:**

* **bccCreateScript** - Create new bcc script

* **bccRegisterSymbolCallback** - Register the callback function for external
  symbol lookup

* **bccReadBC** - Set the source bitcode for compilation

* **bccReadModule** - Set the llvm::Module for compilation

* **bccLinkBC** - Set the library bitcode for linking

* **bccPrepareExecutable** - *deprecated* - Use bccPrepareExecutableEx instead

* **bccPrepareExecutableEx** - Create the in-memory executable by either
  just-in-time compilation or cache loading

* **bccGetFuncAddr** - Get the entry address of the function

* **bccDisposeScript** - Destroy bcc script and release the resources

* **bccGetError** - *deprecated* - Don't use this


**Reflection:**

* **bccGetExportVarCount** - Get the count of exported variables

* **bccGetExportVarList** - Get the addresses of exported variables

* **bccGetExportFuncCount** - Get the count of exported functions

* **bccGetExportFuncList** - Get the addresses of exported functions

* **bccGetPragmaCount** - Get the count of pragmas

* **bccGetPragmaList** - Get the pragmas


**Debug:**

* **bccGetFuncCount** - Get the count of functions (including non-exported)

* **bccGetFuncInfoList** - Get the function information (name, base, size)



Cache File Format
-----------------

A cache file (denoted as \*.oBCC) for libbcc consists of several sections:
header, string pool, dependencies table, relocation table, exported
variable list, exported function list, pragma list, function information
table, and bcc context.  Every section should be aligned to a word size.
Here is the brief description of each sections:

* **Header** (MCO_Header) - The header of a cache file. It contains the
  magic word, version, machine integer type information (the endianness,
  the size of off_t, size_t, and ptr_t), and the size
  and offset of other sections.  The header section is guaranteed
  to be at the beginning of the cache file.

* **String Pool** (MCO_StringPool) - A collection of serialized variable
  length strings.  The strp_index in the other part of the cache file
  represents the index of such string in this string pool.

* **Dependencies Table** (MCO_DependencyTable) - The dependencies table.
  This table stores the resource name (or file path), the resource
  type (rather in APK or on the file system), and the SHA1 checksum.

* **Relocation Table** (MCO_RelocationTable) - *not enabled*

* **Exported Variable List** (MCO_ExportVarList) -
  The list of the addresses of exported variables.

* **Exported Function List** (MCO_ExportFuncList) -
  The list of the addresses of exported functions.

* **Pragma List** (MCO_PragmaList) - The list of pragma key-value pair.

* **Function Information Table** (MCO_FuncTable) - This is a table of
  function information, such as function name, function entry address,
  and function binary size.  Besides, the table should be ordered by
  function name.

* **Context** - The context of the in-memory executable, including
  the code and the data.  The offset of context should aligned to
  a page size, so that we can mmap the context directly into memory.

For furthur information, you may read `bcc_cache.h <include/bcc/bcc_cache.h>`_,
`CacheReader.cpp <lib/bcc/CacheReader.cpp>`_, and
`CacheWriter.cpp <lib/bcc/CacheWriter.cpp>`_ for details.



JIT'ed Code Calling Conventions
-------------------------------

1. Calls from Execution Environment or from/to within script:

   On ARM, the first 4 arguments will go into r0, r1, r2, and r3, in that order.
   The remaining (if any) will go through stack.

   For ext_vec_types such as float2, a set of registers will be used. In the case
   of float2, a register pair will be used. Specifically, if float2 is the first
   argument in the function prototype, float2.x will go into r0, and float2.y,
   r1.

   Note: stack will be aligned to the coarsest-grained argument. In the case of
   float2 above as an argument, parameter stack will be aligned to an 8-byte
   boundary (if the sizes of other arguments are no greater than 8.)

2. Calls from/to a separate compilation unit: (E.g., calls to Execution
   Environment if those runtime library callees are not compiled using LLVM.)

   On ARM, we use hardfp.  Note that double will be placed in a register pair.
