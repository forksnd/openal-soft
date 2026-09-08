#ifndef AL_DYNLOAD_H
#define AL_DYNLOAD_H

#include "config.h"

#if HAVE_DYNLOAD && (defined(_WIN32) || defined(HAVE_DLFCN_H))

#include <new>
#include <string>

#include "expected.hpp"
#include "gsl/gsl"

#include "dlopennote.h"

[[nodiscard]]
auto LoadLib(gsl::czstring name) -> al::expected<void*, std::string>;
void CloseLib(void *handle);
[[nodiscard]]
auto GetSymbol_(void *handle, gsl::czstring name) -> al::expected<void*, std::string>;

template<typename T> [[nodiscard]]
auto GetSymbolAddress(void *const handle, gsl::czstring const name)
    -> al::expected<T*, std::string>
{
    return GetSymbol_(handle, name)
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) */
        .transform([](void *fn) { return reinterpret_cast<T*>(fn); });
}

#endif

#endif /* AL_DYNLOAD_H */
