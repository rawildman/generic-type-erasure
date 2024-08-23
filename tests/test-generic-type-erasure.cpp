#include "generic-type-erasure.hpp"

#include <catch2/catch_test_macros.hpp>
#include <iostream>

class CopyMoveCounter
{
public:
  CopyMoveCounter () = default;
  CopyMoveCounter (const CopyMoveCounter &copy)
      : m_copies{ copy.m_copies + 1 }, m_moves{ copy.m_moves }
  {
    std::cout << "CopyMoveCounter(const&)\n";
  }
  CopyMoveCounter (CopyMoveCounter &&copy)
      : m_copies{ copy.m_copies }, m_moves{ copy.m_moves + 1 }
  {
    std::cout << "CopyMoveCounter(&&)\n";
  }
  CopyMoveCounter &
  operator= (const CopyMoveCounter &copy)
  {
    std::cout << "operator=(const&)\n";
    if (this != &copy)
      {
        m_copies = copy.m_copies + 1;
        m_moves = copy.m_moves;
      }
    return *this;
  }
  CopyMoveCounter &
  operator= (CopyMoveCounter &&copy)
  {
    std::cout << "operator=(&&)\n";
    m_copies = copy.m_copies;
    m_moves = copy.m_moves + 1;
    return *this;
  }

  auto
  copies () const -> unsigned
  {
    return m_copies;
  }
  auto
  moves () const -> unsigned
  {
    return m_moves;
  }

private:
  unsigned m_copies = 0;
  unsigned m_moves = 0;
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
    return { arg.copies (), arg.moves () };
  }
  auto
  value_arg (CopyMoveCounter arg) const -> std::pair<unsigned, unsigned>
  {
    return { arg.copies (), arg.moves () };
  }
  auto
  r_value_ref_arg (CopyMoveCounter &&arg) const
      -> std::pair<unsigned, unsigned>
  {
    return { arg.copies (), arg.moves () };
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
      = gte::TypeErased<FortyTwo, int ()>{ t, &Tester::forty_two };
  REQUIRE (wrapper.call<FortyTwo> () == 42);
}

TEST_CASE ("Copy counts", "[wrapper]")
{
  auto copy_move_counter = CopyMoveCounter{};
  REQUIRE (copy_move_counter.copies () == 0);
  REQUIRE (copy_move_counter.moves () == 0);

  const auto &cm_counter_ref = copy_move_counter;
  REQUIRE (cm_counter_ref.copies () == 0);
  REQUIRE (cm_counter_ref.moves () == 0);

  const auto cm_counter_copy = copy_move_counter;
  REQUIRE (cm_counter_copy.copies () == 1);
  REQUIRE (cm_counter_copy.moves () == 0);

  const auto cm_counter_move = std::move (copy_move_counter);
  REQUIRE (cm_counter_move.copies () == 0);
  REQUIRE (cm_counter_move.moves () == 1);
}

TEST_CASE ("Wrapper copy moves", "[wrapper]")
{
  const auto t = Tester{};
  const auto wrapper_const_ref_arg
      = gte::TypeErased<CopyCounter, std::pair<unsigned, unsigned> (
                                         const CopyMoveCounter &)>{
          t, &Tester::const_ref_arg
        };
  auto counter = CopyMoveCounter{};
  REQUIRE (wrapper_const_ref_arg.call<CopyCounter> (counter).first == 0);
  REQUIRE (wrapper_const_ref_arg.call<CopyCounter> (counter).second == 0);

  const auto wrapper_value_arg
      = gte::TypeErased<CopyCounter,
                        std::pair<unsigned, unsigned> (CopyMoveCounter)>{
          t, &Tester::value_arg
        };
  REQUIRE (wrapper_value_arg.call<CopyCounter> (counter).first == 1);
  REQUIRE (wrapper_value_arg.call<CopyCounter> (counter).second == 2);

  const auto wrapper_r_value_ref_arg
      = gte::TypeErased<CopyCounter,
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
      = gte::TypeErased<FortyTwo, int (int)>{ t, &Tester::set_forty_two };

  REQUIRE (wrapper.call<FortyTwo> (43) == 42);
  REQUIRE (wrapper.call<FortyTwo> (44) == 43);
}
