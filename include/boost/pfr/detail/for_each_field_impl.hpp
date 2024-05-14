// Copyright (c) 2016-2024 Antony Polukhin
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PFR_DETAIL_FOR_EACH_FIELD_IMPL_HPP
#define BOOST_PFR_DETAIL_FOR_EACH_FIELD_IMPL_HPP
#pragma once

#include <boost/pfr/detail/config.hpp>

#include <utility>      // metaprogramming stuff

#include <boost/pfr/detail/sequence_tuple.hpp>
#include <boost/pfr/detail/rvalue_t.hpp>
#include <boost/pfr/detail/core_name.hpp>

namespace boost { namespace pfr { namespace detail {

template <std::size_t Index>
using size_t_ = std::integral_constant<std::size_t, Index >;

template <class T, class F, class I, class = decltype(std::declval<F>()(std::declval<T>(), I{}))>
constexpr void for_each_field_impl_apply(T&& v, F&& f, I i, long) {
    std::forward<F>(f)(std::forward<T>(v), i);
}

template <class T, class F, class I>
constexpr void for_each_field_impl_apply(T&& v, F&& f, I /*i*/, int) {
    std::forward<F>(f)(std::forward<T>(v));
}

#if BOOST_PFR_CORE_NAME_ENABLED

template <typename T, typename F>
concept WithNameAndIndex = requires(T value, F f, std::string_view name, std::integral_constant<std::size_t, 0> i)
{
    { f(name, value, i) };
};

template <typename T, typename F>
concept WithNameOnly = requires(T value, F f, std::string_view name)
{
    { f(name, value) };
};

template <typename T, typename F, typename I>
constexpr void for_each_field_with_name_impl_apply(std::string_view name, T&& v, F&& f, I i) {
    std::forward<F>(f)(name, std::forward<T>(v), i);
}

template <typename T, typename F>
constexpr void for_each_field_with_name_impl_apply(std::string_view name, T&& v, F&& f) {
    std::forward<F>(f)(name, std::forward<T>(v));
}

#endif

#if !defined(__cpp_fold_expressions) || __cpp_fold_expressions < 201603
template <class T, class F, std::size_t... I>
constexpr void for_each_field_impl(T& t, F&& f, std::index_sequence<I...>, std::false_type /*move_values*/) {
     const int v[] = {0, (
         detail::for_each_field_impl_apply(sequence_tuple::get<I>(t), std::forward<F>(f), size_t_<I>{}, 1L),
         0
     )...};
     (void)v;
}


template <class T, class F, std::size_t... I>
constexpr void for_each_field_impl(T& t, F&& f, std::index_sequence<I...>, std::true_type /*move_values*/) {
     const int v[] = {0, (
         detail::for_each_field_impl_apply(sequence_tuple::get<I>(std::move(t)), std::forward<F>(f), size_t_<I>{}, 1L),
         0
     )...};
     (void)v;
}
#else


template <class A, class T, class F, std::size_t... I>
constexpr void for_each_field_impl(T& t, F&& f, std::index_sequence<I...>, std::false_type /*move_values*/) {
     using FirstElemType = sequence_tuple::tuple_element<0, T>::type;
     if constexpr (WithNameOnly<FirstElemType, F>)
     {
        (detail::for_each_field_with_name_impl_apply(detail::get_name<A, I>(), sequence_tuple::get<I>(t), std::forward<F>(f)), ...);
     }
     else if constexpr (WithNameAndIndex<FirstElemType, F>)
     {
        (detail::for_each_field_with_name_impl_apply(detail::get_name<A, I>(), sequence_tuple::get<I>(t), std::forward<F>(f), size_t_<I>{}), ...);
     }
     else
     {
        (detail::for_each_field_impl_apply(sequence_tuple::get<I>(t), std::forward<F>(f), size_t_<I>{}, 1L), ...);
     }
}

template <class A, class T, class F, std::size_t... I>
constexpr void for_each_field_impl(T& t, F&& f, std::index_sequence<I...>, std::true_type /*move_values*/) {
     using FirstElemType = sequence_tuple::tuple_element<0, T>::type;
     if constexpr (WithNameOnly<FirstElemType, F>)
     {
        (detail::for_each_field_with_name_impl_apply(detail::get_name<A, I>(), sequence_tuple::get<I>(std::move(t)), std::forward<F>(f)), ...);
     }
     else if constexpr (WithNameAndIndex<FirstElemType, F>)
     {
        (detail::for_each_field_with_name_impl_apply(detail::get_name<A, I>(), sequence_tuple::get<I>(std::move(t)), std::forward<F>(f), size_t_<I>{}), ...);
     }
     else
     {
        (detail::for_each_field_impl_apply(sequence_tuple::get<I>(std::move(t)), std::forward<F>(f), size_t_<I>{}, 1L), ...);
     }
}
#endif

}}} // namespace boost::pfr::detail


#endif // BOOST_PFR_DETAIL_FOR_EACH_FIELD_IMPL_HPP
