#include <catch2/catch_test_macros.hpp>
#include <vector>

#include "generic-type-erasure.hpp"

namespace {
class CopyMoveCounter {
 public:
  CopyMoveCounter() = default;

  CopyMoveCounter(const CopyMoveCounter &copy)
      : m_copies{copy.m_copies + 1}, m_moves{copy.m_moves} {}

  CopyMoveCounter(CopyMoveCounter &&copy)
      : m_copies{copy.m_copies}, m_moves{copy.m_moves + 1} {}

  auto operator=(const CopyMoveCounter &copy) -> CopyMoveCounter & {
    if (this != &copy) {
      m_copies = copy.m_copies + 1;
      m_moves = copy.m_moves;
    }
    return *this;
  }

  auto operator=(CopyMoveCounter &&copy) -> CopyMoveCounter & {
    m_copies = copy.m_copies;
    m_moves = copy.m_moves + 1;
    return *this;
  }

  auto copies() const -> unsigned { return m_copies; }
  auto moves() const -> unsigned { return m_moves; }

 private:
  unsigned m_copies = 0;
  unsigned m_moves = 0;
};

struct Tester {
  int answer = 42;

  auto the_answer() const -> int { return answer; }

  auto multiply_the_answer(const int multiplier) const -> int {
    return multiplier * answer;
  }

  auto set_the_answer(const int new_value) -> int {
    const auto old_answer = answer;
    answer = new_value;
    return old_answer;
  }

  auto const_ref_arg(const CopyMoveCounter &arg) const
      -> std::pair<unsigned, unsigned> {
    return {arg.copies(), arg.moves()};
  }

  auto value_arg(CopyMoveCounter arg) const -> std::pair<unsigned, unsigned> {
    return {arg.copies(), arg.moves()};
  }

  auto r_value_ref_arg(CopyMoveCounter &&arg) const
      -> std::pair<unsigned, unsigned> {
    return {arg.copies(), arg.moves()};
  }
};

struct Tester2 {
  int answer = 42;

  auto the_answer_2() const -> int { return answer; }

  auto set_the_answer_2(const int new_value) -> int {
    const auto old_answer = answer;
    answer = new_value;
    return old_answer;
  }
};

struct TheAnswer {};
struct MultiplyTheAnswer {};
struct SetTheAnswer {};
struct CopyCounter {};
struct MoveCounter {};
struct Key1 {};
struct Key2 {};
}  // namespace

TEST_CASE("Wrapper", "[wrapper]") {
  using TheAnswerFunction = gte::TagAndConstSignature<TheAnswer, int()>;

  const auto t = Tester{};
  const auto wrapper =
      gte::TypeErased<TheAnswerFunction>{t, &Tester::the_answer};
  CHECK(wrapper.call<TheAnswer>() == 42);
}

TEST_CASE("Copy counts", "[wrapper]") {
  auto copy_move_counter = CopyMoveCounter{};
  CHECK(copy_move_counter.copies() == 0);
  CHECK(copy_move_counter.moves() == 0);

  const auto &cm_counter_ref = copy_move_counter;
  CHECK(cm_counter_ref.copies() == 0);
  CHECK(cm_counter_ref.moves() == 0);

  const auto cm_counter_copy = copy_move_counter;
  CHECK(cm_counter_copy.copies() == 1);
  CHECK(cm_counter_copy.moves() == 0);

  const auto cm_counter_move = std::move(copy_move_counter);
  CHECK(cm_counter_move.copies() == 0);
  CHECK(cm_counter_move.moves() == 1);
}

TEST_CASE("Wrapper copy moves", "[wrapper]") {
  const auto t = Tester{};
  SECTION("Const ref") {
    using CopyMoveFunction =
        gte::TagAndConstSignature<CopyCounter, std::pair<unsigned, unsigned>(
                                                   const CopyMoveCounter &)>;
    const auto wrapper_const_ref_arg =
        gte::TypeErased<CopyMoveFunction>{t, &Tester::const_ref_arg};
    auto counter = CopyMoveCounter{};
    CHECK(wrapper_const_ref_arg.call<CopyCounter>(counter).first == 0);
    CHECK(wrapper_const_ref_arg.call<CopyCounter>(counter).second == 0);
  }
  SECTION("Value") {
    using CopyMoveFunction =
        gte::TagAndConstSignature<CopyCounter, std::pair<unsigned, unsigned>(
                                                   CopyMoveCounter)>;
    const auto wrapper_value_arg =
        gte::TypeErased<CopyMoveFunction>{t, &Tester::value_arg};
    auto counter = CopyMoveCounter{};
    CHECK(wrapper_value_arg.call<CopyCounter>(counter).first == 1);
    CHECK(wrapper_value_arg.call<CopyCounter>(counter).second == 2);
  }
  SECTION("R-value ref") {
    using CopyMoveFunction =
        gte::TagAndConstSignature<CopyCounter, std::pair<unsigned, unsigned>(
                                                   CopyMoveCounter &&)>;

    const auto wrapper_r_value_ref_arg =
        gte::TypeErased<CopyMoveFunction>{t, &Tester::r_value_ref_arg};

    auto counter_for_r_values_1 = CopyMoveCounter{};
    CHECK(wrapper_r_value_ref_arg
              .call<CopyCounter>(std::move(counter_for_r_values_1))
              .first == 0);

    auto counter_for_r_values_2 = CopyMoveCounter{};
    CHECK(wrapper_r_value_ref_arg
              .call<CopyCounter>(std::move(counter_for_r_values_2))
              .second == 0);
  }
}

TEST_CASE("Non-const ref", "[wrapper]") {
  using SetTheAnswerFunction = gte::TagAndSignature<TheAnswer, int(int)>;
  const auto t = Tester{};
  auto wrapper =
      gte::TypeErased<SetTheAnswerFunction>{t, &Tester::set_the_answer};

  CHECK(wrapper.call<TheAnswer>(43) == 42);
  CHECK(wrapper.call<TheAnswer>(44) == 43);
}

TEST_CASE("Copies and moves on construction", "[wrapper]") {
  using CopyCounterFunction =
      gte::TagAndConstSignature<CopyCounter, unsigned()>;
  SECTION("Expect copy") {
    const auto copy_move_counter = CopyMoveCounter{};
    const auto wrapper = gte::TypeErased<CopyCounterFunction>{
        copy_move_counter, &CopyMoveCounter::copies};
    CHECK(wrapper.call<CopyCounter>() == 1);
  }
  SECTION("Expect no copies") {
    auto copy_move_counter = CopyMoveCounter{};
    const auto wrapper = gte::TypeErased<CopyCounterFunction>{
        std::move(copy_move_counter), &CopyMoveCounter::copies};
    CHECK(wrapper.call<CopyCounter>() == 0);
  }
}

TEST_CASE("Tags to map", "[wrapper]") {
  using Map =
      gte::detail::TagValueMap<int, gte::TagAndSignature<TheAnswer, void()> >;
  const auto map = typename Map::Map{42};
  CHECK(map.template get<TheAnswer>() == 42);
}

TEST_CASE("Member function map", "[wrapper]") {
  using Map = gte::detail::TagMemberFunctionMap<
      gte::TagAndSignature<TheAnswer, void()> >;
  static_assert(Map::Map::number_of_keys == 1);
}

TEST_CASE("Multiple const member functions", "[wrapper]") {
  using TheAnswerFunction = gte::TagAndConstSignature<TheAnswer, int()>;
  using MultiplyFunction =
      gte::TagAndConstSignature<MultiplyTheAnswer, int(int)>;

  auto t = Tester{};
  const auto wrapper = gte::TypeErased<TheAnswerFunction, MultiplyFunction>{
      t, &Tester::the_answer, &Tester::multiply_the_answer};

  CHECK(wrapper.call<TheAnswer>() == 42);
  CHECK(wrapper.call<MultiplyTheAnswer>(2) == 84);
}

TEST_CASE("Multiple member functions, mixed const, non-const", "[wrapper]") {
  using TheAnswerFunction = gte::TagAndSignature<TheAnswer, int()>;
  using SetFunction = gte::TagAndSignature<SetTheAnswer, int(int)>;

  auto t = Tester{};
  auto wrapper = gte::TypeErased<SetFunction, TheAnswerFunction>{
      t, &Tester::set_the_answer, &Tester::the_answer};

  CHECK(wrapper.call<TheAnswer>() == 42);
  CHECK(wrapper.call<SetTheAnswer>(43) == 42);
  CHECK(wrapper.call<TheAnswer>() == 43);
}

TEST_CASE("Copying and moving", "[wrapper]") {
  using TheAnswerFunction = gte::TagAndConstSignature<TheAnswer, int()>;
  using SetFunction = gte::TagAndSignature<SetTheAnswer, int(int)>;

  auto wrapper_1 = gte::TypeErased<SetFunction, TheAnswerFunction>{
      Tester{}, &Tester::set_the_answer, &Tester::the_answer};
  wrapper_1.call<SetTheAnswer>(43);

  SECTION("Copy") {
    const auto wrapper_2 = wrapper_1;
    CHECK(wrapper_2.call<TheAnswer>() == 43);
  }
  SECTION("Move") {
    const auto wrapper_2 = std::move(wrapper_1);
    CHECK(wrapper_2.call<TheAnswer>() == 43);
  }
}

TEST_CASE("Copying and moving with counts", "[wrapper]") {
  using CopyCounterFunction =
      gte::TagAndConstSignature<CopyCounter, unsigned()>;
  using MoveCounterFunction =
      gte::TagAndConstSignature<MoveCounter, unsigned()>;
  using CopyMoveCounterWrapper =
      gte::TypeErased<CopyCounterFunction, MoveCounterFunction>;

  auto wrapper_1 = CopyMoveCounterWrapper{
      CopyMoveCounter{}, &CopyMoveCounter::copies, &CopyMoveCounter::moves};

  CHECK(wrapper_1.call<CopyCounter>() == 0);
  CHECK(wrapper_1.call<MoveCounter>() == 1);

  SECTION("Copy") {
    const auto wrapper_2 = wrapper_1;
    CHECK(wrapper_2.call<CopyCounter>() == 1);
    CHECK(wrapper_2.call<MoveCounter>() == 1);
  }
  SECTION("Move") {
    const auto wrapper_2 = std::move(wrapper_1);
    CHECK(wrapper_2.call<CopyCounter>() == 0);
    CHECK(wrapper_2.call<MoveCounter>() == 1);
  }
}

TEST_CASE("Check usage in vector", "[wrapper]") {
  using TheAnswerFunction = gte::TagAndSignature<TheAnswer, int()>;
  using SetFunction = gte::TagAndSignature<SetTheAnswer, int(int)>;
  using Wrapper = gte::TypeErased<SetFunction, TheAnswerFunction>;
  auto wrapper_1 =
      Wrapper{Tester{}, &Tester::set_the_answer, &Tester::the_answer};
  auto wrapper_2 =
      Wrapper{Tester2{}, &Tester2::set_the_answer_2, &Tester2::the_answer_2};

  // Set objects values to the loop index, using two different types for
  // odd/even entries
  auto answers = std::vector<Wrapper>{};
  constexpr auto num_entries = 6;
  for (auto index = 0; index < num_entries; ++index) {
    if (index % 2) {
      answers.push_back(wrapper_1);
    } else {
      answers.push_back(wrapper_2);
    }
    answers.back().call<SetTheAnswer>(index);
  }

  // Check that values were set to the index
  for (auto index = 0; index < num_entries; ++index) {
    CHECK(answers.at(index).call<TheAnswer>() == index);
  }
}

TEST_CASE("Tag and signature constness", "[wrapper]") {
  static_assert(gte::TagAndSignature<Key1, int()>::is_const == false);
  static_assert(gte::TagAndConstSignature<Key1, int()>::is_const == true);
}

TEST_CASE("Const map", "[wrapper]") {
  using KeyWithSignature1 = gte::TagAndSignature<Key1, int()>;
  using KeyWithSignature2 = gte::TagAndConstSignature<Key2, int()>;
  constexpr auto test_map =
      gte::detail::const_map<KeyWithSignature1, KeyWithSignature2>();

  static_assert(!test_map.template get<Key1>());
  static_assert(test_map.template get<Key2>());
}
