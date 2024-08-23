#ifndef TYPE_MAP_HPP
#define TYPE_MAP_HPP

#include "type-helpers.hpp"

#include <type_traits>

namespace gte
{
namespace detail
{
template<typename T, typename Tuple, std::size_t I>
constexpr auto get_index_impl() -> std::size_t
{
  if constexpr (std::is_same_v<T, std::tuple_element_t<I, Tuple>>)
   {
     return I;
   }
  else
  {
    return 0;
  }
}

template<typename T, typename Tuple, std::size_t... Indices>
constexpr auto get_index(std::index_sequence<Indices...>) -> std::size_t
{
  return (get_index_impl<T, Tuple, Indices>() + ...);
}

template<typename T, typename Tuple, std::size_t... Indices>
constexpr auto has_type(std::index_sequence<Indices...>) -> bool
{
  return (std::is_same_v<T, std::tuple_element_t<Indices, Tuple>> || ...);
}
}

template<typename... KeyValuePairs >
struct TypeMap
{
  template<typename... Args>
  TypeMap(Args&&... args) : m_values{std::forward_as_tuple(std::forward<Args>(args)...)}{}

  template<typename Key>
  const auto& get() const
  {
    constexpr auto number_of_keys = std::tuple_size_v<KeyTuple>;
    static_assert(detail::has_type<Key, KeyTuple>(std::make_index_sequence<number_of_keys>()));

    constexpr auto key_index = detail::get_index<Key, KeyTuple>(std::make_index_sequence<number_of_keys>());
    return std::get<key_index>(m_values);
  }
  
  using KeyTuple = typename detail::PairsToTuples<KeyValuePairs...>::FirstTuple;
  using ValueTuple = typename detail::PairsToTuples<KeyValuePairs...>::SecondTuple;

  ValueTuple m_values;
};
}

#endif
