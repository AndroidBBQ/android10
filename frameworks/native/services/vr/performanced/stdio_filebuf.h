// Copyright (c) 2009-2014 by the contributors listed in CREDITS.TXT
// Copyright (c) 2016 Google, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef ANDROID_DVR_PERFORMANCED_STDIO_FILEBUF_H_
#define ANDROID_DVR_PERFORMANCED_STDIO_FILEBUF_H_

#include <cstdio>
#include <istream>
#include <locale>
#include <streambuf>

namespace android {
namespace dvr {

// An implementation of std::basic_streambuf backed by a FILE pointer. This is
// ported from the internal llvm-libc++ support for std::cin. It's really
// unfortunate that we have to do this, but the C++11 standard is too pendantic
// to support creating streams from file descriptors or FILE pointers. This
// implementation uses all standard interfaces, except for the call to
// std::__throw_runtime_error(), which is only needed to deal with exceeding
// locale encoding limits. This class is meant to be used for reading system
// files, which don't require exotic locale support, so this call could be
// removed in the future, if necessary.
//
// Original source file: llvm-libcxx/llvm-libc++/include/__std_stream
// Original class name: __stdinbuf
//
template <class _CharT>
class stdio_filebuf
    : public std::basic_streambuf<_CharT, std::char_traits<_CharT> > {
 public:
  typedef _CharT char_type;
  typedef std::char_traits<char_type> traits_type;
  typedef typename traits_type::int_type int_type;
  typedef typename traits_type::pos_type pos_type;
  typedef typename traits_type::off_type off_type;
  typedef typename traits_type::state_type state_type;

  explicit stdio_filebuf(FILE* __fp);
  ~stdio_filebuf() override;

 protected:
  virtual int_type underflow() override;
  virtual int_type uflow() override;
  virtual int_type pbackfail(int_type __c = traits_type::eof()) override;
  virtual void imbue(const std::locale& __loc) override;

 private:
  FILE* __file_;
  const std::codecvt<char_type, char, state_type>* __cv_;
  state_type __st_;
  int __encoding_;
  int_type __last_consumed_;
  bool __last_consumed_is_next_;
  bool __always_noconv_;

  stdio_filebuf(const stdio_filebuf&);
  stdio_filebuf& operator=(const stdio_filebuf&);

  int_type __getchar(bool __consume);

  static const int __limit = 8;
};

template <class _CharT>
stdio_filebuf<_CharT>::stdio_filebuf(FILE* __fp)
    : __file_(__fp),
      __last_consumed_(traits_type::eof()),
      __last_consumed_is_next_(false) {
  imbue(this->getloc());
}

template <class _CharT>
stdio_filebuf<_CharT>::~stdio_filebuf() {
  if (__file_)
    fclose(__file_);
}

template <class _CharT>
void stdio_filebuf<_CharT>::imbue(const std::locale& __loc) {
  __cv_ = &std::use_facet<std::codecvt<char_type, char, state_type> >(__loc);
  __encoding_ = __cv_->encoding();
  __always_noconv_ = __cv_->always_noconv();
  if (__encoding_ > __limit)
    std::__throw_runtime_error("unsupported locale for standard io");
}

template <class _CharT>
typename stdio_filebuf<_CharT>::int_type stdio_filebuf<_CharT>::underflow() {
  return __getchar(false);
}

template <class _CharT>
typename stdio_filebuf<_CharT>::int_type stdio_filebuf<_CharT>::uflow() {
  return __getchar(true);
}

template <class _CharT>
typename stdio_filebuf<_CharT>::int_type stdio_filebuf<_CharT>::__getchar(
    bool __consume) {
  if (__last_consumed_is_next_) {
    int_type __result = __last_consumed_;
    if (__consume) {
      __last_consumed_ = traits_type::eof();
      __last_consumed_is_next_ = false;
    }
    return __result;
  }
  char __extbuf[__limit];
  int __nread = std::max(1, __encoding_);
  for (int __i = 0; __i < __nread; ++__i) {
    int __c = getc(__file_);
    if (__c == EOF)
      return traits_type::eof();
    __extbuf[__i] = static_cast<char>(__c);
  }
  char_type __1buf;
  if (__always_noconv_)
    __1buf = static_cast<char_type>(__extbuf[0]);
  else {
    const char* __enxt;
    char_type* __inxt;
    std::codecvt_base::result __r;
    do {
      state_type __sv_st = __st_;
      __r = __cv_->in(__st_, __extbuf, __extbuf + __nread, __enxt, &__1buf,
                      &__1buf + 1, __inxt);
      switch (__r) {
        case std::codecvt_base::ok:
          break;
        case std::codecvt_base::partial:
          __st_ = __sv_st;
          if (__nread == sizeof(__extbuf))
            return traits_type::eof();
          {
            int __c = getc(__file_);
            if (__c == EOF)
              return traits_type::eof();
            __extbuf[__nread] = static_cast<char>(__c);
          }
          ++__nread;
          break;
        case std::codecvt_base::error:
          return traits_type::eof();
        case std::codecvt_base::noconv:
          __1buf = static_cast<char_type>(__extbuf[0]);
          break;
      }
    } while (__r == std::codecvt_base::partial);
  }
  if (!__consume) {
    for (int __i = __nread; __i > 0;) {
      if (ungetc(traits_type::to_int_type(__extbuf[--__i]), __file_) == EOF)
        return traits_type::eof();
    }
  } else
    __last_consumed_ = traits_type::to_int_type(__1buf);
  return traits_type::to_int_type(__1buf);
}

template <class _CharT>
typename stdio_filebuf<_CharT>::int_type stdio_filebuf<_CharT>::pbackfail(
    int_type __c) {
  if (traits_type::eq_int_type(__c, traits_type::eof())) {
    if (!__last_consumed_is_next_) {
      __c = __last_consumed_;
      __last_consumed_is_next_ =
          !traits_type::eq_int_type(__last_consumed_, traits_type::eof());
    }
    return __c;
  }
  if (__last_consumed_is_next_) {
    char __extbuf[__limit];
    char* __enxt;
    const char_type __ci = traits_type::to_char_type(__last_consumed_);
    const char_type* __inxt;
    switch (__cv_->out(__st_, &__ci, &__ci + 1, __inxt, __extbuf,
                       __extbuf + sizeof(__extbuf), __enxt)) {
      case std::codecvt_base::ok:
        break;
      case std::codecvt_base::noconv:
        __extbuf[0] = static_cast<char>(__last_consumed_);
        __enxt = __extbuf + 1;
        break;
      case std::codecvt_base::partial:
      case std::codecvt_base::error:
        return traits_type::eof();
    }
    while (__enxt > __extbuf)
      if (ungetc(*--__enxt, __file_) == EOF)
        return traits_type::eof();
  }
  __last_consumed_ = __c;
  __last_consumed_is_next_ = true;
  return __c;
}

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_PERFORMANCED_STDIO_FILEBUF_H_
