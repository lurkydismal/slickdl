#pragma once

#include "slickdl.hpp"

namespace slickdl {

// The structure that defines a point
template < is_int_or_float T >
struct point {
    using native_t = isIntOrFloat_t< T, SDL_Point, SDL_FPoint >;

    point() = default;

    constexpr point( native_t& _rectangle )
        : x( _rectangle.x ), y( _rectangle.y ) {}

    [[nodiscard]] constexpr auto operator<=>( const point< T >& _box ) const =
        default;

    template < typename U >
    [[nodiscard]] constexpr operator point< U >() const {
        return ( point< U >{
            static_cast< U >( x ),
            static_cast< U >( y ),
        } );
    }

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

template < typename T, template < typename > typename U >
[[nodiscard]] constexpr auto toPoints(
    std::span< const U< T > > _somethingWithPoints )
    -> std::vector< point_t< T > > {
    std::vector< point_t< T > > l_returnValue;

    l_returnValue.reserve( _somethingWithPoints.size() *
                           _somethingWithPoints.front().points().size() );

    for ( const U< T >& _something : _somethingWithPoints ) {
        for ( const point_t< T >& _point : _something.points() ) {
            l_returnValue.emplace_back( _point );
        }
    }

    return ( l_returnValue );
}

} // namespace slickdl
