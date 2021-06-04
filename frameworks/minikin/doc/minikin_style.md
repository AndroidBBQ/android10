### Minikin Style Guide

The C++ style in Minikin follows Android Framework C++ Code Style Guide except for following rules:

 * Order of include

 In dir/foo.cc or dir/foo_test.cc, whose main purpose is to implement or test the stuff in
 dir2/foo2.h, order your includes as follows:

   1. dir2/foo.h
   2. A blank line
   3. C system files
   4. C++ system files
   5. A blank line
   6. Other libraries' files
   7. A blank line
   8. Minikin public files
   9. A blank line
   10. Minikin private files

 For example,
 ```
 #include "minikin/Layout.h"  // The corresponding header file.

 #include <math.h>  // C system header files.
 #include <string>  // C++ system header files.

 #include <hb.h>  // Other library, HarfBuzz, header file.
 #include <log/log.h>  // Other library, Android, header file.
 #include <unicode/ubidi.h>  // Other library, ICU, header file.

 #include "minikin/Emoji.h"  // The minikin public header file.
 #include "HbFontCache.h"  // The minikin private header file.
 ```

 * "<>" vs ""

   * `#include <...>` should be used for non local library files.
   * `#include "..."` should be used for minikin header files.
