----------------------
Slang compiler version
----------------------

See SlangVersion in frameworks/compile/slang/slang_version.h.  The
bitcode wrapper has a field that indicates the version of the slang
compiler that produced that bitcode.  A bitcode consumer is allowed to
make certain assumptions if the version number is sufficiently high.
However, only user bitcode has a wrapper -- libclcore.bc does not.
Therefore, libclcore.bc must not violate ANY of the guarantees
provided at a particular SlangVersion that allow the aforementioned
assumptions.  This is important because when user bitcode is linked to
libclcore.bc (bcc::Script::LinkRuntime()) the linked bitcode is
treated as having the same SlangVersion as the user bitcode.  This
implies that whenever we modify (runtime, driver, bcc) to take
advantage of some new SlangVersion guarantee, we may have to update
libclcore.bc to conform to that guarantee.
