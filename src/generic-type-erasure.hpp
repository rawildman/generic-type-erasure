#ifndef GENERIC_TYPE_ERASURE_HPP
#define GENERIC_TYPE_ERASURE_HPP

#include "type-helpers.hpp"

#include <any>
#include <iostream>
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
  using ReturnType = typename detail::SignatureHelper<Signature>::ReturnType;
  if constexpr (MemberSignature::is_const)
    {
      return [] (const std::any &object, const std::any &pointer_to_member,
                 ArgTypes &&args) {
        const auto any_as_t
            = std::tuple<const T &>{ std::any_cast<T> (object) };
        const auto &any_as_mem_fun
            = std::any_cast<MemberFunction> (pointer_to_member);
        return std::apply (any_as_mem_fun,
                           std::tuple_cat (any_as_t, std::move (args)));
      };
    }
  else
    {
      return [] (std::any &object, const std::any &pointer_to_member,
                 ArgTypes &&args) {
        auto any_as_t = std::tuple<T &>{ std::any_cast<T> (object) };
        const auto &any_as_mem_fun
            = std::any_cast<MemberFunction> (pointer_to_member);
        return std::apply (any_as_mem_fun,
                           std::tuple_cat (any_as_t, std::move (args)));
      };
    }
}

}

template <typename Tag, typename Signature> class TypeErased
{
private:
  using WrappedMemberFunctionSignature =
      typename detail::WrapperMemberFunctionSignature<Signature>::Signature;
  using WrappedConstMemberFunctionSignature =
      typename detail::WrapperConstMemberFunctionSignature<
          Signature>::Signature;
  using WrappedMemberFunctionPtr
      = std::add_pointer_t<WrappedMemberFunctionSignature>;
  using WrappedConstMemberFunctionPtr
      = std::add_pointer_t<WrappedConstMemberFunctionSignature>;

  using WrappedMemberFunctionVariant
      = std::variant<WrappedMemberFunctionPtr, WrappedConstMemberFunctionPtr>;

public:
  template <typename T, typename MemberFunction>
  TypeErased (T &&t, const MemberFunction &member_function)
      : mMemberFunctionVariant{ detail::member_function<T, MemberFunction,
                                                        Signature> () },
        mMemberFunction{ member_function }, mObject{ std::forward<T> (t) }
  {
  }

  template <typename CallTag, typename... Args>
  auto
  call (Args &&...args) const
  {
    static_assert (std::is_same_v<CallTag, Tag>);
    std::cout << "call() const\n";
    if (std::holds_alternative<WrappedConstMemberFunctionPtr> (
            mMemberFunctionVariant))
      {
        const auto const_function
            = std::get<WrappedConstMemberFunctionPtr> (mMemberFunctionVariant);
        return (*const_function) (
            mObject, mMemberFunction,
            std::forward_as_tuple (std::forward<Args> (args)...));
      }
    assert (false);
  }

  template <typename CallTag, typename... Args>
  auto
  call (Args &&...args)
  {
    static_assert (std::is_same_v<CallTag, Tag>);
    std::cout << "call()\n";
    if (std::holds_alternative<WrappedConstMemberFunctionPtr> (
            mMemberFunctionVariant))
      {
        const auto const_function
            = std::get<WrappedConstMemberFunctionPtr> (mMemberFunctionVariant);
        return (*const_function) (
            mObject, mMemberFunction,
            std::forward_as_tuple (std::forward<Args> (args)...));
      }

    const auto function
        = std::get<WrappedMemberFunctionPtr> (mMemberFunctionVariant);
    return (*function) (mObject, mMemberFunction,
                        std::forward_as_tuple (std::forward<Args> (args)...));
  }

private:
  WrappedMemberFunctionVariant mMemberFunctionVariant;

  std::any mMemberFunction;
  std::any mObject;
};
}

#endif
