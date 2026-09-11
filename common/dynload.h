#ifndef AL_DYNLOAD_H
#define AL_DYNLOAD_H

#include "config.h"

#if HAVE_DYNLOAD && (defined(_WIN32) || defined(HAVE_DLFCN_H))

#include <new>
#include <string>

#include "expected.hpp"
#include "gsl/gsl"

#include "dlopennote.h"
#include "zstring_view.hpp"

[[nodiscard]]
auto LoadLib(al::zstring_view name) -> al::expected<void*, std::string>;
void CloseLib(void *handle);
[[nodiscard]]
auto GetSymbol_(void *handle, al::zstring_view name) -> al::expected<void*, std::string>;

template<typename T> [[nodiscard]]
auto GetSymbolAddress(void *const handle, al::zstring_view const name)
    -> al::expected<T*, std::string>
{
    return GetSymbol_(handle, name)
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) */
        .transform([](void *fn) { return reinterpret_cast<T*>(fn); });
}

#endif

#endif /* AL_DYNLOAD_H */
