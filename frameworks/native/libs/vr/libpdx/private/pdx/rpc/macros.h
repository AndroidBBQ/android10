#ifndef ANDROID_PDX_RPC_MACROS_H_
#define ANDROID_PDX_RPC_MACROS_H_

// Macros to apply other macros over all elements in a list.
//
// For example, for a macro A(x) and B(x, y):
// - FOR_EACH(A, 1, 2, 3) -> A(1) A(2) A(3).
// - FOR_EACH_BINARY(B, z, 1, 2, 3) -> B(z, 1) B(z, 2) B(z, 3)
// - FOR_EACH_LIST(A, 1, 2, 3) -> A(1), B(2), C(3)
// - FOR_EACH_BINARY_LIST(B, z, 1, 2, 3) -> B(z, 1), B(z, 2), B(z, 3)
//
// Empty lists are supported and will produce no output.

// Recursive expansion macros.
#define _PDX_EXPAND0(...) __VA_ARGS__
#define _PDX_EXPAND1(...) _PDX_EXPAND0(_PDX_EXPAND0(_PDX_EXPAND0(__VA_ARGS__)))
#define _PDX_EXPAND2(...) _PDX_EXPAND1(_PDX_EXPAND1(_PDX_EXPAND1(__VA_ARGS__)))
#define _PDX_EXPAND3(...) _PDX_EXPAND2(_PDX_EXPAND2(_PDX_EXPAND2(__VA_ARGS__)))
#define _PDX_EXPAND4(...) _PDX_EXPAND3(_PDX_EXPAND3(_PDX_EXPAND3(__VA_ARGS__)))
#define _PDX_EXPAND(...) _PDX_EXPAND4(_PDX_EXPAND4(_PDX_EXPAND4(__VA_ARGS__)))

// Required to workaround a bug in the VC++ preprocessor.
#define _PDX_INDIRECT_EXPAND(macro, args) macro args

// Defines a step separation for macro expansion.
#define _PDX_SEPARATOR

// Clears any remaining contents wrapped in parentheses.
#define _PDX_CLEAR(...)

// Introduces a first dummy argument and _PDX_CLEAR as second argument.
#define _PDX_CLEAR_IF_LAST() _, _PDX_CLEAR

// Returns the first argument of a list.
#define _PDX_FIRST_ARG(first, ...) first

// Returns the second argument of a list.
#define _PDX_SECOND_ARG(_, second, ...) second

// Expands the arguments and introduces a separator.
#define _PDX_EXPAND_NEXT_FUNC(_, next_func, ...)        \
  _PDX_INDIRECT_EXPAND(_PDX_SECOND_ARG, (_, next_func)) \
  _PDX_SEPARATOR

// Returns next_func if the next element is not (), or _PDX_CLEAR
// otherwise.
//
// _PDX_CLEAR_IF_LAST inserts an extra first dummy argument if peek is ().
#define _PDX_NEXT_FUNC(next_element, next_func) \
  _PDX_EXPAND_NEXT_FUNC(_PDX_CLEAR_IF_LAST next_element, next_func)

// Macros for the unary version of PDX_FOR_EACH.

// Applies the unary macro. Duplicated for macro recursive expansion.
#define _PDX_APPLY_1(macro, head, next, ...) \
  macro(head) _PDX_NEXT_FUNC(next, _PDX_APPLY_2)(macro, next, __VA_ARGS__)

// Applies the unary macro. Duplicated for macro recursive expansion.
#define _PDX_APPLY_2(macro, head, next, ...) \
  macro(head) _PDX_NEXT_FUNC(next, _PDX_APPLY_1)(macro, next, __VA_ARGS__)

// Stops expansion if __VA_ARGS__ is empty, calling _PDX_APPLY_1
// otherwise.
#define _PDX_HANDLE_EMPTY_ARGS(macro, ...)                    \
  _PDX_NEXT_FUNC(_PDX_FIRST_ARG(__VA_ARGS__()), _PDX_APPLY_1) \
  (macro, __VA_ARGS__, ())

// Applies a unary macro over all the elements in a list.
#define PDX_FOR_EACH(macro, ...) \
  _PDX_EXPAND(_PDX_HANDLE_EMPTY_ARGS(macro, __VA_ARGS__))

// Applies the unary macro at the end of a list. Duplicated for macro recursive
// expansion.
#define _PDX_APPLY_LIST_1(macro, head, next, ...) \
  , macro(head)                                   \
        _PDX_NEXT_FUNC(next, _PDX_APPLY_LIST_2)(macro, next, __VA_ARGS__)

// Applies the unary macro at the end of a list. Duplicated for macro recursive
// expansion.
#define _PDX_APPLY_LIST_2(macro, head, next, ...) \
  , macro(head)                                   \
        _PDX_NEXT_FUNC(next, _PDX_APPLY_LIST_1)(macro, next, __VA_ARGS__)

// Applies the unary macro at the start of a list.
#define _PDX_APPLY_LIST_0(macro, head, next, ...) \
  macro(head) _PDX_NEXT_FUNC(next, _PDX_APPLY_LIST_1)(macro, next, __VA_ARGS__)

// Stops expansion if __VA_ARGS__ is empty, calling _PDX_APPLY_LIST_0
// otherwise.
#define _PDX_HANDLE_EMPTY_LIST(macro, ...)                         \
  _PDX_NEXT_FUNC(_PDX_FIRST_ARG(__VA_ARGS__()), _PDX_APPLY_LIST_0) \
  (macro, __VA_ARGS__, ())

// Applies a unary macro over all the elements in a list.
#define PDX_FOR_EACH_LIST(macro, ...) \
  _PDX_EXPAND(_PDX_HANDLE_EMPTY_LIST(macro, __VA_ARGS__))

// Macros for the binary version of PDX_FOR_EACH.

// Applies the binary macro. Duplicated for macro recursive expansion.
#define _PDX_APPLY_BINARY_1(macro, arg, head, next, ...) \
  macro(arg, head)                                       \
      _PDX_NEXT_FUNC(next, _PDX_APPLY_BINARY_2)(macro, arg, next, __VA_ARGS__)

// Applies the binary macro. Duplicated for macro recursive expansion.
#define _PDX_APPLY_BINARY_2(macro, arg, head, next, ...) \
  macro(arg, head)                                       \
      _PDX_NEXT_FUNC(next, _PDX_APPLY_BINARY_1)(macro, arg, next, __VA_ARGS__)

// Version of _PDX_HANDLE_EMPTY_ARGS that takes 1 fixed argument for a
// binary macro.
#define _PDX_HANDLE_EMPTY_ARGS_BINARY(macro, arg, ...)               \
  _PDX_NEXT_FUNC(_PDX_FIRST_ARG(__VA_ARGS__()), _PDX_APPLY_BINARY_1) \
  (macro, arg, __VA_ARGS__, ())

// Applies a binary macro over all the elements in a list and a given argument.
#define PDX_FOR_EACH_BINARY(macro, arg, ...) \
  _PDX_EXPAND(_PDX_HANDLE_EMPTY_ARGS_BINARY(macro, arg, __VA_ARGS__))

// Applies the binary macro at the end of a list. Duplicated for macro recursive
// expansion.
#define _PDX_APPLY_BINARY_LIST_1(macro, arg, head, next, ...)        \
  , macro(arg, head) _PDX_NEXT_FUNC(next, _PDX_APPLY_BINARY_LIST_2)( \
        macro, arg, next, __VA_ARGS__)

// Applies the binary macro at the end of a list. Duplicated for macro recursive
// expansion.
#define _PDX_APPLY_BINARY_LIST_2(macro, arg, head, next, ...)        \
  , macro(arg, head) _PDX_NEXT_FUNC(next, _PDX_APPLY_BINARY_LIST_1)( \
        macro, arg, next, __VA_ARGS__)

// Applies the binary macro at the start of a list. Duplicated for macro
// recursive expansion.
#define _PDX_APPLY_BINARY_LIST_0(macro, arg, head, next, ...)      \
  macro(arg, head) _PDX_NEXT_FUNC(next, _PDX_APPLY_BINARY_LIST_1)( \
      macro, arg, next, __VA_ARGS__)

// Version of _PDX_HANDLE_EMPTY_LIST that takes 1 fixed argument for a
// binary macro.
#define _PDX_HANDLE_EMPTY_LIST_BINARY(macro, arg, ...)                    \
  _PDX_NEXT_FUNC(_PDX_FIRST_ARG(__VA_ARGS__()), _PDX_APPLY_BINARY_LIST_0) \
  (macro, arg, __VA_ARGS__, ())

// Applies a binary macro over all the elements in a list and a given argument.
#define PDX_FOR_EACH_BINARY_LIST(macro, arg, ...) \
  _PDX_EXPAND(_PDX_HANDLE_EMPTY_LIST_BINARY(macro, arg, __VA_ARGS__))

#endif  // ANDROID_PDX_RPC_MACROS_H_
