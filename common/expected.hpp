#ifndef AL_EXPECTED_HPP
#define AL_EXPECTED_HPP

#include <type_traits>
#include <utility>
#include <variant>

#include "opthelpers.h"

namespace al {

template<typename E>
class unexpected {
    E mError;

public:
    constexpr unexpected(const unexpected&) = default;
    constexpr unexpected(unexpected&&) = default;
    template<typename E2=E> requires(!std::is_same_v<std::remove_cvref_t<E2>, unexpected>
        && !std::is_same_v<std::remove_cvref_t<E2>, std::in_place_t>
        && std::is_constructible_v<E, E2>)
    constexpr explicit unexpected(E2&& rhs) : mError{std::forward<E2>(rhs)}
    { }
    template<typename ...Args> requires(std::is_constructible_v<E, Args...>)
    constexpr explicit unexpected(std::in_place_t, Args&& ...args)
        : mError{std::forward<Args>(args)...}
    { }
    template<typename U, typename ...Args>
        requires(std::is_constructible_v<E, std::initializer_list<U>&, Args...>)
    constexpr explicit unexpected(std::in_place_t, std::initializer_list<U> il, Args&& ...args)
        : mError{il, std::forward<Args>(args)...}
    { }

    [[nodiscard]] constexpr auto error() const& noexcept -> const E& { return mError; }
    [[nodiscard]] constexpr auto error() & noexcept -> E& { return mError; }
    [[nodiscard]] constexpr auto error() const&& noexcept -> const E&& { return std::move(mError); }
    [[nodiscard]] constexpr auto error() && noexcept -> E&& { return std::move(mError); }

    constexpr void swap(unexpected& other) noexcept(std::is_nothrow_swappable_v<E>)
    { std::swap(mError, other.mError); }

    template<typename E2>
    friend constexpr auto operator==(const unexpected& lhs, const unexpected<E2>& rhs) -> bool
    { return lhs.error() == rhs.error(); }

    friend constexpr void swap(unexpected& lhs, unexpected& rhs) noexcept(noexcept(lhs.swap(rhs)))
    { lhs.swap(rhs); }
};

template<typename E>
unexpected(E) -> unexpected<E>;

namespace detail_ {

template<typename T> struct VSType { using type = T; };
template<> struct VSType<void> { using type = std::monostate; };
template<typename T>
using VSType_t = typename VSType<T>::type;
}

template<typename Ty, typename Er>
class [[nodiscard]] expected {
    using S = detail_::VSType_t<Ty>;
    using variant_type = std::variant<S, Er>;

    static constexpr auto void_success = std::is_same_v<std::remove_cv_t<Ty>, void>;

    std::variant<S, Er> mValues;

public:
    constexpr expected() noexcept(std::is_nothrow_default_constructible_v<variant_type>) = default;
    constexpr expected(const expected &rhs) noexcept(std::is_nothrow_copy_constructible_v<variant_type>) = default;
    constexpr expected(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<variant_type>) = default;

    /* Value constructors */
    template<typename U=std::remove_cv_t<Ty>>
        requires(not std::is_same_v<std::remove_cvref_t<U>, std::in_place_t>
            and not std::is_same_v<expected, std::remove_cvref_t<U>>
            and std::is_constructible_v<Ty, U>)
    constexpr explicit(!std::is_convertible_v<U, S>) expected(U&& v)
        : mValues{std::in_place_index<0>, std::forward<U>(v)}
    { }

    template<typename ...Args>
    constexpr explicit
    expected(std::in_place_t, Args&& ...args)
        requires(not std::is_same_v<std::remove_cv_t<Ty>, void>
            and std::is_constructible_v<Ty, Args...>)
        : mValues{std::in_place_index<0>, std::forward<Args>(args)...}
    { }

    constexpr explicit
    expected(std::in_place_t) noexcept requires(std::is_same_v<std::remove_cv_t<Ty>, void>)
    { }

    /* Error constructors */
    template<typename U> requires(std::is_constructible_v<Er, const U&>)
    constexpr explicit(not std::is_convertible_v<const U&, Er>)
    expected(const unexpected<Ty> &rhs)
        : mValues{std::in_place_index<1>, rhs.error()}
    { }

    template<typename U> requires(std::is_constructible_v<Er, U>)
    constexpr explicit(!std::is_convertible_v<U, Er>) expected(unexpected<U>&& rhs)
        : mValues{std::in_place_index<1>, std::move(rhs).error()}
    { }

    template<typename ...Args> requires(std::is_nothrow_constructible_v<Ty, Args...>) constexpr
    auto emplace(Args&& ...args) & noexcept LIFETIMEBOUND -> expected&
    {
        mValues.template emplace<0>(std::forward<Args>(args)...);
        return *this;
    }

    [[nodiscard]] constexpr auto has_value() const noexcept -> bool { return mValues.index() == 0; }
    [[nodiscard]] constexpr explicit operator bool() const noexcept { return has_value(); }

    [[nodiscard]] constexpr auto operator*() & noexcept -> S& requires(not void_success)
    { return *std::get_if<0>(&mValues); }
    [[nodiscard]] constexpr auto operator*() const& noexcept -> S const& requires(not void_success)
    { return *std::get_if<0>(&mValues); }
    [[nodiscard]] constexpr auto operator*() && noexcept -> S&& requires(not void_success)
    { return std::move(*std::get_if<0>(&mValues)); }
    [[nodiscard]] constexpr
    auto operator*() const&& noexcept -> S const&& requires(not void_success)
    { return std::move(*std::get_if<0>(&mValues)); }

    [[nodiscard]] constexpr auto value() & -> S& requires(not void_success)
    { return std::get<0>(mValues); }
    [[nodiscard]] constexpr auto value() const& -> const S& requires(not void_success)
    { return std::get<0>(mValues); }
    [[nodiscard]] constexpr auto value() && -> S&& requires(not void_success)
    { return std::move(std::get<0>(mValues)); }
    [[nodiscard]] constexpr auto value() const&& -> const S&& requires(not void_success)
    { return std::move(std::get<0>(mValues)); }

    [[nodiscard]] constexpr auto operator->() noexcept -> S* requires(not void_success)
    { return std::get_if<0>(&mValues); }
    [[nodiscard]] constexpr auto operator->() const noexcept -> const S* requires(not void_success)
    { return std::get_if<0>(&mValues); }

    template<typename U> [[nodiscard]] constexpr
    auto value_or(U&& defval) const& -> S requires(not void_success)
    { return bool{*this} ? **this : static_cast<S>(std::forward<U>(defval)); }
    template<typename U> [[nodiscard]] constexpr
    auto value_or(U&& defval) && -> S requires(not void_success)
    { return bool{*this} ? std::move(**this) : static_cast<S>(std::forward<U>(defval)); }

    [[nodiscard]] constexpr auto error() & -> Er& { return std::get<1>(mValues); }
    [[nodiscard]] constexpr auto error() const& -> const Er& { return std::get<1>(mValues); }
    [[nodiscard]] constexpr auto error() && -> Er&& { return std::move(std::get<1>(mValues)); }
    [[nodiscard]] constexpr auto error() const&& -> const Er&& { return std::move(std::get<1>(mValues)); }

    template<typename F> [[nodiscard]] constexpr
    auto and_then(F&& fn) &
    {
        using ret_t = std::remove_cvref_t<std::invoke_result_t<F&&, Ty&>>;
        if(has_value())
            return std::invoke(std::forward<F>(fn), **this);
        return ret_t{al::unexpected(error())};
    }
    template<typename F> [[nodiscard]] constexpr
    auto and_then(F&& fn) const&
    {
        using ret_t = std::remove_cvref_t<std::invoke_result_t<F&&, Ty const&>>;
        if(has_value())
            return std::invoke(std::forward<F>(fn), **this);
        return ret_t{al::unexpected(error())};
    }
    template<typename F> [[nodiscard]] constexpr
    auto and_then(F&& fn) &&
    {
        using ret_t = std::remove_cvref_t<std::invoke_result_t<F&&, Ty&&>>;
        if(has_value())
            return std::invoke(std::forward<F>(fn), std::move(**this));
        return ret_t{al::unexpected(error())};
    }
    template<typename F> [[nodiscard]] constexpr
    auto and_then(F&& fn) const&&
    {
        using ret_t = std::remove_cvref_t<std::invoke_result_t<F&&, Ty const&&>>;
        if(has_value())
            return std::invoke(std::forward<F>(fn), std::move(**this));
        return ret_t{al::unexpected(error())};
    }
};

} /* namespace al */

#endif /* AL_EXPECTED_HPP */
