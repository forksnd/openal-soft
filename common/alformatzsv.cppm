module;

#include "alformat.hpp"
#include "zstring_view.hpp"

export module format.zsv;

namespace {
    template<typename>
    struct is_zstring_view_ : std::false_type { };

    template<typename CharT, typename TraitsT>
    struct is_zstring_view_<al::basic_zstring_view<CharT, TraitsT>> : std::true_type { };

    template<typename T>
    inline auto constexpr is_zstring_view_v = is_zstring_view_<T>::value;
}

export {

template<typename T, typename CharT> requires(is_zstring_view_v<T>)
struct al::formatter<T, CharT> : formatter<typename T::underlying_type, CharT> {
    using fmttype_t = typename T::underlying_type;

    auto format(T const &zsv, auto& ctx) const
    { return formatter<fmttype_t,CharT>::format(zsv, ctx); }
};

} /* export */
