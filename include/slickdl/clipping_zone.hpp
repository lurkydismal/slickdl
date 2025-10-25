#pragma once

#include "slickdl.hpp"
#include "slickdl/point.hpp"

namespace slickdl {

template < is_int_or_float T >
struct clippingZone {
    using native_t = isIntOrFloat_t< T, SDL_Rect, SDL_FRect >;

    static constexpr size_t g_pointsAmount = 2;

    clippingZone() = default;

    constexpr clippingZone( T _minX, T _minY, T _maxX, T _maxY )
        : minX( _minX ), minY( _minY ), maxX( _maxX ), maxY( _maxY ) {}

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

    template < typename U >
    [[nodiscard]] constexpr operator clippingZone< U >() const {
        return ( clippingZone< U >{
            static_cast< U >( minX ),
            static_cast< U >( minY ),
            static_cast< U >( maxX ),
            static_cast< U >( maxY ),
        } );
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

    [[nodiscard]] constexpr auto points() const
        -> std::array< point_t< T >, g_pointsAmount > {
        return {
            point_t< T >( minX, minY ),
            point_t< T >( maxX, maxY ),
        };
    }

    T minX{}, minY{}, maxX{}, maxY{};
};

template < typename T >
using clippingZone_t = struct clippingZone< T >;

} // namespace slickdl
