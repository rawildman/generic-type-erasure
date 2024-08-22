#include "type-helpers.hpp"

#include <catch2/catch_test_macros.hpp>

struct Tester
{
  int answer = 42;
  int
  forty_two () const
  {
    return answer;
  }

  int
  set_forty_two (const int new_value)
  {
    const auto old_answer = answer;
    answer = new_value;
    return old_answer;
  }
};

struct FortyTwo
{
};

TEST_CASE ("SignatureHelper static assertions", "[signaturehelper]")
{
  using TestIntVoid = generic::detail::SignatureHelper<int ()>;
  static_assert (std::is_same_v<int, typename TestIntVoid::ReturnType>);
  static_assert (std::is_same_v<std::tuple<>, typename TestIntVoid::ArgTypes>);

  using TestDoubleInt = generic::detail::SignatureHelper<double (int)>;
  static_assert (std::is_same_v<double, typename TestDoubleInt::ReturnType>);
  static_assert (
      std::is_same_v<std::tuple<int>, typename TestDoubleInt::ArgTypes>);

  using TestVoidIntDouble
      = generic::detail::SignatureHelper<void (int, double)>;
  static_assert (std::is_same_v<void, typename TestVoidIntDouble::ReturnType>);
  static_assert (std::is_same_v<std::tuple<int, double>,
                                typename TestVoidIntDouble::ArgTypes>);
}

TEST_CASE ("Wrapper member function signature helper static assertions",
           "[signaturehelper]")
{
  using TestWrapperMemFun
      = generic::detail::WrapperConstMemberFunctionSignature<int (double,
                                                                  int)>;
  using ReturnFromWrapper = typename generic::detail::SignatureHelper<
      typename TestWrapperMemFun::Signature>::ReturnType;
  static_assert (std::is_same_v<int, ReturnFromWrapper>);
  using ArgsFromWrapper = typename generic::detail::SignatureHelper<
      typename TestWrapperMemFun::Signature>::ArgTypes;
  static_assert (std::is_same_v<std::tuple<const std::any &, const std::any &,
                                           std::tuple<double, int> &&>,
                                ArgsFromWrapper>);
}

TEST_CASE ("Member function signature helper static assertions",
           "[signaturehelper]")
{
  using TestConst = generic::detail::MemberFunctionSignatureHelper<decltype (
      &Tester::forty_two)>;
  static_assert (std::is_same_v<typename TestConst::Name, Tester>);
  static_assert (std::is_same_v<typename TestConst::ReturnType, int>);
  static_assert (std::is_same_v<typename TestConst::ArgTypes, std::tuple<> >);
  static_assert (TestConst::is_const);

  using TestNonConst
      = generic::detail::MemberFunctionSignatureHelper<decltype (
          &Tester::set_forty_two)>;
  static_assert (std::is_same_v<typename TestNonConst::Name, Tester>);
  static_assert (std::is_same_v<typename TestNonConst::ReturnType, int>);
  static_assert (
      std::is_same_v<typename TestNonConst::ArgTypes, std::tuple<int> >);
  static_assert (!TestNonConst::is_const);
}
