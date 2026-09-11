#ifndef AL_FORMATZSV_HPP
#define AL_FORMATZSV_HPP

#include "alformat.hpp"
#include "zstring_view.hpp"

namespace al::detail_ {
    template<typename>
    struct is_zstring_view_ : std::false_type { };

    template<typename CharT, typename TraitsT>
    struct is_zstring_view_<basic_zstring_view<CharT, TraitsT>> : std::true_type { };

    template<typename T>
    inline auto constexpr is_zstring_view_v = is_zstring_view_<T>::value;
} /* namespace al::detail_ */

template<typename T, typename CharT> requires(al::detail_::is_zstring_view_v<T>)
struct al::formatter<T, CharT> : formatter<typename T::underlying_type, CharT> {
    using fmttype_t = typename T::underlying_type;

    auto format(T const &zsv, auto& ctx) const
    { return formatter<fmttype_t,CharT>::format(zsv, ctx); }
};

#endif /* AL_FORMATZSV_HPP */