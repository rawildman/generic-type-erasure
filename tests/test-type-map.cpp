#include <catch2/catch_test_macros.hpp>

#include "type-map.hpp"

namespace {
struct Key1 {};
struct Key2 {};
}  // namespace

TEST_CASE("Get index", "[typemap]") {
  using TestTuple = std::tuple<int, double, char>;
  constexpr auto indices =
      std::make_index_sequence<std::tuple_size_v<TestTuple> >();

  static_assert(gte::detail::get_index<int, TestTuple>(indices) == 0);
  static_assert(gte::detail::get_index<double, TestTuple>(indices) == 1);
  static_assert(gte::detail::get_index<char, TestTuple>(indices) == 2);
}

TEST_CASE("Has type", "[typemap]") {
  using TestTuple = std::tuple<Key1, int, Key2>;
  constexpr auto indices =
      std::make_index_sequence<std::tuple_size_v<TestTuple> >();

  static_assert(gte::detail::has_type<Key1, TestTuple>(indices));
  static_assert(gte::detail::has_type<Key2, TestTuple>(indices));
  static_assert(gte::detail::has_type<int, TestTuple>(indices));
  static_assert(!gte::detail::has_type<double, TestTuple>(indices));
}

TEST_CASE("Number of types", "[typemap]") {
  using TestTuple = std::tuple<Key1, int, Key2, int>;
  constexpr auto indices =
      std::make_index_sequence<std::tuple_size_v<TestTuple> >();

  static_assert(gte::detail::type_count<Key1, TestTuple>(indices) == 1);
  static_assert(gte::detail::type_count<Key2, TestTuple>(indices) == 1);
  static_assert(gte::detail::type_count<int, TestTuple>(indices) == 2);
  static_assert(gte::detail::type_count<double, TestTuple>(indices) == 0);
}

TEST_CASE("All types unique", "[typemap]") {
  SECTION("Non-unique") {
    using TestTuple = std::tuple<Key1, int, Key2, int>;
    constexpr auto indices =
        std::make_index_sequence<std::tuple_size_v<TestTuple> >();
    static_assert(!gte::detail::all_types_unique<TestTuple>(indices));
  }
  SECTION("Unique") {
    using TestTuple = std::tuple<Key1, Key2, int>;
    constexpr auto indices =
        std::make_index_sequence<std::tuple_size_v<TestTuple> >();
    static_assert(gte::detail::all_types_unique<TestTuple>(indices));
  }
}

TEST_CASE("Type map", "[typemap]") {
  using TestMap = gte::TypeMap<std::pair<Key1, int>, std::pair<Key2, double> >;

  const auto test_map = TestMap{42, 84.0};

  CHECK(test_map.get<Key1>() == 42);
  CHECK(test_map.get<Key2>() == 84.0);
}

TEST_CASE("Type map to constexpr bool", "[typemap]")
{
  using TestMap = gte::TypeMap<std::pair<Key1, bool>, std::pair<Key2, bool>>;
  constexpr auto test_map = TestMap{true, false};

  static_assert(test_map.get<Key1>());
  static_assert(!test_map.get<Key2>());
}
