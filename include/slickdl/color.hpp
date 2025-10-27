#pragma once

#include <SDL3/SDL_pixels.h>

#include <array>
#include <bit>
#include <cstdint>
#include <limits>

namespace slickdl {

using color_t = struct color {
    using native_t = SDL_Color;

    static constexpr size_t g_maxValue = std::numeric_limits< uint8_t >::max();

    color() = default;
    color( const color& ) = default;
    color( color&& ) = default;
    ~color() = default;

    constexpr color( uint8_t _red,
                     uint8_t _green,
                     uint8_t _blue,
                     uint8_t _alpha )
        : red( _red ), green( _green ), blue( _blue ), alpha( _alpha ) {}

    constexpr color( uint8_t _red, uint8_t _green, uint8_t _blue )
        : color( _red, _green, _blue, g_maxValue ) {}

    constexpr color( uint32_t _colorPacked ) { unpack( _colorPacked ); }

    constexpr color( native_t _color )
        : color( _color.r, _color.g, _color.b, _color.a ) {}

    constexpr color( SDL_FColor _color )
        : color( ( _color.r * g_maxValue ),
                 ( _color.g * g_maxValue ),
                 ( _color.b * g_maxValue ),
                 ( _color.a * g_maxValue ) ) {}

    auto operator=( const color& ) -> color& = default;
    auto operator=( color&& ) -> color& = default;

    [[nodiscard]] constexpr operator native_t() const {
        static_assert( sizeof( decltype( *this ) ) == sizeof( native_t ) );

        return ( std::bit_cast< native_t >( *this ) );
    }

    [[nodiscard]] constexpr operator native_t*() {
        static_assert( sizeof( decltype( *this ) ) == sizeof( native_t ) );

        return ( std::bit_cast< native_t* >( this ) );
    }

    [[nodiscard]] constexpr operator const native_t*() const {
        static_assert( sizeof( decltype( *this ) ) == sizeof( native_t ) );

        return ( std::bit_cast< native_t* >( this ) );
    }

    explicit constexpr operator SDL_FColor() const {
        return ( SDL_FColor{
            .r = ( static_cast< float >( red ) / g_maxValue ),
            .g = ( static_cast< float >( green ) / g_maxValue ),
            .b = ( static_cast< float >( blue ) / g_maxValue ),
            .a = ( static_cast< float >( alpha ) / g_maxValue ),
        } );
    }

    [[nodiscard]] constexpr auto pack() const -> uint32_t {
        std::array< uint8_t, 4 > l_bytes{};

        if constexpr ( std::endian::native == std::endian::little ) {
            l_bytes = {
                alpha,
                blue,
                green,
                red,
            };

        } else {
            l_bytes = {
                red,
                green,
                blue,
                alpha,
            };
        }

        return ( std::bit_cast< uint32_t >( l_bytes ) );
    }

    constexpr void unpack( uint32_t _colorPacked ) {
        const auto l_bytes =
            std::bit_cast< std::array< uint8_t, 4 > >( _colorPacked );

        // NOTE: Use decomposition when it is available in constexpr
        if constexpr ( std::endian::native == std::endian::little ) {
            red = l_bytes[ 3 ];
            green = l_bytes[ 2 ];
            blue = l_bytes[ 1 ];
            alpha = l_bytes[ 0 ];

        } else {
            red = l_bytes[ 0 ];
            green = l_bytes[ 1 ];
            blue = l_bytes[ 2 ];
            alpha = l_bytes[ 3 ];
        }
    }

    uint8_t red{};
    uint8_t green{};
    uint8_t blue{};
    uint8_t alpha{};
};

} // namespace slickdl
