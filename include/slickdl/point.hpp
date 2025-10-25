#pragma once

#include "slickdl.hpp"
#include "stdinplace_vector.hpp"

namespace slickdl {

// The structure that defines a point
template < is_int_or_float T >
struct point {
    using native_t = isIntOrFloat_t< T, SDL_Point, SDL_FPoint >;

    point() = default;

    point( T _x, T _y ) : x( _x ), y( _y ) {}

    constexpr point( const native_t& _rectangle )
        : x( _rectangle.x ), y( _rectangle.y ) {}

    [[nodiscard]] constexpr auto operator<=>( const point< T >& _box ) const =
        default;

    template < typename U >
    [[nodiscard]] explicit constexpr operator point< U >() const {
        return ( point< U >{
            static_cast< U >( x ),
            static_cast< U >( y ),
        } );
    }

    [[nodiscard]] constexpr operator native_t() const {
        static_assert( sizeof( decltype( *this ) ) == sizeof( native_t ) );

        return ( std::bit_cast< native_t >( *this ) );
    }

    [[nodiscard]] constexpr operator native_t*() const {
        static_assert( sizeof( decltype( *this ) ) == sizeof( native_t ) );

        return ( std::bit_cast< native_t* >( this ) );
    }

    T x{}, y{};
};

template < typename T >
using point_t = point< T >;

template < typename T >
concept has_points = ( requires( const T& _argument ) {
    { T::g_pointsAmount } -> std::convertible_to< size_t >;
    { _argument.points() } -> std::ranges::input_range;
} );

template < typename T, template < typename > typename U >
    requires( !std::is_same_v< U< T >, std::span< T > > &&
              has_points< U< T > > )
[[nodiscard]] constexpr auto toPoints( const U< T > _somethingWithPoints )
    -> auto {
    stdfunc::inplaceVector_t< point_t< T >, U< T >::g_pointsAmount >
        l_returnValue;

    for ( const point_t< T >& _point : _somethingWithPoints.points() ) {
        l_returnValue.emplace_back( _point );
    }

    return ( l_returnValue );
}

template < typename T, template < typename > typename U >
    requires has_points< U< T > >
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
