#ifndef TYPE_HELPERS_HPP
#define TYPE_HELPERS_HPP

#include <tuple>
#include <type_traits>

namespace gte::detail {
template <typename T>
struct SignatureHelper {};

template <typename R, typename... Args>
struct SignatureHelper<R(Args...)> {
  using ReturnType = R;
  using ArgTypes = std::tuple<Args...>;
};

template <typename BaseSignature, typename... ExtraArgs>
struct SignatureWithExtraArgs {
  using ReturnType = typename SignatureHelper<BaseSignature>::ReturnType;
  using ArgTuple = typename SignatureHelper<BaseSignature>::ArgTypes;
  using Signature = ReturnType(ExtraArgs..., ArgTuple &&);
};

template <typename T>
struct MemberFunctionSignatureHelper {};

template <typename StructName, typename R, typename... Args>
struct MemberFunctionSignatureHelper<R (StructName::*)(Args...)> {
  using Name = StructName;
  using ReturnType = R;
  using ArgTypes = std::tuple<Args...>;
  static constexpr auto is_const = false;
};

template <typename StructName, typename R, typename... Args>
struct MemberFunctionSignatureHelper<R (StructName::*)(Args...) const> {
  using Name = StructName;
  using ReturnType = R;
  using ArgTypes = std::tuple<Args...>;
  static constexpr auto is_const = true;
};

template <typename... Pairs>
struct PairsToTuples {
  using FirstTuple = std::tuple<typename Pairs::first_type...>;
  using SecondTuple = std::tuple<typename Pairs::second_type...>;
};

}  // namespace gte::detail

#endif
