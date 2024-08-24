#ifndef GENERIC_TYPE_ERASURE_HPP
#define GENERIC_TYPE_ERASURE_HPP

#include "type-helpers.hpp"
#include "type-map.hpp"

#include <any>
#include <tuple>
#include <type_traits>
#include <variant>

#include <cassert>

namespace gte
{
namespace detail
{
template <typename T, typename MemberFunction, typename Signature>
auto
member_function ()
{
  using MemberSignature = MemberFunctionSignatureHelper<MemberFunction>;
  using ArgTypes = typename detail::SignatureHelper<Signature>::ArgTypes;
  using BaseType = std::decay_t<T>;
  if constexpr (MemberSignature::is_const)
    {
      return [] (const std::any &object, const std::any &pointer_to_member,
                 ArgTypes &&args) {
        const auto any_as_t = std::tuple<const BaseType &>{
          std::any_cast<const BaseType &> (object)
        };
        const auto any_as_mem_fun
            = std::any_cast<MemberFunction> (pointer_to_member);
        return std::apply (any_as_mem_fun,
                           std::tuple_cat (any_as_t, std::move (args)));
      };
    }
  else
    {
      return [] (std::any &object, const std::any &pointer_to_member,
                 ArgTypes &&args) {
        auto any_as_t
            = std::tuple<BaseType &>{ std::any_cast<BaseType &> (object) };
        const auto any_as_mem_fun
            = std::any_cast<MemberFunction> (pointer_to_member);
        return std::apply (any_as_mem_fun,
                           std::tuple_cat (any_as_t, std::move (args)));
      };
    }
}

constexpr auto const_member_function_wrapper_index = std::size_t{ 0 };

template <typename TagAndSignatureType> struct MemberFunctionHelper
{
  using Signature = typename TagAndSignatureType::Signature;
  using Tag = typename TagAndSignatureType::Tag;

  using WrappedMemberFunctionSignature =
      typename detail::SignatureWithExtraArgs<Signature, std::any &,
                                              const std::any &>::Signature;
  using WrappedConstMemberFunctionSignature =
      typename detail::SignatureWithExtraArgs<Signature, const std::any &,
                                              const std::any &>::Signature;
  using WrappedMemberFunctionPtr
      = std::add_pointer_t<WrappedMemberFunctionSignature>;
  using WrappedConstMemberFunctionPtr
      = std::add_pointer_t<WrappedConstMemberFunctionSignature>;

  using WrappedMemberFunctionVariant
      = std::variant<WrappedConstMemberFunctionPtr, WrappedMemberFunctionPtr>;
};

template <typename Value, typename... TagAndSignatureTypes> struct TagValueMap
{
  using Map = TypeMap<std::pair<
      typename MemberFunctionHelper<TagAndSignatureTypes>::Tag, Value>...>;
};

template <typename... TagAndSignatureTypes> struct TagMemberFunctionMap
{
  using Map = TypeMap<
      std::pair<typename MemberFunctionHelper<TagAndSignatureTypes>::Tag,
                typename MemberFunctionHelper<
                    TagAndSignatureTypes>::WrappedMemberFunctionVariant>...>;
};
}

template <typename TagType, typename SignatureType> struct TagAndSignature
{
  using Tag = TagType;
  using Signature = SignatureType;
};

template <typename... TagAndSignatureTypes> class TypeErased
{
public:
  template <typename T, typename... MemberFunctions>
  TypeErased (T &&t, const MemberFunctions &...member_functions)
      : m_wrapped_member_functions{ detail::member_function<
          T, MemberFunctions,
          typename TagAndSignatureTypes::Signature> ()... },
        m_object_member_functions{ std::make_any<MemberFunctions> (
            member_functions)... },
        m_object{ std::forward<T> (t) }
  {
  }

  template <typename CallTag, typename... Args>
  auto
  call (Args &&...args) const
  {
    const auto &wrapped_member
        = m_wrapped_member_functions.template get<CallTag> ();

    assert (wrapped_member.index ()
            == detail::const_member_function_wrapper_index);

    const auto &object_member
        = m_object_member_functions.template get<CallTag> ();
    const auto &function
        = std::get<detail::const_member_function_wrapper_index> (
            wrapped_member);
    return (*function) (m_object, object_member,
                        std::forward_as_tuple (std::forward<Args> (args)...));
  }

  template <typename CallTag, typename... Args>
  auto
  call (Args &&...args)
  {
    const auto &wrapped_member
        = m_wrapped_member_functions.template get<CallTag> ();
    return std::visit (
        [&args..., this] (const auto &function) {
          const auto &object_member
              = m_object_member_functions.template get<CallTag> ();
          return (*function) (
              m_object, object_member,
              std::forward_as_tuple (std::forward<Args> (args)...));
        },
        wrapped_member);
  }

private:
  using WrappedMemberTypeMap =
      typename detail::TagMemberFunctionMap<TagAndSignatureTypes...>::Map;
  using MemberFunctionTypeMap =
      typename detail::TagValueMap<std::any, TagAndSignatureTypes...>::Map;

  WrappedMemberTypeMap m_wrapped_member_functions;
  MemberFunctionTypeMap m_object_member_functions;

  std::any m_object;
};
}

#endif
