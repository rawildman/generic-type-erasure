#ifndef GENERIC_TYPE_ERASURE_HPP
#define GENERIC_TYPE_ERASURE_HPP

#include <any>
#include <cassert>
#include <tuple>
#include <type_traits>
#include <variant>

#include "generic-type-erasure-impl.hpp"
#include "type-helpers.hpp"
#include "type-map.hpp"

namespace gte {
template <typename TagType, typename SignatureType>
struct MemberSignature {
  using Tag = TagType;
  using Signature = SignatureType;
  static constexpr bool is_const = false;
};

template <typename TagType, typename SignatureType>
struct ConstMemberSignature {
  using Tag = TagType;
  using Signature = SignatureType;
  static constexpr bool is_const = true;
};

template <typename... MemberSignatureTypes>
class TypeErased {
 public:
  template <typename T, typename... MemberFunctions,
            std::enable_if_t<!std::is_same_v<std::decay_t<T>, TypeErased>,
                             bool> = true>
  TypeErased(T &&t, const MemberFunctions &...member_functions)
      : m_wrapped_member_functions{detail::member_function<
            T, MemberFunctions, typename MemberSignatureTypes::Signature>()...},
        m_object_member_functions{
            std::make_any<MemberFunctions>(member_functions)...},
        m_object{std::forward<T>(t)} {
    (detail::enforce_constness<MemberFunctions, MemberSignatureTypes>(), ...);
  }

  template <typename CallTag, typename... Args>
  auto call(Args &&...args) const {
    constexpr auto is_const =
        m_member_function_is_const.template get<CallTag>();
    static_assert(is_const,
                  "Attempted call of a non-const member "
                  "function with a const object.");

    const auto &wrapped_member =
        m_wrapped_member_functions.template get<CallTag>();
    const auto &object_member =
        m_object_member_functions.template get<CallTag>();
    const auto &function =
        std::get<detail::const_member_function_wrapper_index>(wrapped_member);
    return (*function)(m_object, object_member,
                       std::forward_as_tuple(std::forward<Args>(args)...));
  }

  template <typename CallTag, typename... Args>
  auto call(Args &&...args) {
    const auto &wrapped_member =
        m_wrapped_member_functions.template get<CallTag>();
    return std::visit(
        [&args..., this](const auto &function) {
          const auto &object_member =
              m_object_member_functions.template get<CallTag>();
          return (*function)(
              m_object, object_member,
              std::forward_as_tuple(std::forward<Args>(args)...));
        },
        wrapped_member);
  }

 private:
  using WrappedMemberTypeMap =
      typename detail::TagMemberFunctionMap<MemberSignatureTypes...>::Map;
  using MemberFunctionTypeMap =
      typename detail::TagValueMap<std::any, MemberSignatureTypes...>::Map;

  static constexpr auto m_member_function_is_const =
      detail::const_map<MemberSignatureTypes...>();

  WrappedMemberTypeMap m_wrapped_member_functions;
  MemberFunctionTypeMap m_object_member_functions;

  std::any m_object;
};
}  // namespace gte

#endif
