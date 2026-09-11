module;

#include <type_traits>

#include "alformat.hpp"
#include "zstring_view.hpp"

export module format.zsv;

namespace {
    template<typename T, template<typename...> typename U>
    inline constexpr auto is_instance_of_v = false;

    template<template<typename...> typename U, typename... Vs>
    inline constexpr auto is_instance_of_v<U<Vs...>, U> = true;

    template<typename T>
    concept zstring_view_type = is_instance_of_v<std::remove_cvref_t<T>, al::basic_zstring_view>;
}

export {

template<zstring_view_type T, typename CharT>
struct al::formatter<T, CharT> : formatter<typename T::underlying_type, CharT> {
    using fmttype_t = typename T::underlying_type;

    auto format(T const &zsv, auto& ctx) const
    { return formatter<fmttype_t,CharT>::format(zsv, ctx); }
};

} /* export */
