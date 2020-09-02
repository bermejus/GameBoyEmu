#pragma once

#include <type_traits>

template <size_t Index, typename... Ts>
using nth_type = typename std::tuple_element_t<Index, std::tuple<Ts...>>;

template <class T, class... Ts>
struct all_same : std::conjunction<std::is_same<T, Ts>...>{};

template <class T, class... Ts>
constexpr inline bool all_same_v = all_same<T, Ts...>::value;

template <class T, class... Ts>
struct least_same : std::disjunction<std::is_same<T, Ts>...>{};

template <class T, class... Ts>
constexpr inline bool least_same_v = least_same<T, Ts...>::value;