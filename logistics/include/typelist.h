#pragma once

#include <utility>

template<size_t I, class T>
struct TypeListNode {
};

template<class Is, class... Ts>
struct TypeListImpl;

template<size_t... Is, class... Ts>
struct TypeListImpl<std::index_sequence<Is...>, Ts...> : TypeListNode<Is, Ts>... {
};

template<class... Ts>
struct TypeList : TypeListImpl<std::make_index_sequence<sizeof...(Ts)>, Ts...> {
  static constexpr size_t size = sizeof...(Ts);
};