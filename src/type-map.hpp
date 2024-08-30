#ifndef TYPE_MAP_HPP
#define TYPE_MAP_HPP

#include <type_traits>

#include "type-helpers.hpp"

namespace gte {
namespace detail {
template <typename T, typename Tuple, std::size_t I>
constexpr auto get_index_impl() -> std::size_t {
  if constexpr (std::is_same_v<T, std::tuple_element_t<I, Tuple> >) {
    return I;
  } else {
    return 0;
  }
}

template <typename T, typename Tuple, std::size_t... Indices>
constexpr auto get_index(std::index_sequence<Indices...>) -> std::size_t {
  return (get_index_impl<T, Tuple, Indices>() + ...);
}

template <typename T, typename Tuple, std::size_t... Indices>
constexpr auto has_type(std::index_sequence<Indices...>) -> bool {
  return (std::is_same_v<T, std::tuple_element_t<Indices, Tuple> > || ...);
}

template <typename T, typename Tuple, std::size_t... Indices>
constexpr auto type_count(std::index_sequence<Indices...>) -> std::size_t {
  return (static_cast<std::size_t>(
              std::is_same_v<T, std::tuple_element_t<Indices, Tuple> >) +
          ...);
}

template <typename T, typename Tuple, std::size_t... Indices>
constexpr auto type_is_unique(std::index_sequence<Indices...> indices)
    -> std::size_t {
  return type_count<T, Tuple>(indices) == 1;
}

template <typename Tuple, std::size_t... Indices>
constexpr auto all_types_unique(std::index_sequence<Indices...> indices)
    -> bool {
  return (
      type_is_unique<std::tuple_element_t<Indices, Tuple>, Tuple>(indices) &&
      ...);
}
}  // namespace detail

template <typename... KeyValuePairs>
struct TypeMap {
  template <typename... Args>
  constexpr TypeMap(Args &&...args)
      : m_values{std::forward_as_tuple(std::forward<Args>(args)...)} {
    static_assert(detail::all_types_unique<KeyTuple>(
        std::make_index_sequence<number_of_keys>()));
  }

  template <typename Key>
  constexpr const auto &get() const {
    static_assert(detail::has_type<Key, KeyTuple>(
        std::make_index_sequence<number_of_keys>()));

    constexpr auto key_index = detail::get_index<Key, KeyTuple>(
        std::make_index_sequence<number_of_keys>());
    return std::get<key_index>(m_values);
  }

  using KeyTuple = typename detail::PairsToTuples<KeyValuePairs...>::FirstTuple;
  using ValueTuple =
      typename detail::PairsToTuples<KeyValuePairs...>::SecondTuple;
  static constexpr auto number_of_keys = std::tuple_size_v<KeyTuple>;

  ValueTuple m_values;
};
}  // namespace gte

#endif
