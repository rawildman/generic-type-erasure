#include "type-map.hpp"

#include <catch2/catch_test_macros.hpp>

namespace
{
struct Key1
{
};
struct Key2
{
};
}

TEST_CASE ("Get index", "[typemap]")
{
  using TestTuple = std::tuple<int, double, char>;
  constexpr auto indices
      = std::make_index_sequence<std::tuple_size_v<TestTuple> > ();

  static_assert (gte::detail::get_index<int, TestTuple> (indices) == 0);
  static_assert (gte::detail::get_index<double, TestTuple> (indices) == 1);
  static_assert (gte::detail::get_index<char, TestTuple> (indices) == 2);
}

TEST_CASE ("Has type", "[typemap]")
{
  using TestTuple = std::tuple<Key1, int, Key2>;
  constexpr auto indices
      = std::make_index_sequence<std::tuple_size_v<TestTuple> > ();

  static_assert (gte::detail::has_type<Key1, TestTuple> (indices));
  static_assert (gte::detail::has_type<Key2, TestTuple> (indices));
  static_assert (gte::detail::has_type<int, TestTuple> (indices));
  static_assert (!gte::detail::has_type<double, TestTuple> (indices));
}

TEST_CASE ("Type map", "[typemap]")
{
  using TestMap = gte::TypeMap<std::pair<Key1, int>, std::pair<Key2, double> >;

  const auto test_map = TestMap{ 42, 84.0 };

  REQUIRE (test_map.template get<Key1> () == 42);
  REQUIRE (test_map.template get<Key2> () == 84.0);
}
