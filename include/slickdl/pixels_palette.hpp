#pragma once

#include <SDL3/SDL_pixels.h>

#include <cstdint>
#include <gsl/pointers>
#include <ranges>
#include <span>
#include <string_view>
#include <variant>

#include "color.hpp"
#include "slickdl.hpp"
#include "stdfunc.hpp"

// Largely these facilities deal with pixel _format_: what does this set of
// bits represent?
//
// If you mostly want to think of a pixel as some combination of red, green,
// blue, and maybe alpha intensities, this is all pretty straightforward, and
// in many cases, is enough information to build a perfectly fine game
//
// However, the actual definition of a pixel is more complex than that:
//
// Pixels are a representation of a color in a particular color space
//
// The first characteristic of a color space is the color type. SDL
// understands two different color types, RGB and YCbCr, or in SDL also
// referred to as YUV
//
// RGB colors consist of red, green, and blue channels of color that are added
// together to represent the colors we see on the screen
//
// YCbCr colors represent colors as a Y luma brightness component and red and
// blue chroma color offsets. This color representation takes advantage of the
// fact that the human eye is more sensitive to brightness than the color in
// an image. The Cb and Cr components are often compressed and have lower
// resolution than the luma component
//
// When the color information in YCbCr is compressed, the Y pixels are left at
// full resolution and each Cr and Cb pixel represents an average of the color
// information in a block of Y pixels. The chroma location determines where in
// that block of pixels the color information is coming from
//
// The color range defines how much of the pixel to use when converting a
// pixel into a color on the display. When the full color range is used, the
// entire numeric range of the pixel bits is significant. When narrow color
// range is used, for historical reasons, the pixel uses only a portion of the
// numeric range to represent colors
//
// The color primaries and white point are a definition of the colors in the
// color space relative to the standard XYZ color space
//
// The transfer characteristic, or opto-electrical transfer function (OETF),
// is the way a color is converted from mathematically linear space into a
// non-linear output signals
//
// The matrix coefficients are used to convert between YCbCr and RGB colors
namespace slickdl {

using pixelFormatDetails_t = gsl::not_null< SDL_PixelFormatDetails* >;

template < std::unsigned_integral U >
    requires( sizeof( U ) <= sizeof( uint32_t ) )
using pixelValue_t = U;

using palette_t = struct palette {
    palette() = delete;

    // Create a palette structure with the specified number of color entries
    //
    // The palette entries are initialized to white
    palette( size_t _colorAmount )
        : _data( SDL_CreatePalette( _colorAmount ) ) {}

    palette( const palette& ) = default;

    // TODO: Maybe will assert
    palette( palette&& ) = default;

    template < typename OtherType >
        requires std::is_convertible_v< OtherType, SDL_Palette* >
    constexpr palette( OtherType&& _other )
        : _data( std::forward< OtherType >( _other ) ) {}

    ~palette() { SDL_DestroyPalette( _data ); }

    auto operator=( const palette& ) -> palette& = default;
    auto operator=( palette&& ) -> palette& = default;

    constexpr operator SDL_Palette*() const { return ( _data ); }

    // Set a range of colors in a palette
    void colors( std::span< const color_t > _colors,
                 size_t _firstEntryIndex = 0 ) {
        const std::vector< SDL_Color > l_colors =
            _colors |
            std::views::transform( []( const color_t& _color ) -> SDL_Color {
                return ( _color );
            } ) |
            std::ranges::to< std::vector >();

        const bool l_result = SDL_SetPaletteColors(
            _data, l_colors.data(), _firstEntryIndex, l_colors.size() );

        slickdl::assert( l_result );
    }

    // Map an RGBA quadruple to a pixel value for a given pixel format
    //
    // This function maps the RGBA color value to the specified pixel format and
    // returns the pixel value best approximating the given RGBA color value for
    // the given pixel format
    //
    // If the specified pixel format has no alpha component the alpha value will
    // be ignored (as it will be in formats with a palette)
    //
    // If the format has a palette (8-bit) the index of the closest matching
    // color in the palette will be returned
    //
    // If the pixel format bpp (color depth) is less than 32-bpp then the unused
    // upper bits of the return value can safely be ignored (e.g., with a 16-bpp
    // format the return value can be assigned to a Uint16, and similarly a
    // Uint8 for an 8-bpp format)
    template < std::unsigned_integral U >
        requires( sizeof( U ) <= sizeof( uint32_t ) )
    auto map( pixelFormatDetails_t _format, color_t _color )
        -> pixelValue_t< U > {
        return ( SDL_MapRGBA( _format, _data, _color.red, _color.green,
                              _color.blue, _color.alpha ) );
    }

    // Get RGBA values from a pixel in the specified format
    //
    // This function uses the entire 8-bit [0..255] range when converting color
    // components from pixel formats with less than 8-bits per RGB component
    // (e.g., a completely white pixel in 16-bit RGB565 format would return
    // [0xff, 0xff, 0xff] not [0xf8, 0xfc, 0xf8])
    //
    // If the surface has no alpha component, the alpha will be returned as 0xff
    // (100% opaque)
    template < std::unsigned_integral U >
        requires( sizeof( U ) <= sizeof( uint32_t ) )
    auto get( pixelValue_t< U > _value, pixelFormatDetails_t _format )
        -> color_t {
        color_t l_color{};

        SDL_GetRGBA( _value, _format, _data, &l_color.red, &l_color.green,
                     &l_color.blue, &l_color.alpha );

        return ( l_color );
    }

    // Variables
private:
    gsl::not_null< SDL_Palette* > _data;
};
} // namespace pixels

} // namespace slickdl
