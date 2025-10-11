#pragma once

#include "slickdl.hpp"

namespace slickdl {

template < typename T >
    requires is_int_or_float< T >
struct clippingZone {
    constexpr clippingZone( SDL_Rect& _rectangle )
        : minX( _rectangle.x ),
          minY( _rectangle.y ),
          maxX( minX + _rectangle.w ),
          maxY( minY + _rectangle.h ) {}

    [[nodiscard]] constexpr auto operator<=>(
        const clippingZone< T >& _box ) const = default;

    constexpr auto operator=( SDL_Rect& _rectangle ) -> clippingZone& {
        minX = _rectangle.x;
        minY = _rectangle.y;
        maxX = ( minX + _rectangle.w );
        maxY = ( minY + _rectangle.h );

        return ( *this );
    }

    explicit constexpr operator SDL_Rect() const {
        return ( SDL_Rect{
            minX,
            minY,
            ( maxX - minX ),
            ( maxY - minY ),
        } );
    }

    T minX, minY, maxX, maxY;
};

template < typename T >
    requires is_int_or_float< T >
using clippingZone_t = struct clippingZone< T >;

} // namespace slickdl
