#pragma once

#include "slickdl.hpp"

namespace slickdl {

// The structure that defines a point
template < is_int_or_float T >
struct point {
    using native_t = isIntOrFloat_t< T, SDL_Point, SDL_FPoint >;

    [[nodiscard]] constexpr auto operator<=>( const point< T >& _box ) const =
        default;

    [[nodiscard]] constexpr operator native_t() const {
        return ( native_t{
            x,
            y,
        } );
    }

    T x, y;
};

template < typename T >
using point_t = point< T >;

} // namespace slickdl
