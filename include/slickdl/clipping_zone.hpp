#pragma once

#include "slickdl.hpp"
#include "slickdl/point.hpp"

namespace slickdl {

template < is_int_or_float T >
struct clippingZone {
    using native_t = isIntOrFloat_t< T, SDL_Rect, SDL_FRect >;

    constexpr clippingZone() = default;

    constexpr clippingZone( native_t& _rectangle )
        : minX( _rectangle.x ),
          minY( _rectangle.y ),
          maxX( minX + _rectangle.w ),
          maxY( minY + _rectangle.h ) {}

    [[nodiscard]] constexpr auto operator<=>( const clippingZone& _box ) const =
        default;

    constexpr auto operator=( native_t& _rectangle ) -> clippingZone& {
        *this = clippingZone( _rectangle );

        return ( *this );
    }

    constexpr operator native_t() const {
        return ( native_t{
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

template < typename T, typename ContainerType = std::vector< point_t< T > > >
[[nodiscard]] constexpr auto toPoints(
    std::span< const clippingZone_t< T > > _zones ) -> ContainerType {
    ContainerType l_returnValue;

    l_returnValue.reserve( _zones.size() * 2 );

    for ( const clippingZone_t< T >& _zone : _zones ) {
        l_returnValue.emplace_back( _zone.minX );
        l_returnValue.emplace_back( _zone.minY );
        l_returnValue.emplace_back( _zone.maxX );
        l_returnValue.emplace_back( _zone.maxY );
    }

    return ( l_returnValue );
}

} // namespace slickdl
