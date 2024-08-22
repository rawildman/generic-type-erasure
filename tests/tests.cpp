#include "generic-type-erasure.hpp"

#include <catch2/catch_test_macros.hpp>
#include <iostream>

struct CopyMoveCounter
{
  unsigned copies = 0;
  unsigned moves = 0;

  CopyMoveCounter () = default;
  CopyMoveCounter (const CopyMoveCounter &copy)
      : copies{ copy.copies + 1 }, moves{ copy.moves }
  {
    std::cout << "CopyMoveCounter(const&)\n";
  }
  CopyMoveCounter (CopyMoveCounter &&copy)
      : copies{ copy.copies }, moves{ copy.moves + 1 }
  {
    std::cout << "CopyMoveCounter(&&)\n";
  }
  CopyMoveCounter &
  operator= (const CopyMoveCounter &copy)
  {
    std::cout << "operator=(const&)\n";
    if (this != &copy)
      {
        copies = copy.copies + 1;
        moves = copy.moves;
      }
    return *this;
  }
  CopyMoveCounter &
  operator= (CopyMoveCounter &&copy)
  {
    std::cout << "operator=(&&)\n";
    copies = copy.copies;
    moves = copy.moves + 1;
    return *this;
  }
};

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

  auto
  const_ref_arg (const CopyMoveCounter &arg) const
      -> std::pair<unsigned, unsigned>
  {
    return { arg.copies, arg.moves };
  }
  auto
  value_arg (CopyMoveCounter arg) const -> std::pair<unsigned, unsigned>
  {
    return { arg.copies, arg.moves };
  }
  auto
  r_value_ref_arg (CopyMoveCounter &&arg) const
      -> std::pair<unsigned, unsigned>
  {
    return { arg.copies, arg.moves };
  }
};

struct FortyTwo
{
};
struct FortyThree
{
};
struct CopyCounter
{
};

TEST_CASE ("Wrapper", "[wrapper]")
{
  const auto t = Tester{};
  const auto wrapper
      = generic::TypeErased<FortyTwo, int ()>{ t, &Tester::forty_two };
  REQUIRE (wrapper.call<FortyTwo> () == 42);
}

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

TEST_CASE ("Copy counts", "[wrapper]")
{
  auto copy_move_counter = CopyMoveCounter{};
  REQUIRE (copy_move_counter.copies == 0);
  REQUIRE (copy_move_counter.moves == 0);

  const auto &cm_counter_ref = copy_move_counter;
  REQUIRE (cm_counter_ref.copies == 0);
  REQUIRE (cm_counter_ref.moves == 0);

  const auto cm_counter_copy = copy_move_counter;
  REQUIRE (cm_counter_copy.copies == 1);
  REQUIRE (cm_counter_copy.moves == 0);

  const auto cm_counter_move = std::move (copy_move_counter);
  REQUIRE (cm_counter_move.copies == 0);
  REQUIRE (cm_counter_move.moves == 1);
}

TEST_CASE ("Wrapper copy moves", "[wrapper]")
{
  const auto t = Tester{};
  const auto wrapper_const_ref_arg
      = generic::TypeErased<CopyCounter, std::pair<unsigned, unsigned> (
                                             const CopyMoveCounter &)>{
          t, &Tester::const_ref_arg
        };
  auto counter = CopyMoveCounter{};
  REQUIRE (wrapper_const_ref_arg.call<CopyCounter> (counter).first == 0);
  REQUIRE (wrapper_const_ref_arg.call<CopyCounter> (counter).second == 0);

  const auto wrapper_value_arg
      = generic::TypeErased<CopyCounter,
                            std::pair<unsigned, unsigned> (CopyMoveCounter)>{
          t, &Tester::value_arg
        };
  REQUIRE (wrapper_value_arg.call<CopyCounter> (counter).first == 1);
  REQUIRE (wrapper_value_arg.call<CopyCounter> (counter).second == 2);

  const auto wrapper_r_value_ref_arg
      = generic::TypeErased<CopyCounter,
                            std::pair<unsigned, unsigned> (CopyMoveCounter)>{
          t, &Tester::r_value_ref_arg
        };
  auto counter_for_r_values_1 = CopyMoveCounter{};
  REQUIRE (wrapper_r_value_ref_arg
               .call<CopyCounter> (std::move (counter_for_r_values_1))
               .first
           == 0);
  auto counter_for_r_values_2 = CopyMoveCounter{};
  REQUIRE (wrapper_r_value_ref_arg
               .call<CopyCounter> (std::move (counter_for_r_values_2))
               .second
           == 2);
}

TEST_CASE ("Non-const ref", "[wrapper]")
{
  auto t = Tester{};
  auto wrapper
      = generic::TypeErased<FortyTwo, int (int)>{ t, &Tester::set_forty_two };

  REQUIRE (wrapper.call<FortyTwo> (43) == 42);
  REQUIRE (wrapper.call<FortyTwo> (44) == 43);
}
