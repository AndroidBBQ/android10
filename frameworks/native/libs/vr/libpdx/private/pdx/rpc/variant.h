#ifndef ANDROID_PDX_RPC_VARIANT_H_
#define ANDROID_PDX_RPC_VARIANT_H_

#include <cstdint>
#include <tuple>
#include <type_traits>

namespace android {
namespace pdx {
namespace rpc {

// Type tag denoting an empty variant.
struct EmptyVariant {};

namespace detail {

// Type for matching tagged overloads.
template <typename T>
struct TypeTag {};

// Determines the type of the I-th element of Types....
template <std::size_t I, typename... Types>
using TypeForIndex = std::tuple_element_t<I, std::tuple<Types...>>;

// Determines the type tag for the I-th element of Types....
template <std::size_t I, typename... Types>
using TypeTagForIndex = TypeTag<TypeForIndex<I, Types...>>;

// Similar to std::is_constructible except that it evaluates to false for bool
// construction from pointer types: this helps prevent subtle to bugs caused by
// assigning values that decay to pointers to Variants with bool elements.
//
// Here is an example of the problematic situation this trait avoids:
//
//  Variant<int, bool> v;
//  const int array[3] = {1, 2, 3};
//  v = array; // This is allowed by regular std::is_constructible.
//
template <typename...>
struct IsConstructible;
template <typename T, typename U>
struct IsConstructible<T, U>
    : std::integral_constant<bool,
                             std::is_constructible<T, U>::value &&
                                 !(std::is_same<std::decay_t<T>, bool>::value &&
                                   std::is_pointer<std::decay_t<U>>::value)> {};
template <typename T, typename... Args>
struct IsConstructible<T, Args...> : std::is_constructible<T, Args...> {};

// Enable if T(Args...) is well formed.
template <typename R, typename T, typename... Args>
using EnableIfConstructible =
    typename std::enable_if<IsConstructible<T, Args...>::value, R>::type;
// Enable if T(Args...) is not well formed.
template <typename R, typename T, typename... Args>
using EnableIfNotConstructible =
    typename std::enable_if<!IsConstructible<T, Args...>::value, R>::type;

// Determines whether T is an element of Types...;
template <typename... Types>
struct HasType : std::false_type {};
template <typename T, typename U>
struct HasType<T, U> : std::is_same<std::decay_t<T>, std::decay_t<U>> {};
template <typename T, typename First, typename... Rest>
struct HasType<T, First, Rest...>
    : std::integral_constant<bool, HasType<T, First>::value ||
                                       HasType<T, Rest...>::value> {};

// Defines set operations on a set of Types...
template <typename... Types>
struct Set {
  // Default specialization catches the empty set, which is always a subset.
  template <typename...>
  struct IsSubset : std::true_type {};
  template <typename T>
  struct IsSubset<T> : HasType<T, Types...> {};
  template <typename First, typename... Rest>
  struct IsSubset<First, Rest...>
      : std::integral_constant<bool, IsSubset<First>::value &&
                                         IsSubset<Rest...>::value> {};
};

// Determines the number of elements of Types... that are constructible from
// From.
template <typename... Types>
struct ConstructibleCount;
template <typename From, typename To>
struct ConstructibleCount<From, To>
    : std::integral_constant<std::size_t, IsConstructible<To, From>::value> {};
template <typename From, typename First, typename... Rest>
struct ConstructibleCount<From, First, Rest...>
    : std::integral_constant<std::size_t,
                             IsConstructible<First, From>::value +
                                 ConstructibleCount<From, Rest...>::value> {};

// Enable if T is an element of Types...
template <typename R, typename T, typename... Types>
using EnableIfElement =
    typename std::enable_if<HasType<T, Types...>::value, R>::type;
// Enable if T is not an element of Types...
template <typename R, typename T, typename... Types>
using EnableIfNotElement =
    typename std::enable_if<!HasType<T, Types...>::value, R>::type;

// Enable if T is convertible to an element of Types... T is considered
// convertible IIF a single element of Types... is assignable from T and T is
// not a direct element of Types...
template <typename R, typename T, typename... Types>
using EnableIfConvertible =
    typename std::enable_if<!HasType<T, Types...>::value &&
                                ConstructibleCount<T, Types...>::value == 1,
                            R>::type;

// Enable if T is assignable to an element of Types... T is considered
// assignable IFF a single element of Types... is constructible from T or T is a
// direct element of Types.... Note that T is REQUIRED to be an element of
// Types... when multiple elements are constructible from T to prevent ambiguity
// in conversion.
template <typename R, typename T, typename... Types>
using EnableIfAssignable =
    typename std::enable_if<HasType<T, Types...>::value ||
                                ConstructibleCount<T, Types...>::value == 1,
                            R>::type;

// Selects a type for SFINAE constructor selection.
template <bool CondA, typename SelectA, typename SelectB>
using Select = std::conditional_t<CondA, SelectA, SelectB>;

// Recursive union type.
template <typename... Types>
union Union;

// Specialization handling a singular type, terminating template recursion.
template <typename Type>
union Union<Type> {
  Union() {}
  ~Union() {}

  template <typename T>
  Union(std::int32_t index, std::int32_t* index_out, TypeTag<Type>, T&& value)
      : first_(std::forward<T>(value)) {
    *index_out = index;
  }
  template <typename T, typename = EnableIfAssignable<void, T, Type>>
  Union(std::int32_t index, std::int32_t* index_out, T&& value)
      : first_(std::forward<T>(value)) {
    *index_out = index;
  }
  Union(const Union& other, std::int32_t index) {
    if (index == 0)
      new (&first_) Type(other.first_);
  }
  Union(Union&& other, std::int32_t index) {
    if (index == 0)
      new (&first_) Type(std::move(other.first_));
  }
  Union(const Union&) = delete;
  Union(Union&&) = delete;
  void operator=(const Union&) = delete;
  void operator=(Union&&) = delete;

  Type& get(TypeTag<Type>) { return first_; }
  const Type& get(TypeTag<Type>) const { return first_; }
  EmptyVariant get(TypeTag<EmptyVariant>) const { return {}; }
  constexpr std::int32_t index(TypeTag<Type>) const { return 0; }

  template <typename... Args>
  std::int32_t Construct(TypeTag<Type>, Args&&... args) {
    new (&first_) Type(std::forward<Args>(args)...);
    return 0;
  }
  template <typename... Args>
  EnableIfConstructible<std::int32_t, Type, Args...> Construct(Args&&... args) {
    new (&first_) Type(std::forward<Args>(args)...);
    return 0;
  }

  void Destruct(std::int32_t target_index) {
    if (target_index == index(TypeTag<Type>{})) {
      (&get(TypeTag<Type>{}))->~Type();
    }
  }

  template <typename T>
  bool Assign(TypeTag<Type>, std::int32_t target_index, T&& value) {
    if (target_index == 0) {
      first_ = std::forward<T>(value);
      return true;
    } else {
      return false;
    }
  }
  template <typename T>
  EnableIfConstructible<bool, Type, T> Assign(std::int32_t target_index,
                                              T&& value) {
    if (target_index == 0) {
      first_ = std::forward<T>(value);
      return true;
    } else {
      return false;
    }
  }
  template <typename T>
  EnableIfNotConstructible<bool, Type, T> Assign(std::int32_t /*target_index*/,
                                                 T&& /*value*/) {
    return false;
  }

  template <typename Op>
  decltype(auto) Visit(std::int32_t target_index, Op&& op) {
    if (target_index == index(TypeTag<Type>{}))
      return std::forward<Op>(op)(get(TypeTag<Type>{}));
    else
      return std::forward<Op>(op)(get(TypeTag<EmptyVariant>{}));
  }
  template <typename Op>
  decltype(auto) Visit(std::int32_t target_index, Op&& op) const {
    if (target_index == index(TypeTag<Type>{}))
      return std::forward<Op>(op)(get(TypeTag<Type>{}));
    else
      return std::forward<Op>(op)(get(TypeTag<EmptyVariant>{}));
  }

  template <typename... Args>
  bool Become(std::int32_t target_index, Args&&... args) {
    if (target_index == index(TypeTag<Type>{})) {
      Construct(TypeTag<Type>{}, std::forward<Args>(args)...);
      return true;
    } else {
      return false;
    }
  }

 private:
  Type first_;
};

// Specialization that recursively unions types from the paramater pack.
template <typename First, typename... Rest>
union Union<First, Rest...> {
  Union() {}
  ~Union() {}

  template <typename T>
  Union(std::int32_t index, std::int32_t* index_out, TypeTag<First>, T&& value)
      : first_(std::forward<T>(value)) {
    *index_out = index;
  }
  template <typename T, typename U>
  Union(std::int32_t index, std::int32_t* index_out, TypeTag<T>, U&& value)
      : rest_(index + 1, index_out, TypeTag<T>{}, std::forward<U>(value)) {}
  Union(const Union& other, std::int32_t index) {
    if (index == 0)
      new (&first_) First(other.first_);
    else
      new (&rest_) Union<Rest...>(other.rest_, index - 1);
  }
  Union(Union&& other, std::int32_t index) {
    if (index == 0)
      new (&first_) First(std::move(other.first_));
    else
      new (&rest_) Union<Rest...>(std::move(other.rest_), index - 1);
  }
  Union(const Union&) = delete;
  Union(Union&&) = delete;
  void operator=(const Union&) = delete;
  void operator=(Union&&) = delete;

  struct FirstType {};
  struct RestType {};
  template <typename T>
  using SelectConstructor =
      Select<ConstructibleCount<T, First>::value == 1, FirstType, RestType>;

  template <typename T>
  Union(std::int32_t index, std::int32_t* index_out, T&& value)
      : Union(index, index_out, std::forward<T>(value),
              SelectConstructor<T>{}) {}

  template <typename T>
  Union(std::int32_t index, std::int32_t* index_out, T&& value, FirstType)
      : first_(std::forward<T>(value)) {
    *index_out = index;
  }
  template <typename T>
  Union(std::int32_t index, std::int32_t* index_out, T&& value, RestType)
      : rest_(index + 1, index_out, std::forward<T>(value)) {}

  First& get(TypeTag<First>) { return first_; }
  const First& get(TypeTag<First>) const { return first_; }
  constexpr std::int32_t index(TypeTag<First>) const { return 0; }

  template <typename T>
  T& get(TypeTag<T>) {
    return rest_.get(TypeTag<T>{});
  }
  template <typename T>
  const T& get(TypeTag<T>) const {
    return rest_.template get(TypeTag<T>{});
  }
  template <typename T>
  constexpr std::int32_t index(TypeTag<T>) const {
    return 1 + rest_.index(TypeTag<T>{});
  }

  template <typename... Args>
  std::int32_t Construct(TypeTag<First>, Args&&... args) {
    new (&first_) First(std::forward<Args>(args)...);
    return 0;
  }
  template <typename T, typename... Args>
  std::int32_t Construct(TypeTag<T>, Args&&... args) {
    return 1 +
           rest_.template Construct(TypeTag<T>{}, std::forward<Args>(args)...);
  }

  template <typename... Args>
  EnableIfConstructible<std::int32_t, First, Args...> Construct(
      Args&&... args) {
    new (&first_) First(std::forward<Args>(args)...);
    return 0;
  }
  template <typename... Args>
  EnableIfNotConstructible<std::int32_t, First, Args...> Construct(
      Args&&... args) {
    return 1 + rest_.template Construct(std::forward<Args>(args)...);
  }

  void Destruct(std::int32_t target_index) {
    if (target_index == index(TypeTag<First>{})) {
      (get(TypeTag<First>{})).~First();
    } else {
      rest_.Destruct(target_index - 1);
    }
  }

  template <typename T>
  bool Assign(TypeTag<First>, std::int32_t target_index, T&& value) {
    if (target_index == 0) {
      first_ = std::forward<T>(value);
      return true;
    } else {
      return false;
    }
  }
  template <typename T, typename U>
  bool Assign(TypeTag<T>, std::int32_t target_index, U&& value) {
    return rest_.Assign(TypeTag<T>{}, target_index - 1, std::forward<U>(value));
  }
  template <typename T>
  EnableIfConstructible<bool, First, T> Assign(std::int32_t target_index,
                                               T&& value) {
    if (target_index == 0) {
      first_ = std::forward<T>(value);
      return true;
    } else {
      return rest_.Assign(target_index - 1, std::forward<T>(value));
    }
  }
  template <typename T>
  EnableIfNotConstructible<bool, First, T> Assign(std::int32_t target_index,
                                                  T&& value) {
    return rest_.Assign(target_index - 1, std::forward<T>(value));
  }

  // Recursively traverses the union and calls Op on the active value when the
  // active type is found. If the union is empty Op is called on EmptyVariant.
  // TODO(eieio): This could be refactored into an array or jump table. It's
  // unclear whether this would be more efficient for practical variant arity.
  template <typename Op>
  decltype(auto) Visit(std::int32_t target_index, Op&& op) {
    if (target_index == index(TypeTag<First>{}))
      return std::forward<Op>(op)(get(TypeTag<First>{}));
    else
      return rest_.Visit(target_index - 1, std::forward<Op>(op));
  }
  template <typename Op>
  decltype(auto) Visit(std::int32_t target_index, Op&& op) const {
    if (target_index == index(TypeTag<First>{}))
      return std::forward<Op>(op)(get(TypeTag<First>{}));
    else
      return rest_.Visit(target_index - 1, std::forward<Op>(op));
  }

  template <typename... Args>
  bool Become(std::int32_t target_index, Args&&... args) {
    if (target_index == index(TypeTag<First>{})) {
      Construct(TypeTag<First>{}, std::forward<Args>(args)...);
      return true;
    } else {
      return rest_.Become(target_index - 1, std::forward<Args>(args)...);
    }
  }

 private:
  First first_;
  Union<Rest...> rest_;
};

}  // namespace detail

// Variant is a type safe union that can store values of any of its element
// types. A Variant is different than std::tuple in that it only stores one type
// at a time or a special empty type. Variants are always default constructible
// to empty, even when none of the element types are default constructible.
template <typename... Types>
class Variant {
 private:
  // Convenience types.
  template <typename T>
  using TypeTag = detail::TypeTag<T>;
  template <typename T>
  using DecayedTypeTag = TypeTag<std::decay_t<T>>;
  template <std::size_t I>
  using TypeForIndex = detail::TypeForIndex<I, Types...>;
  template <std::size_t I>
  using TypeTagForIndex = detail::TypeTagForIndex<I, Types...>;
  template <typename T>
  using HasType = detail::HasType<T, Types...>;
  template <typename R, typename T>
  using EnableIfElement = detail::EnableIfElement<R, T, Types...>;
  template <typename R, typename T>
  using EnableIfConvertible = detail::EnableIfConvertible<R, T, Types...>;
  template <typename R, typename T>
  using EnableIfAssignable = detail::EnableIfAssignable<R, T, Types...>;

  struct Direct {};
  struct Convert {};
  template <typename T>
  using SelectConstructor = detail::Select<HasType<T>::value, Direct, Convert>;

  // Constructs by type tag when T is an direct element of Types...
  template <typename T>
  explicit Variant(T&& value, Direct)
      : value_(0, &index_, DecayedTypeTag<T>{}, std::forward<T>(value)) {}
  // Conversion constructor when T is not a direct element of Types...
  template <typename T>
  explicit Variant(T&& value, Convert)
      : value_(0, &index_, std::forward<T>(value)) {}

 public:
  // Variants are default construcible, regardless of whether the elements are
  // default constructible. Default consruction yields an empty Variant.
  Variant() {}
  explicit Variant(EmptyVariant) {}
  ~Variant() { Destruct(); }

  Variant(const Variant& other)
      : index_{other.index_}, value_{other.value_, other.index_} {}
  Variant(Variant&& other) noexcept
      : index_{other.index_}, value_{std::move(other.value_), other.index_} {}

// Recent Clang versions has a regression that produces bogus
// unused-lambda-capture warning. Suppress the warning as a temporary
// workaround. http://b/71356631
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-lambda-capture"
  // Copy and move construction from Variant types. Each element of OtherTypes
  // must be convertible to an element of Types.
  template <typename... OtherTypes>
  explicit Variant(const Variant<OtherTypes...>& other) {
    other.Visit([this](const auto& value) { Construct(value); });
  }
#pragma clang diagnostic pop

  template <typename... OtherTypes>
  explicit Variant(Variant<OtherTypes...>&& other) {
    other.Visit([this](auto&& value) { Construct(std::move(value)); });
  }

  Variant& operator=(const Variant& other) {
    other.Visit([this](const auto& value) { *this = value; });
    return *this;
  }
  Variant& operator=(Variant&& other) noexcept {
    other.Visit([this](auto&& value) { *this = std::move(value); });
    return *this;
  }

  // Construction from non-Variant types.
  template <typename T, typename = EnableIfAssignable<void, T>>
  explicit Variant(T&& value)
      : Variant(std::forward<T>(value), SelectConstructor<T>{}) {}

  // Performs assignment from type T belonging to Types. This overload takes
  // priority to prevent implicit conversion in cases where T is implicitly
  // convertible to multiple elements of Types.
  template <typename T>
  EnableIfElement<Variant&, T> operator=(T&& value) {
    Assign(DecayedTypeTag<T>{}, std::forward<T>(value));
    return *this;
  }

  // Performs assignment from type T not belonging to Types. This overload
  // matches in cases where conversion is the only viable option.
  template <typename T>
  EnableIfConvertible<Variant&, T> operator=(T&& value) {
    Assign(std::forward<T>(value));
    return *this;
  }

  // Handles assignment from the empty type. This overload supports assignment
  // in visitors using generic lambdas.
  Variant& operator=(EmptyVariant) {
    Destruct();
    return *this;
  }

  // Assignment from Variant types. Each element of OtherTypes must be
  // convertible to an element of Types. Forwards through non-Variant assignment
  // operators to apply conversion checks.
  template <typename... OtherTypes>
  Variant& operator=(const Variant<OtherTypes...>& other) {
    other.Visit([this](const auto& value) { *this = value; });
    return *this;
  }
  template <typename... OtherTypes>
  Variant& operator=(Variant<OtherTypes...>&& other) {
    other.Visit([this](auto&& value) { *this = std::move(value); });
    return *this;
  }

  // Becomes the target type, constructing a new element from the given
  // arguments if necessary. No action is taken if the active element is already
  // the target type. Otherwise the active element is destroyed and replaced by
  // constructing an element of the new type using |Args|. An invalid target
  // type index results in an empty Variant.
  template <typename... Args>
  void Become(std::int32_t target_index, Args&&... args) {
    if (target_index != index()) {
      Destruct();
      index_ = value_.Become(target_index, std::forward<Args>(args)...)
                   ? target_index
                   : kEmptyIndex;
    }
  }

  // Invokes |Op| on the active element. If the Variant is empty |Op| is invoked
  // on EmptyVariant.
  template <typename Op>
  decltype(auto) Visit(Op&& op) {
    return value_.Visit(index_, std::forward<Op>(op));
  }
  template <typename Op>
  decltype(auto) Visit(Op&& op) const {
    return value_.Visit(index_, std::forward<Op>(op));
  }

  // Index returned when the Variant is empty.
  enum : std::int32_t { kEmptyIndex = -1 };

  // Returns the index of the given type.
  template <typename T>
  constexpr std::int32_t index_of() const {
    static_assert(HasType<T>::value, "T is not an element type of Variant.");
    return value_.index(DecayedTypeTag<T>{});
  }

  // Returns the index of the active type. If the Variant is empty -1 is
  // returned.
  std::int32_t index() const { return index_; }

  // Returns true if the given type is active, false otherwise.
  template <typename T>
  bool is() const {
    static_assert(HasType<T>::value, "T is not an element type of Variant.");
    return index() == index_of<T>();
  }

  // Returns true if the Variant is empty, false otherwise.
  bool empty() const { return index() == kEmptyIndex; }

  // Element accessors. Returns a pointer to the active value if the given
  // type/index is active, otherwise nullptr is returned.
  template <typename T>
  T* get() {
    if (is<T>())
      return &value_.get(DecayedTypeTag<T>{});
    else
      return nullptr;
  }
  template <typename T>
  const T* get() const {
    if (is<T>())
      return &value_.template get(DecayedTypeTag<T>{});
    else
      return nullptr;
  }
  template <std::size_t I>
  TypeForIndex<I>* get() {
    if (is<TypeForIndex<I>>())
      return &value_.get(TypeTagForIndex<I>{});
    else
      return nullptr;
  }
  template <std::size_t I>
  const TypeForIndex<I>* get() const {
    if (is<TypeForIndex<I>>())
      return &value_.template get(TypeTagForIndex<I>{});
    else
      return nullptr;
  }

 private:
  std::int32_t index_ = kEmptyIndex;
  detail::Union<std::decay_t<Types>...> value_;

  // Constructs an element from the given arguments and sets the Variant to the
  // resulting type.
  template <typename... Args>
  void Construct(Args&&... args) {
    index_ = value_.template Construct(std::forward<Args>(args)...);
  }
  void Construct(EmptyVariant) {}

  // Destroys the active element of the Variant.
  void Destruct() {
    value_.Destruct(index_);
    index_ = kEmptyIndex;
  }

  // Assigns the Variant when non-empty and the current type matches the target
  // type, otherwise destroys the current value and constructs a element of the
  // new type. Tagged assignment is used when T is an element of the Variant to
  // prevent implicit conversion in cases where T is implicitly convertible to
  // multiple element types.
  template <typename T, typename U>
  void Assign(TypeTag<T>, U&& value) {
    if (!value_.template Assign(TypeTag<T>{}, index_, std::forward<U>(value))) {
      Destruct();
      Construct(TypeTag<T>{}, std::forward<U>(value));
    }
  }
  template <typename T>
  void Assign(T&& value) {
    if (!value_.template Assign(index_, std::forward<T>(value))) {
      Destruct();
      Construct(std::forward<T>(value));
    }
  }
};

// Utility type to extract/convert values from a variant. This class simplifies
// conditional logic to get/move/swap/action values from a variant when one or
// more elements are compatible with the destination type.
//
// Example:
//    Variant<int, bool, std::string> v(10);
//    bool bool_value;
//    if (IfAnyOf<int, bool>::Get(v, &bool_value)) {
//      DoSomething(bool_value);
//    } else {
//      HandleInvalidType();
//    }
//    IfAnyOf<int>::Call(v, [](const auto& value) { DoSomething(value); });
//
template <typename... ValidTypes>
struct IfAnyOf {
  // Calls Op on the underlying value of the variant and returns true when the
  // variant is a valid type, otherwise does nothing and returns false.
  template <typename Op, typename... Types>
  static bool Call(Variant<Types...>* variant, Op&& op) {
    static_assert(
        detail::Set<Types...>::template IsSubset<ValidTypes...>::value,
        "ValidTypes may only contain element types from the Variant.");
    return variant->Visit(CallOp<Op>{std::forward<Op>(op)});
  }
  template <typename Op, typename... Types>
  static bool Call(const Variant<Types...>* variant, Op&& op) {
    static_assert(
        detail::Set<Types...>::template IsSubset<ValidTypes...>::value,
        "ValidTypes may only contain element types from the Variant.");
    return variant->Visit(CallOp<Op>{std::forward<Op>(op)});
  }

  // Gets/converts the underlying value of the variant to type T and returns
  // true when the variant is a valid type, otherwise does nothing and returns
  // false.
  template <typename T, typename... Types>
  static bool Get(const Variant<Types...>* variant, T* value_out) {
    return Call(variant,
                [value_out](const auto& value) { *value_out = value; });
  }

  // Moves the underlying value of the variant and returns true when the variant
  // is a valid type, otherwise does nothing and returns false.
  template <typename T, typename... Types>
  static bool Take(Variant<Types...>* variant, T* value_out) {
    return Call(variant,
                [value_out](auto&& value) { *value_out = std::move(value); });
  }

  // Swaps the underlying value of the variant with |*value_out| and returns
  // true when the variant is a valid type, otherwise does nothing and returns
  // false.
  template <typename T, typename... Types>
  static bool Swap(Variant<Types...>* variant, T* value_out) {
    return Call(variant,
                [value_out](auto&& value) { std::swap(*value_out, value); });
  }

 private:
  template <typename Op>
  struct CallOp {
    Op&& op;
    template <typename U>
    detail::EnableIfNotElement<bool, U, ValidTypes...> operator()(U&&) {
      return false;
    }
    template <typename U>
    detail::EnableIfElement<bool, U, ValidTypes...> operator()(const U& value) {
      std::forward<Op>(op)(value);
      return true;
    }
    template <typename U>
    detail::EnableIfElement<bool, U, ValidTypes...> operator()(U&& value) {
      std::forward<Op>(op)(std::forward<U>(value));
      return true;
    }
  };
};

}  // namespace rpc
}  // namespace pdx
}  // namespace android

// Overloads of std::get<T> and std::get<I> for android::pdx::rpc::Variant.
namespace std {

template <typename T, typename... Types>
inline T& get(::android::pdx::rpc::Variant<Types...>& v) {
  return *v.template get<T>();
}
template <typename T, typename... Types>
inline T&& get(::android::pdx::rpc::Variant<Types...>&& v) {
  return std::move(*v.template get<T>());
}
template <typename T, typename... Types>
inline const T& get(const ::android::pdx::rpc::Variant<Types...>& v) {
  return *v.template get<T>();
}
template <std::size_t I, typename... Types>
inline ::android::pdx::rpc::detail::TypeForIndex<I, Types...>& get(
    ::android::pdx::rpc::Variant<Types...>& v) {
  return *v.template get<I>();
}
template <std::size_t I, typename... Types>
inline ::android::pdx::rpc::detail::TypeForIndex<I, Types...>&& get(
    ::android::pdx::rpc::Variant<Types...>&& v) {
  return std::move(*v.template get<I>());
}
template <std::size_t I, typename... Types>
inline const ::android::pdx::rpc::detail::TypeForIndex<I, Types...>& get(
    const ::android::pdx::rpc::Variant<Types...>& v) {
  return *v.template get<I>();
}

}  // namespace std

#endif  // ANDROID_PDX_RPC_VARIANT_H_
