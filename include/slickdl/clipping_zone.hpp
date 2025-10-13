#pragma once

#include "slickdl.hpp"

namespace slickdl {

template < is_int_or_float T >
struct clippingZone {
    constexpr clippingZone() = default;

    constexpr clippingZone( SDL_Rect& _rectangle )
        : minX( _rectangle.x ),
          minY( _rectangle.y ),
          maxX( minX + _rectangle.w ),
          maxY( minY + _rectangle.h ) {}

    [[nodiscard]] constexpr auto operator<=>( const clippingZone& _box ) const =
        default;

    constexpr auto operator=( SDL_Rect& _rectangle ) -> clippingZone& {
        *this = clippingZone( _rectangle );

        return ( *this );
    }

    constexpr operator SDL_Rect() const {
        return ( SDL_Rect{
            minX,
            minY,
            ( maxX - minX ),
            ( maxY - minY ),
        } );
    }

    [[nodiscard]] constexpr auto empty() const -> bool {
        return ( *this == clippingZone{} );
    }

    T minX, minY, maxX, maxY;
};

template < typename T >
using clippingZone_t = struct clippingZone< T >;

} // namespace slickdl
