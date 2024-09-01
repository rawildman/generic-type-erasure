#include <any>
#include <tuple>
#include <type_traits>
#include <variant>

#include "type-helpers.hpp"
#include "type-map.hpp"

namespace gte {
namespace detail {
template <typename T, typename MemberFunction, typename Signature>
[[nodiscard]] auto member_function() {
  using MemberSignature = MemberFunctionSignatureHelper<MemberFunction>;
  using ArgTypes = typename detail::SignatureHelper<Signature>::ArgTypes;
  using BaseType = std::decay_t<T>;

  static_assert(
      std::is_same_v<BaseType, typename MemberSignature::Name>,
      "The object type does not match the member function's object type.");

  if constexpr (MemberSignature::is_const) {
    return [](const std::any &object, const std::any &pointer_to_member,
              ArgTypes &&args) {
      const auto any_as_t =
          std::tuple<const BaseType &>{std::any_cast<const BaseType &>(object)};
      const auto any_as_mem_fun =
          std::any_cast<MemberFunction>(pointer_to_member);
      return std::apply(any_as_mem_fun,
                        std::tuple_cat(any_as_t, std::move(args)));
    };
  } else {
    return [](std::any &object, const std::any &pointer_to_member,
              ArgTypes &&args) {
      auto any_as_t = std::tuple<BaseType &>{std::any_cast<BaseType &>(object)};
      const auto any_as_mem_fun =
          std::any_cast<MemberFunction>(pointer_to_member);
      return std::apply(any_as_mem_fun,
                        std::tuple_cat(any_as_t, std::move(args)));
    };
  }
}

constexpr auto const_member_function_wrapper_index = std::size_t{0};

template <typename TagAndSignatureType>
struct MemberFunctionHelper {
  using Signature = typename TagAndSignatureType::Signature;
  using Tag = typename TagAndSignatureType::Tag;

  using WrappedMemberFunctionSignature =
      typename detail::SignatureWithExtraArgs<Signature, std::any &,
                                              const std::any &>::Signature;
  using WrappedConstMemberFunctionSignature =
      typename detail::SignatureWithExtraArgs<Signature, const std::any &,
                                              const std::any &>::Signature;
  using WrappedMemberFunctionPtr =
      std::add_pointer_t<WrappedMemberFunctionSignature>;
  using WrappedConstMemberFunctionPtr =
      std::add_pointer_t<WrappedConstMemberFunctionSignature>;

  using WrappedMemberFunctionVariant =
      std::variant<WrappedConstMemberFunctionPtr, WrappedMemberFunctionPtr>;
};

template <typename Value, typename... TagAndSignatureTypes>
struct TagValueMap {
  using Map = TypeMap<std::pair<
      typename MemberFunctionHelper<TagAndSignatureTypes>::Tag, Value>...>;
};

template <typename... TagAndSignatureTypes>
struct TagMemberFunctionMap {
  using Map = TypeMap<
      std::pair<typename MemberFunctionHelper<TagAndSignatureTypes>::Tag,
                typename MemberFunctionHelper<
                    TagAndSignatureTypes>::WrappedMemberFunctionVariant>...>;
};

template <typename... TagAndSignatureTypes>
[[nodiscard]] constexpr auto const_map() {
  using ConstMap = typename TagValueMap<bool, TagAndSignatureTypes...>::Map;
  return ConstMap{TagAndSignatureTypes::is_const...};
}

template <typename MemberFunction, typename TagAndSignatureType>
[[nodiscard]] constexpr auto constness_matches() -> bool {
  using MemberSignature = MemberFunctionSignatureHelper<MemberFunction>;
  return !TagAndSignatureType::is_const || MemberSignature::is_const;
}

template <typename MemberFunction, typename TagAndSignatureType>
constexpr void enforce_constness() {
  static_assert(constness_matches<MemberFunction, TagAndSignatureType>(),
                "const TypeErased member functions must be constructed with "
                "const member functions.");
}
}  // namespace detail
}  // namespace gte
