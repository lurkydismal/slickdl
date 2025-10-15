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
    auto color( pixelValue_t< U > _value, pixelFormatDetails_t _format )
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

namespace pixels {

// Palette
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
auto map( pixelFormatDetails_t _format, color_t _color ) -> U {
    return ( SDL_MapRGBA( _format, nullptr, _color.red, _color.green,
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
auto color( pixelValue_t< U > _value, pixelFormatDetails_t _format )
    -> color_t {
    color_t l_color{};

    SDL_GetRGBA( _value, _format, nullptr, &l_color.red, &l_color.green,
                 &l_color.blue, &l_color.alpha );

    return ( l_color );
}

// Pixels
/**
 * SDL's pixel formats have the following naming convention:
 *
 * - Names with a list of components and a single bit count, such as RGB24 and
 *   ABGR32, define a platform-independent encoding into bytes in the order
 *   specified. For example, in RGB24 data, each pixel is encoded in 3 bytes
 *   (red, green, blue) in that order, and in ABGR32 data, each pixel is
 *   encoded in 4 bytes alpha, blue, green, red) in that order. Use these
 *   names if the property of a format that is important to you is the order
 *   of the bytes in memory or on disk.
 * - Names with a bit count per component, such as ARGB8888 and XRGB1555, are
 *   "packed" into an appropriately-sized integer in the platform's native
 *   endianness. For example, ARGB8888 is a sequence of 32-bit integers; in
 *   each integer, the most significant bits are alpha, and the least
 *   significant bits are blue. On a little-endian CPU such as x86, the least
 *   significant bits of each integer are arranged first in memory, but on a
 *   big-endian CPU such as s390x, the most significant bits are arranged
 *   first. Use these names if the property of a format that is important to
 *   you is the meaning of each bit position within a native-endianness
 *   integer.
 * - In indexed formats such as INDEX4LSB, each pixel is represented by
 *   encoding an index into the palette into the indicated number of bits,
 *   with multiple pixels packed into each byte if appropriate. In LSB
 *   formats, the first (leftmost) pixel is stored in the least-significant
 *   bits of the byte; in MSB formats, it's stored in the most-significant
 *   bits. INDEX8 does not need LSB/MSB variants, because each pixel exactly
 *   fills one byte.
 *
 * The 32-bit byte-array encodings such as RGBA32 are aliases for the
 * appropriate 8888 encoding for the current platform. For example, RGBA32 is
 * an alias for ABGR8888 on little-endian CPUs like x86, or an alias for
 * RGBA8888 on big-endian CPUs.
 */
using format_t = enum class format {
    unknown = 0,
    index1LSB = 0x11100100u,
    /* format(pixel_t::index1, bitmapOrder_t::bo4321, 0, 1,
       0), */
    index1MSB = 0x11200100u,
    /* format(pixel_t::index1, bitmapOrder_t::1234, 0, 1,
       0), */
    index2LSB = 0x1c100200u,
    /* format(pixel_t::index2, bitmapOrder_t::4321, 0, 2,
       0), */
    index2MSB = 0x1c200200u,
    /* format(pixel_t::index2, bitmapOrder_t::1234, 0, 2,
       0), */
    index4LSB = 0x12100400u,
    /* format(pixel_t::index4, bitmapOrder_t::4321, 0, 4,
       0), */
    index4MSB = 0x12200400u,
    /* format(pixel_t::index4, bitmapOrder_t::1234, 0, 4,
       0), */
    index8 = 0x13000801u,
    /* format(pixel_t::index8, 0, 0, 8, 1), */
    fRGB332 = 0x14110801u,
    /* format(pixel_t::packed8, packedOrder_t::poXRGB,
       packedLayout_t::pl332, 8, 1), */
    fXRGB4444 = 0x15120c02u,
    /* format(pixel_t::packed16, packedOrder_t::poXRGB,
       packedLayout_t::pl4444, 12, 2), */
    fXBGR4444 = 0x15520c02u,
    /* format(pixel_t::packed16, packedOrder_t::poXBGR,
       packedLayout_t::pl4444, 12, 2), */
    fXRGB1555 = 0x15130f02u,
    /* format(pixel_t::packed16, packedOrder_t::poXRGB,
       packedLayout_t::pl1555, 15, 2), */
    fXBGR1555 = 0x15530f02u,
    /* format(pixel_t::packed16, packedOrder_t::poXBGR,
       packedLayout_t::pl1555, 15, 2), */
    fARGB4444 = 0x15321002u,
    /* format(pixel_t::packed16, packedOrder_t::poARGB,
       packedLayout_t::pl4444, 16, 2), */
    fRGBA4444 = 0x15421002u,
    /* format(pixel_t::packed16, packedOrder_t::poRGBA,
       packedLayout_t::pl4444, 16, 2), */
    fABGR4444 = 0x15721002u,
    /* format(pixel_t::packed16, packedOrder_t::poABGR,
       packedLayout_t::pl4444, 16, 2), */
    fBGRA4444 = 0x15821002u,
    /* format(pixel_t::packed16, packedOrder_t::poBGRA,
       packedLayout_t::pl4444, 16, 2), */
    fARGB1555 = 0x15331002u,
    /* format(pixel_t::packed16, packedOrder_t::poARGB,
       packedLayout_t::pl1555, 16, 2), */
    fRGBA5551 = 0x15441002u,
    /* format(pixel_t::packed16, packedOrder_t::poRGBA,
       packedLayout_t::pl5551, 16, 2), */
    fABGR1555 = 0x15731002u,
    /* format(pixel_t::packed16, packedOrder_t::poABGR,
       packedLayout_t::pl1555, 16, 2), */
    fBGRA5551 = 0x15841002u,
    /* format(pixel_t::packed16, packedOrder_t::poBGRA,
       packedLayout_t::pl5551, 16, 2), */
    fRGB565 = 0x15151002u,
    /* format(pixel_t::packed16, packedOrder_t::poXRGB,
       packedLayout_t::pl565, 16, 2), */
    fBGR565 = 0x15551002u,
    /* format(pixel_t::packed16, packedOrder_t::poXBGR,
       packedLayout_t::pl565, 16, 2), */
    fRGB24 = 0x17101803u,
    /* format(pixel_t::arrayu8, arrayOrder_t::aoRGB, 0, 24,
       3), */
    fBGR24 = 0x17401803u,
    /* format(pixel_t::arrayu8, arrayOrder_t::aoBGR, 0, 24,
       3), */
    fXRGB8888 = 0x16161804u,
    /* format(pixel_t::packed32, packedOrder_t::poXRGB,
       packedLayout_t::pl8888, 24, 4), */
    fRGBX8888 = 0x16261804u,
    /* format(pixel_t::packed32, packedOrder_t::poRGBX,
       packedLayout_t::pl8888, 24, 4), */
    fXBGR8888 = 0x16561804u,
    /* format(pixel_t::packed32, packedOrder_t::poXBGR,
       packedLayout_t::pl8888, 24, 4), */
    fBGRX8888 = 0x16661804u,
    /* format(pixel_t::packed32, packedOrder_t::poBGRX,
       packedLayout_t::pl8888, 24, 4), */
    fARGB8888 = 0x16362004u,
    /* format(pixel_t::packed32, packedOrder_t::poARGB,
       packedLayout_t::pl8888, 32, 4), */
    fRGBA8888 = 0x16462004u,
    /* format(pixel_t::packed32, packedOrder_t::poRGBA,
       packedLayout_t::pl8888, 32, 4), */
    fABGR8888 = 0x16762004u,
    /* format(pixel_t::packed32, packedOrder_t::poABGR,
       packedLayout_t::pl8888, 32, 4), */
    fBGRA8888 = 0x16862004u,
    /* format(pixel_t::packed32, packedOrder_t::poBGRA,
       packedLayout_t::pl8888, 32, 4), */
    fXRGB2101010 = 0x16172004u,
    /* format(pixel_t::packed32, packedOrder_t::poXRGB,
       packedLayout_t::pl2101010, 32, 4), */
    fXBGR2101010 = 0x16572004u,
    /* format(pixel_t::packed32, packedOrder_t::poXBGR,
       packedLayout_t::pl2101010, 32, 4), */
    fARGB2101010 = 0x16372004u,
    /* format(pixel_t::packed32, packedOrder_t::poARGB,
       packedLayout_t::pl2101010, 32, 4), */
    fABGR2101010 = 0x16772004u,
    /* format(pixel_t::packed32, packedOrder_t::poABGR,
       packedLayout_t::pl2101010, 32, 4), */
    fRGB48 = 0x18103006u,
    /* format(pixel_t::arrayu16, arrayOrder_t::aoRGB, 0, 48,
       6), */
    fBGR48 = 0x18403006u,
    /* format(pixel_t::arrayu16, arrayOrder_t::aoBGR, 0, 48,
       6), */
    fRGBA64 = 0x18204008u,
    /* format(pixel_t::arrayu16, arrayOrder_t::aoRGBA, 0,
       64, 8), */
    fARGB64 = 0x18304008u,
    /* format(pixel_t::arrayu16, arrayOrder_t::aoARGB, 0,
       64, 8), */
    fBGRA64 = 0x18504008u,
    /* format(pixel_t::arrayu16, arrayOrder_t::aoBGRA, 0,
       64, 8), */
    fABGR64 = 0x18604008u,
    /* format(pixel_t::arrayu16, arrayOrder_t::aoABGR, 0,
       64, 8), */
    fRGB48Float = 0x1a103006u,
    /* format(pixel_t::arrayf16, arrayOrder_t::aoRGB, 0, 48,
       6), */
    fBGR48Float = 0x1a403006u,
    /* format(pixel_t::arrayf16, arrayOrder_t::aoBGR, 0, 48,
       6), */
    fRGBA64Float = 0x1a204008u,
    /* format(pixel_t::arrayf16, arrayOrder_t::aoRGBA, 0,
       64, 8), */
    fARGB64Float = 0x1a304008u,
    /* format(pixel_t::arrayf16, arrayOrder_t::aoARGB, 0,
       64, 8), */
    fBGRA64Float = 0x1a504008u,
    /* format(pixel_t::arrayf16, arrayOrder_t::aoBGRA, 0,
       64, 8), */
    fABGR64Float = 0x1a604008u,
    /* format(pixel_t::arrayf16, arrayOrder_t::aoABGR, 0,
       64, 8), */
    fRGB96Float = 0x1b10600cu,
    /* format(pixel_t::arrayf32, arrayOrder_t::aoRGB, 0, 96,
       12), */
    fBGR96Float = 0x1b40600cu,
    /* format(pixel_t::arrayf32, arrayOrder_t::aoBGR, 0, 96,
       12), */
    fRGBA128Float = 0x1b208010u,
    /* format(pixel_t::arrayf32, arrayOrder_t::aoRGBA, 0,
       128, 16), */
    fARGB128Float = 0x1b308010u,
    /* format(pixel_t::arrayf32, arrayOrder_t::aoARGB, 0,
       128, 16), */
    fBGRA128Float = 0x1b508010u,
    /* format(pixel_t::arrayf32, arrayOrder_t::aoBGRA, 0,
       128, 16), */
    fABGR128Float = 0x1b608010u,
    /* format(pixel_t::arrayf32, arrayOrder_t::aoABGR, 0,
       128, 16), */

    fYV12 = 0x32315659u, /**< Planar mode: Y + V + U  (3 planes) */
    /* formatFourCC('Y', 'V', '1', '2'), */
    fIYUV = 0x56555949u, /**< Planar mode: Y + U + V  (3 planes) */
    /* formatFourCC('I', 'Y', 'U', 'V'), */
    fYUY2 = 0x32595559u, /**< Packed mode: Y0+U0+Y1+V0 (1 plane) */
    /* formatFourCC('Y', 'U', 'Y', '2'), */
    fUYVY = 0x59565955u, /**< Packed mode: U0+Y0+V0+Y1 (1 plane) */
    /* formatFourCC('U', 'Y', 'V', 'Y'), */
    fYVYU = 0x55595659u, /**< Packed mode: Y0+V0+Y1+U0 (1 plane) */
    /* formatFourCC('Y', 'V', 'Y', 'U'), */
    fNV12 = 0x3231564eu, /**< Planar mode: Y + U/V interleaved  (2 planes) */
    /* formatFourCC('N', 'V', '1', '2'), */
    fNV21 = 0x3132564eu, /**< Planar mode: Y + V/U interleaved  (2 planes) */
    /* formatFourCC('N', 'V', '2', '1'), */
    fP010 = 0x30313050u, /**< Planar mode: Y + U/V interleaved  (2 planes) */
    /* formatFourCC('P', '0', '1', '0'), */
    externalOES = 0x2053454fu, /**< Android video texture format */
    /* formatFourCC('O', 'E', 'S', ' ') */

    fMJPG = 0x47504a4du, /**< Motion JPEG */
/* formatFourCC('M', 'J', 'P', 'G') */

// Aliases for RGBA byte arrays of color data, for the current platform
#if ( __BYTE_ORDER == __BIG_ENDIAN )

    fRGBA32 = fRGBA8888,
    fARGB32 = fARGB8888,
    fBGRA32 = fBGRA8888,
    fABGR32 = fABGR8888,
    fRGBX32 = fRGBX8888,
    fXRGB32 = fXRGB8888,
    fBGRX32 = fBGRX8888,
    fXBGR32 = fXBGR8888,

#else

    fRGBA32 = fABGR8888,
    fARGB32 = fBGRA8888,
    fBGRA32 = fARGB8888,
    fABGR32 = fRGBA8888,
    fRGBX32 = fXBGR8888,
    fXRGB32 = fBGRX8888,
    fBGRX32 = fXRGB8888,
    fXBGR32 = fRGBX8888,

#endif
};

using formatUnderlying_t = std::underlying_type_t< format_t >;

using pixel_t = enum class pixel : uint8_t {
    unknown,
    index1,
    index4,
    index8,
    packed8,
    packed16,
    packed32,
    arrayu8,
    arrayu16,
    arrayu32,
    arrayf16,
    arrayf32,
};

using pixelUnderlying_t = std::underlying_type_t< pixel_t >;

// Bitmap pixel order, high bit -> low bit
using bitmapOrder_t = enum class bitmapOrder : uint8_t {
    none,
    bo4321,
    bo1234,
};

using bitmapOrderUnderlying_t = std::underlying_type_t< pixel_t >;

// Packed component order, high bit -> low bit
using packedOrder_t = enum class packedOrder : uint8_t {
    none,
    poXRGB,
    poRGBX,
    poARGB,
    poRGBA,
    poXBGR,
    poBGRX,
    poABGR,
    poBGRA,
};

using packedOrderUnderlying_t = std::underlying_type_t< pixel_t >;

// Array component order, low byte -> high byte
using arrayOrder_t = enum class arrayOrder : uint8_t {
    none,
    aoRGB,
    aoRGBA,
    aoARGB,
    aoBGR,
    aoBGRA,
    aoABGR,
};

using arrayOrderUnderlying_t = std::underlying_type_t< pixel_t >;

using order_t =
    std::variant< std::monostate, bitmapOrder_t, packedOrder_t, arrayOrder_t >;

[[nodiscard]] constexpr auto order( format_t _format ) -> order_t {
    const uint8_t l_order =
        ( ( static_cast< formatUnderlying_t >( _format ) >> 20 ) & 0xF );

    // Priorities
    {
        if ( l_order <= static_cast< uint8_t >( arrayOrder_t::aoABGR ) ) {
            switch ( static_cast< arrayOrder_t >( l_order ) ) {
                case ( arrayOrder_t::aoARGB ): {
                    return ( arrayOrder_t::aoARGB );
                }

                case ( arrayOrder_t::aoRGBA ): {
                    return ( arrayOrder_t::aoRGBA );
                }

                case ( arrayOrder_t::aoABGR ): {
                    return ( arrayOrder_t::aoABGR );
                }

                case ( arrayOrder_t::aoBGRA ): {
                    return ( arrayOrder_t::aoBGRA );
                }

                default: {
                }
            }

        } else if ( l_order <=
                    static_cast< uint8_t >( packedOrder_t::poBGRA ) ) {
            switch ( static_cast< packedOrder_t >( l_order ) ) {
                case ( packedOrder_t::poARGB ): {
                    return ( packedOrder_t::poARGB );
                }

                case ( packedOrder_t::poRGBA ): {
                    return ( packedOrder_t::poRGBA );
                }

                case ( packedOrder_t::poABGR ): {
                    return ( packedOrder_t::poABGR );
                }

                case ( packedOrder_t::poBGRA ): {
                    return ( packedOrder_t::poBGRA );
                }

                default: {
                }
            }
        }
    }

    if ( l_order <= static_cast< uint8_t >( packedOrder_t::poBGRA ) ) {
        return ( static_cast< packedOrder_t >( l_order ) );
    }

    return {};
}

using packedLayout_t = enum class packedLayout : uint8_t {
    none,
    pl332,
    pl4444,
    pl1555,
    pl5551,
    pl565,
    pl8888,
    pl2101010,
    pl1010102,
};

using packedLayoutUnderlying_t = std::underlying_type_t< packedLayout_t >;

// For example, defining format_t::YV12 looks like this:
// formatFourCC('Y', 'V', '1', '2')
//
// Returns a format value in the style of format_t
// TODO: Maybe reduntant
[[nodiscard]] constexpr auto formatFourCC( char _a, char _b, char _c, char _d )
    -> uint32_t {
    return ( std::bit_cast< uint32_t >( std::to_array( {
        _a,
        _b,
        _c,
        _d,
    } ) ) );
}

// For example, defining format_t::RGBA8888 looks like this:
// format(pixel_t::packed32, packedOrder_t::poRGBA,
// packedLayout_t::pl8888, 32, 4)
//
// Returns a format value in the style of format_t
[[nodiscard]] constexpr auto format( pixel_t _type,
                                     order_t _order,
                                     packedLayout_t _layout,
                                     uint8_t _bitsAmount,
                                     uint8_t _bytesAmount = 0 ) -> format_t {
    if ( !_bytesAmount ) {
        _bytesAmount = stdfunc::bitsToBytes( _bitsAmount );
    }

    return ( _order.visit( stdfunc::overloadedVisit{
        []( std::monostate ) constexpr -> format_t { return {}; },
        [ & ]( auto&& _order ) constexpr -> format_t {
            return ( static_cast< format_t >(
                ( 1 << 28 ) |
                ( static_cast< pixelUnderlying_t >( _type ) << 24 ) |
                ( static_cast< uint8_t >( _order ) << 20 ) |
                ( static_cast< packedLayoutUnderlying_t >( _layout ) << 16 ) |
                ( _bitsAmount << 8 ) | ( _bytesAmount << 0 ) ) );
        },
    } ) );
}

// This is generally not needed directly by an app, which should use
// specific tests, like isFourCC, instead
[[nodiscard]] constexpr auto flag( format_t _format ) -> uint8_t {
    return ( ( static_cast< formatUnderlying_t >( _format ) >> 28 ) & 0xF );
}

[[nodiscard]] constexpr auto type( format_t _format ) -> pixel_t {
    return ( static_cast< pixel_t >(
        ( static_cast< formatUnderlying_t >( _format ) >> 24 ) & 0xF ) );
}

[[nodiscard]] constexpr auto layout( format_t _format ) -> packedLayout_t {
    return ( static_cast< packedLayout_t >(
        ( static_cast< formatUnderlying_t >( _format ) >> 16 ) & 0xF ) );
}

// This covers custom and other unusual formats.
[[nodiscard]] constexpr auto isFourCC( format_t _format ) -> bool {
    // The flag is set to 1 because 0x1? is not in the printable ASCII range
    return ( flag( _format ) != 1 );
}

// FourCC formats will report zero here, as it rarely makes sense to measure
// them per-pixel
[[nodiscard]] constexpr auto bits( format_t _format ) -> uint8_t {
    return ( isFourCC( _format )
                 ? ( 0 )
                 : ( ( static_cast< formatUnderlying_t >( _format ) >> 8 ) &
                     0xFF ) );
}

// FourCC formats do their best here, but many of them don't have a
// *meaningful* measurement of bytes per pixel
[[nodiscard]] constexpr auto bytes( format_t _format ) -> uint8_t {
    return ( isFourCC( _format ) ? ( ( ( _format == format_t::fYUY2 ) ||
                                       ( _format == format_t::fUYVY ) ||
                                       ( _format == format_t::fYVYU ) ||
                                       ( _format == format_t::fP010 ) )
                                         ? ( 2 )
                                         : ( 1 ) )
                                 : ( bits( _format ) ) );
}

[[nodiscard]] constexpr auto isIndexed( format_t _format ) -> bool {
    const pixel_t l_type = type( _format );

    const bool l_isIndexed =
        ( ( l_type == pixel_t::index1 ) || ( l_type == pixel_t::index4 ) ||
          ( l_type == pixel_t::index8 ) );

    return ( !isFourCC( _format ) && l_isIndexed );
}

[[nodiscard]] constexpr auto isPacked( format_t _format ) -> bool {
    const pixel_t l_type = type( _format );

    return ( !isFourCC( _format ) && ( ( l_type == pixel_t::packed8 ) ||
                                       ( l_type == pixel_t::packed16 ) ||
                                       ( l_type == pixel_t::packed32 ) ) );
}

[[nodiscard]] constexpr auto isArray( format_t _format ) -> bool {
    const pixel_t l_type = type( _format );

    return ( !isFourCC( _format ) && ( ( l_type == pixel_t::arrayu8 ) ||
                                       ( l_type == pixel_t::arrayu16 ) ||
                                       ( l_type == pixel_t::arrayu32 ) ||
                                       ( l_type == pixel_t::arrayf16 ) ||
                                       ( l_type == pixel_t::arrayf32 ) ) );
}

[[nodiscard]] constexpr auto is10bit( format_t _format ) -> bool {
    const pixel_t l_type = type( _format );

    return ( !isFourCC( _format ) &&
             ( ( l_type == pixel_t::packed32 ) &&
               ( layout( _format ) == packedLayout_t::pl2101010 ) ) );
}

[[nodiscard]] constexpr auto isFloat( format_t _format ) -> bool {
    const pixel_t l_type = type( _format );

    return ( !isFourCC( _format ) && ( ( l_type == pixel_t::arrayf16 ) ||
                                       ( l_type == pixel_t::arrayf32 ) ) );
}

[[nodiscard]] constexpr auto hasAlpha( format_t _format ) -> bool {
    const order_t l_order = order( _format );

    return ( l_order.visit( stdfunc::overloadedVisit{
        []( packedOrder_t _order ) constexpr -> bool {
            return ( ( _order == packedOrder_t::poARGB ) ||
                     ( _order == packedOrder_t::poRGBA ) ||
                     ( _order == packedOrder_t::poABGR ) ||
                     ( _order == packedOrder_t::poBGRA ) );
        },
        []( arrayOrder_t _order ) constexpr -> bool {
            return ( ( _order == arrayOrder_t::aoARGB ) ||
                     ( _order == arrayOrder_t::aoRGBA ) ||
                     ( _order == arrayOrder_t::aoABGR ) ||
                     ( _order == arrayOrder_t::aoBGRA ) );
        },
        []( auto&& ) constexpr -> bool { return ( false ); },
    } ) );
}

using color_t = enum class color : uint8_t {
    unknown = 0,
    cRGB = 1,
    cYCBCR = 2,
};

using colorUnderlying_t = std::underlying_type_t< color_t >;

using colorRange_t = enum class colorRange : uint8_t {
    unknown = 0,
    limited = 1, /**< Narrow range, e.g. 16-235 for 8-bit RGB
                                    and luma, and 16-240 for 8-bit chroma */
    full = 2,    /**< Full range, e.g. 0-255 for 8-bit RGB and luma,
                                   and 1-255 for 8-bit chroma */
};

using colorRangeUnderlying_t = std::underlying_type_t< colorRange_t >;

using colorPrimaries_t = enum class colorPrimaries : uint8_t {
    unknown = 0,
    cpBT709 = 1, /**< ITU-R BT.709-6 */
    unspecified = 2,
    cpBT470M = 4,    /**< ITU-R BT.470-6 System M */
    cpBT470BG = 5,   /**< ITU-R BT.470-6 System B, G / ITU-R BT.601-7 625 */
    cpBT601 = 6,     /**< ITU-R BT.601-7 525, SMPTE 170M */
    cpSMPTE240 = 7,  /**< SMPTE 240M, functionally the same as
                                          BT601 */
    genericFilm = 8, /**< Generic film (color filters using Illuminant C) */
    cpBT2020 = 9,    /**< ITU-R BT.2020-2 / ITU-R BT.2100-0 */
    cpXYZ = 10,      /**< SMPTE ST 428-1 */
    cpSMPTE431 = 11, /**< SMPTE RP 431-2 */
    cpSMPTE432 = 12, /**< SMPTE EG 432-1 / DCI P3 */
    cpEBU3213 = 22,  /**< EBU Tech. 3213-E */
    custom = 31,
};

using colorPrimariesUnderlying_t = std::underlying_type_t< colorPrimaries_t >;

using transferCharacteristics_t = enum class transferCharacteristics : uint8_t {
    unknown = 0,
    tcBT709 = 1, /**< Rec. ITU-R BT.709-6 / ITU-R BT1361 */
    unspecified = 2,
    gamma22 = 4, /**< ITU-R BT.470-6 System M / ITU-R BT1700 625 PAL & SECAM */
    gamma28 = 5, /**< ITU-R BT.470-6 System B, G */
    tcBT601 = 6, /**< SMPTE ST 170M / ITU-R BT.601-7 525 or 625 */
    tcSMPTE240 = 7, /**< SMPTE ST 240M */
    linear = 8,
    log100 = 9,
    log100Sqrt10 = 10,
    tcIEC61966 = 11,    /**< IEC 61966-2-4 */
    tcBT1361 = 12,      /**< ITU-R BT1361 Extended Colour Gamut */
    tcSRGB = 13,        /**< IEC 61966-2-1 (sRGB or sYCC) */
    tcBT2020Bit10 = 14, /**< ITU-R BT2020 for 10-bit system */
    tcBT2020Bit12 = 15, /**< ITU-R BT2020 for 12-bit system */
    tcPQ = 16,       /**< SMPTE ST 2084 for 10-, 12-, 14- and 16-bit systems */
    tcSMPTE428 = 17, /**< SMPTE ST 428-1 */
    tcHLG = 18,      /**< ARIB STD-B67, known as "hybrid log-gamma" (HLG) */
    custom = 31,
};

using transferCharacteristicsUnderlying_t =
    std::underlying_type_t< transferCharacteristics_t >;

using matrixCoefficients_t = enum class matrixCoefficients : uint8_t {
    identity = 0,
    mcBT709 = 1, /**< ITU-R BT.709-6 */
    unspecified = 2,
    mcFCC = 4,      /**< US FCC Title 47 */
    mcBT470BG = 5,  /**< ITU-R BT.470-6 System B, G / ITU-R BT.601-7 625,
                       functionally  the same as BT601 */
    mcBT601 = 6,    /**< ITU-R BT.601-7 525 */
    mcSMPTE240 = 7, /**< SMPTE 240M */
    mcYCGCO = 8,
    mcBT2020NCL = 9,  /**< ITU-R BT.2020-2 non-constant luminance */
    mcBT2020CL = 10,  /**< ITU-R BT.2020-2 constant luminance */
    mcSMPTE2085 = 11, /**< SMPTE ST 2085 */
    chromaDerivedNCL = 12,
    chromaDerivedCL = 13,
    mcICTCP = 14, /**< ITU-R BT.2100-0 ICTCP */
    custom = 31,
};

using matrixCoefficientsUnderlying_t =
    std::underlying_type_t< matrixCoefficients_t >;

using chromaLocation_t = enum class chromaLocation : uint8_t {
    none = 0,   /**< RGB, no chroma sampling */
    left = 1,   /**< In MPEG-2, MPEG-4, and AVC, Cb and Cr are taken on midpoint
                   of the left-edge of the 2x2 square. In other words, they have
                   the same horizontal location as the top-left pixel, but is
                   shifted one-half pixel down vertically. */
    center = 2, /**< In JPEG/JFIF, H.261, and MPEG-1, Cb and Cr are taken at the
                   center of the 2x2 square. In other words, they are offset
                   one-half pixel to the right and one-half pixel down compared
                   to the top-left pixel. */
    topLeft = 3, /**< In HEVC for BT.2020 and BT.2100 content (in particular on
                    Blu-rays), Cb and Cr are sampled at the same location as the
                    group's top-left Y pixel ("co-sited", "co-located"). */
};

using chromaLocationUnderlying_t = std::underlying_type_t< chromaLocation_t >;

// Since similar colorspaces may vary in their details (matrix, transfer
// function, etc.), this is not an exhaustive list, but rather a
// representative sample of the kinds of colorspaces supported in SDL.
using colorspace_t = enum class colorspace : uint32_t {
    unknown = 0,

    /* sRGB is a gamma corrected colorspace, and the default colorspace for SDL
       rendering and 8-bit RGB surfaces */
    csSRGB = 0x120005a0u, /**< Equivalent to
                             DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709 */
    /* colorspace(color_t::cRGB,
                             colorRange_t::full,
                             colorPrimaries_t::cpBT709,
                             transferCharacteristics_t::tcSRGB,
                             matrixCoefficients_t::mcIDENTITY,
                             chromaLocation_t::none), */

    /* This is a linear colorspace and the default colorspace for floating point
       surfaces. On Windows this is the scRGB colorspace, and on Apple platforms
       this is kCGColorSpaceExtendedLinearSRGB for EDR content */
    csSRGBLinear = 0x12000500u, /**< Equivalent to
                                   DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709  */
    /* colorspace(color_t::cRGB,
                             colorRange_t::full,
                             colorPrimaries_t::cpBT709,
                             transferCharacteristics_t::tcLINEAR,
                             matrixCoefficients_t::mcIDENTITY,
                             chromaLocation_t::none), */

    /* HDR10 is a non-linear HDR colorspace and the default colorspace for
       10-bit surfaces */
    csHDR10 = 0x12002600u, /**< Equivalent to
                              DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020  */
    /* colorspace(color_t::cRGB,
                             colorRange_t::full,
                             colorPrimaries_t::cpBT2020,
                             transferCharacteristics_t::tcPQ,
                             matrixCoefficients_t::mcIDENTITY,
                             chromaLocation_t::none), */

    csJPEG = 0x220004c6u, /**< Equivalent to
                             DXGI_COLOR_SPACE_YCBCR_FULL_G22_NONE_P709_X601 */
    /* colorspace(color_t::cYCBCR,
                             colorRange_t::full,
                             colorPrimaries_t::cpBT709,
                             transferCharacteristics_t::tcBT601,
                             matrixCoefficients_t::mcBT601,
                             chromaLocation_t::none), */

    csBT601Limited =
        0x211018c6u, /**< Equivalent to
                        DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P601 */
    /* colorspace(color_t::cYCBCR,
                             colorRange_t::LIMITED,
                             colorPrimaries_t::cpBT601,
                             transferCharacteristics_t::tcBT601,
                             matrixCoefficients_t::mcBT601,
                             chromaLocation_t::left), */

    csBT601Full = 0x221018c6u, /**< Equivalent to
                                  DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P601 */
    /* colorspace(color_t::cYCBCR,
                             colorRange_t::full,
                             colorPrimaries_t::cpBT601,
                             transferCharacteristics_t::tcBT601,
                             matrixCoefficients_t::mcBT601,
                             chromaLocation_t::left), */

    csBT709Limited =
        0x21100421u, /**< Equivalent to
                        DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P709 */
    /* colorspace(color_t::cYCBCR,
                             colorRange_t::LIMITED,
                             colorPrimaries_t::cpBT709,
                             transferCharacteristics_t::tcBT709,
                             matrixCoefficients_t::mcBT709,
                             chromaLocation_t::left), */

    csBT709Full = 0x22100421u, /**< Equivalent to
                                  DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P709 */
    /* colorspace(color_t::cYCBCR,
                             colorRange_t::full,
                             colorPrimaries_t::cpBT709,
                             transferCharacteristics_t::tcBT709,
                             matrixCoefficients_t::mcBT709,
                             chromaLocation_t::left), */

    csBT2020Limited =
        0x21102609u, /**< Equivalent to
                        DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P2020 */
    /* colorspace(color_t::cYCBCR,
                             colorRange_t::LIMITED,
                             colorPrimaries_t::cpBT2020,
                             transferCharacteristics_t::tcPQ,
                             matrixCoefficients_t::mcBT2020_NCL,
                             chromaLocation_t::left), */

    csBT2020Full = 0x22102609u, /**< Equivalent to
                                   DXGI_COLOR_SPACE_YCBCR_FULL_G22_LEFT_P2020 */
    /* colorspace(color_t::cYCBCR,
                             colorRange_t::full,
                             colorPrimaries_t::cpBT2020,
                             transferCharacteristics_t::tcPQ,
                             matrixCoefficients_t::mcBT2020_NCL,
                             chromaLocation_t::left), */

    csRGBDefault = csSRGB, /**< The default colorspace for RGB surfaces if no
                              colorspace is specified */
    csYUVDefault = csJPEG, /**< The default colorspace for YUV surfaces if no
                             colorspace is specified */
};

using colorspaceUnderlying_t = std::underlying_type_t< colorspace_t >;

// For example, defining colorspace_t::csSRGB looks like this:
// colorspace(color_t::cRGB, colorRange_t::full, colorPrimaries_t::cpBT709,
// transferCharacteristics_t::tcSRGB, matrixCoefficients_t::mcIDENTITY,
// chromaLocation_t::none)
[[nodiscard]] constexpr auto colorspace( color_t _type,
                                         colorRange_t _range,
                                         colorPrimaries_t _primaries,
                                         transferCharacteristics_t _transfer,
                                         matrixCoefficients_t _matrix,
                                         chromaLocation_t _chroma )
    -> colorspace_t {
    // TODO: Improve
    return ( static_cast< colorspace_t >(
        ( static_cast< uint32_t >( _type ) << 28 ) |
        ( static_cast< uint32_t >( _range ) << 24 ) |
        ( static_cast< uint32_t >( _chroma ) << 20 ) |
        ( static_cast< uint32_t >( _primaries ) << 10 ) |
        ( static_cast< uint32_t >( _transfer ) << 5 ) |
        ( static_cast< uint32_t >( _matrix ) << 0 ) ) );
}

[[nodiscard]] constexpr auto type( colorspace_t _colorspace ) -> color_t {
    return ( static_cast< color_t >(
        ( static_cast< colorspaceUnderlying_t >( _colorspace ) >> 28 ) &
        0x0F ) );
}

[[nodiscard]] constexpr auto range( colorspace_t _colorspace ) -> colorRange_t {
    return ( static_cast< colorRange_t >(
        ( static_cast< colorspaceUnderlying_t >( _colorspace ) >> 24 ) &
        0x0F ) );
}

[[nodiscard]] constexpr auto chroma( colorspace_t _colorspace )
    -> chromaLocation_t {
    return ( static_cast< chromaLocation_t >(
        ( static_cast< colorspaceUnderlying_t >( _colorspace ) >> 20 ) &
        0x0F ) );
}

[[nodiscard]] constexpr auto primaries( colorspace_t _colorspace )
    -> colorPrimaries_t {
    return ( static_cast< colorPrimaries_t >(
        ( static_cast< colorspaceUnderlying_t >( _colorspace ) >> 10 ) &
        0x1F ) );
}

[[nodiscard]] constexpr auto transfer( colorspace_t _colorspace )
    -> transferCharacteristics_t {
    return ( static_cast< transferCharacteristics_t >(
        ( static_cast< colorspaceUnderlying_t >( _colorspace ) >> 5 ) &
        0x1F ) );
}

[[nodiscard]] constexpr auto matrix( colorspace_t _colorspace )
    -> matrixCoefficients_t {
    return ( static_cast< matrixCoefficients_t >(
        static_cast< colorspaceUnderlying_t >( _colorspace ) & 0x1F ) );
}

// Returns true if BT601 or BT470BG, false otherwise.
[[nodiscard]] constexpr auto isMatrixBT601( colorspace_t _colorspace ) -> bool {
    matrixCoefficients_t l_matrix = matrix( _colorspace );

    return ( l_matrix == matrixCoefficients_t::mcBT601 ||
             l_matrix == matrixCoefficients_t::mcBT470BG );
}

// Returns true if BT709, false otherwise.
[[nodiscard]] constexpr auto isMatrixBT709( colorspace_t _colorspace ) -> bool {
    return ( matrix( _colorspace ) == matrixCoefficients_t::mcBT709 );
}

// Returns true if BT2020_NCL, false otherwise.
[[nodiscard]] constexpr auto isMatrixBT2020NCL( colorspace_t _colorspace )
    -> bool {
    return ( matrix( _colorspace ) == matrixCoefficients_t::mcBT2020NCL );
}

// Returns true if limited range, false otherwise.
[[nodiscard]] constexpr auto isLimitedRange( colorspace_t _colorspace )
    -> bool {
    return ( range( _colorspace ) != colorRange_t::full );
}

// Returns true if full range, false otherwise.
[[nodiscard]] constexpr auto isFullRange( colorspace_t _colorspace ) -> bool {
    return ( range( _colorspace ) == colorRange_t::full );
}

// Get the human readable name of a pixel format.
[[nodiscard]] constexpr auto name( format_t _format ) -> std::string_view {
    std::string_view l_returnValue;

    switch ( _format ) {
        case ( format_t::index1LSB ): {
            l_returnValue = ( "index1LSB" );
        }

        case ( format_t::index1MSB ): {
            l_returnValue = ( "index1MSB" );
        }

        case ( format_t::index2LSB ): {
            l_returnValue = ( "index2LSB" );
        }

        case ( format_t::index2MSB ): {
            l_returnValue = ( "index2MSB" );
        }

        case ( format_t::index4LSB ): {
            l_returnValue = ( "index4LSB" );
        }

        case ( format_t::index4MSB ): {
            l_returnValue = ( "index4MSB" );
        }

        case ( format_t::index8 ): {
            l_returnValue = ( "index8" );
        }

        case ( format_t::fRGB332 ): {
            l_returnValue = ( "fRGB332" );
        }

        case ( format_t::fXRGB4444 ): {
            l_returnValue = ( "fXRGB4444" );
        }

        case ( format_t::fXBGR4444 ): {
            l_returnValue = ( "fXBGR4444" );
        }

        case ( format_t::fXRGB1555 ): {
            l_returnValue = ( "fXRGB1555" );
        }

        case ( format_t::fXBGR1555 ): {
            l_returnValue = ( "fXBGR1555" );
        }

        case ( format_t::fARGB4444 ): {
            l_returnValue = ( "fARGB4444" );
        }

        case ( format_t::fRGBA4444 ): {
            l_returnValue = ( "fRGBA4444" );
        }

        case ( format_t::fABGR4444 ): {
            l_returnValue = ( "fABGR4444" );
        }

        case ( format_t::fBGRA4444 ): {
            l_returnValue = ( "fBGRA4444" );
        }

        case ( format_t::fARGB1555 ): {
            l_returnValue = ( "fARGB1555" );
        }

        case ( format_t::fRGBA5551 ): {
            l_returnValue = ( "fRGBA5551" );
        }

        case ( format_t::fABGR1555 ): {
            l_returnValue = ( "fABGR1555" );
        }

        case ( format_t::fBGRA5551 ): {
            l_returnValue = ( "fBGRA5551" );
        }

        case ( format_t::fRGB565 ): {
            l_returnValue = ( "fRGB565" );
        }

        case ( format_t::fBGR565 ): {
            l_returnValue = ( "fBGR565" );
        }

        case ( format_t::fRGB24 ): {
            l_returnValue = ( "fRGB24" );
        }

        case ( format_t::fBGR24 ): {
            l_returnValue = ( "fBGR24" );
        }

        case ( format_t::fXRGB8888 ): {
            l_returnValue = ( "fXRGB8888" );
        }

        case ( format_t::fRGBX8888 ): {
            l_returnValue = ( "fRGBX8888" );
        }

        case ( format_t::fXBGR8888 ): {
            l_returnValue = ( "fXBGR8888" );
        }

        case ( format_t::fBGRX8888 ): {
            l_returnValue = ( "fBGRX8888" );
        }

        case ( format_t::fARGB8888 ): {
            l_returnValue = ( "fARGB8888" );
        }

        case ( format_t::fRGBA8888 ): {
            l_returnValue = ( "fRGBA8888" );
        }

        case ( format_t::fABGR8888 ): {
            l_returnValue = ( "fABGR8888" );
        }

        case ( format_t::fBGRA8888 ): {
            l_returnValue = ( "fBGRA8888" );
        }

        case ( format_t::fXRGB2101010 ): {
            l_returnValue = ( "fXRGB2101010" );
        }

        case ( format_t::fXBGR2101010 ): {
            l_returnValue = ( "fXBGR2101010" );
        }

        case ( format_t::fARGB2101010 ): {
            l_returnValue = ( "fARGB2101010" );
        }

        case ( format_t::fABGR2101010 ): {
            l_returnValue = ( "fABGR2101010" );
        }

        case ( format_t::fRGB48 ): {
            l_returnValue = ( "fRGB48" );
        }

        case ( format_t::fBGR48 ): {
            l_returnValue = ( "fBGR48" );
        }

        case ( format_t::fRGBA64 ): {
            l_returnValue = ( "fRGBA64" );
        }

        case ( format_t::fARGB64 ): {
            l_returnValue = ( "fARGB64" );
        }

        case ( format_t::fBGRA64 ): {
            l_returnValue = ( "fBGRA64" );
        }

        case ( format_t::fABGR64 ): {
            l_returnValue = ( "fABGR64" );
        }

        case ( format_t::fRGB48Float ): {
            l_returnValue = ( "fRGB48Float" );
        }

        case ( format_t::fBGR48Float ): {
            l_returnValue = ( "fBGR48Float" );
        }

        case ( format_t::fRGBA64Float ): {
            l_returnValue = ( "fRGBA64Float" );
        }

        case ( format_t::fARGB64Float ): {
            l_returnValue = ( "fARGB64Float" );
        }

        case ( format_t::fBGRA64Float ): {
            l_returnValue = ( "fBGRA64Float" );
        }

        case ( format_t::fABGR64Float ): {
            l_returnValue = ( "fABGR64Float" );
        }

        case ( format_t::fRGB96Float ): {
            l_returnValue = ( "fRGB96Float" );
        }

        case ( format_t::fBGR96Float ): {
            l_returnValue = ( "fBGR96Float" );
        }

        case ( format_t::fRGBA128Float ): {
            l_returnValue = ( "fRGBA128Float" );
        }

        case ( format_t::fARGB128Float ): {
            l_returnValue = ( "fARGB128Float" );
        }

        case ( format_t::fBGRA128Float ): {
            l_returnValue = ( "fBGRA128Float" );
        }

        case ( format_t::fABGR128Float ): {
            l_returnValue = ( "fABGR128Float" );
        }

        case ( format_t::fYV12 ): {
            l_returnValue = ( "fYV12" );
        }

        case ( format_t::fIYUV ): {
            l_returnValue = ( "fIYUV" );
        }

        case ( format_t::fYUY2 ): {
            l_returnValue = ( "fYUY2" );
        }

        case ( format_t::fUYVY ): {
            l_returnValue = ( "fUYVY" );
        }

        case ( format_t::fYVYU ): {
            l_returnValue = ( "fYVYU" );
        }

        case ( format_t::fNV12 ): {
            l_returnValue = ( "fNV12" );
        }

        case ( format_t::fNV21 ): {
            l_returnValue = ( "fNV21" );
        }

        case ( format_t::fP010 ): {
            l_returnValue = ( "fP010" );
        }

        case ( format_t::externalOES ): {
            l_returnValue = ( "externalOES" );
        }

        case ( format_t::fMJPG ): {
            l_returnValue = ( "fMJPG" );
        }

        default: {
            l_returnValue = ( "unknown" );
        }
    }

    return ( l_returnValue );
}

using mask_t = struct mask {
    uint32_t red;
    uint32_t green;
    uint32_t blue;
    uint32_t alpha;

    size_t bitsPerPixel;
};

// Convert one of the enumerated pixel formats to a BPP value and RGBA masks.
[[nodiscard]] constexpr auto masks( format_t _format )
    -> std::optional< mask_t > {
    std::optional< mask_t > l_returnValue = std::nullopt;
    mask_t l_mask{};

    std::array< uint32_t, 4 > l_masks{};

    do {
        // Partial support for SDL_Surface with FOURCC
        if ( isFourCC( _format ) ) {
            // Not a _format that uses masks however, some of these are packed
            // formats, and can legit declare bits-per-pixel!
            switch ( _format ) {
                case format_t::fYUY2:
                case format_t::fUYVY:
                case format_t::fYVYU: {
                    l_mask.bitsPerPixel = 32;

                    break;
                }

                default: {
                    // Oh well
                    // TODO: Maybe reduntant
                    l_mask.bitsPerPixel = 0;
                }
            }

            break;
        }

        // Initialize the values here
        if ( bytes( _format ) <= 2 ) {
            l_mask.bitsPerPixel = bits( _format );

        } else {
            l_mask.bitsPerPixel = ( bytes( _format ) * 8UZ );
        }

        if ( _format == format_t::fRGB24 ) {
#if __BYTE_ORDER == __BIG_ENDIAN

            l_mask.red = 0x00FF0000;
            l_mask.green = 0x0000FF00;
            l_mask.blue = 0x000000FF;

#else

            l_mask.red = 0x000000FF;
            l_mask.green = 0x0000FF00;
            l_mask.blue = 0x00FF0000;

#endif

            break;

        } else if ( _format == format_t::fBGR24 ) {
#if __BYTE_ORDER == __BIG_ENDIAN

            l_mask.red = 0x000000FF;
            l_mask.green = 0x0000FF00;
            l_mask.blue = 0x00FF0000;

#else

            l_mask.red = 0x00FF0000;
            l_mask.green = 0x0000FF00;
            l_mask.blue = 0x000000FF;

#endif

            break;
        }

        // TODO: Write comment
        // Check format
        {
            const pixel_t l_type = type( _format );

            if ( ( l_type != pixel_t::packed8 ) &&
                 ( l_type != pixel_t::packed16 ) &&
                 ( l_type != pixel_t::packed32 ) ) {
                // Not a _format that uses masks
                break;
            }
        }

        switch ( layout( _format ) ) {
            case ( packedLayout_t::pl332 ): {
                l_masks = {
                    0x00000000,
                    0x000000E0,
                    0x0000001C,
                    0x00000003,
                };

                break;
            }

            case ( packedLayout_t::pl4444 ): {
                l_masks = {
                    0x0000F000,
                    0x00000F00,
                    0x000000F0,
                    0x0000000F,
                };

                break;
            }

            case ( packedLayout_t::pl1555 ): {
                l_masks = {
                    0x00008000,
                    0x00007C00,
                    0x000003E0,
                    0x0000001F,
                };

                break;
            }

            case ( packedLayout_t::pl5551 ): {
                l_masks = {
                    0x0000F800,
                    0x000007C0,
                    0x0000003E,
                    0x00000001,
                };

                break;
            }

            case ( packedLayout_t::pl565 ): {
                l_masks = {
                    0x00000000,
                    0x0000F800,
                    0x000007E0,
                    0x0000001F,
                };

                break;
            }

            case ( packedLayout_t::pl8888 ): {
                l_masks = {
                    0xFF000000,
                    0x00FF0000,
                    0x0000FF00,
                    0x000000FF,
                };

                break;
            }

            case ( packedLayout_t::pl2101010 ): {
                l_masks = {
                    0xC0000000,
                    0x3FF00000,
                    0x000FFC00,
                    0x000003FF,
                };

                break;
            }

            case ( packedLayout_t::pl1010102 ): {
                l_masks = {
                    0xFFC00000,
                    0x003FF000,
                    0x00000FFC,
                    0x00000003,
                };

                break;
            }

            default: {
                // FIX: Remove
                return ( std::nullopt );
            }
        }

        l_returnValue = order( _format ).visit( stdfunc::overloadedVisit{
            [ & ]( packedOrder_t _packedOrder ) constexpr
                -> std::optional< mask_t > {
                switch ( _packedOrder ) {
                    case ( packedOrder_t::poXRGB ): {
                        l_mask.red = l_masks[ 1 ];
                        l_mask.green = l_masks[ 2 ];
                        l_mask.blue = l_masks[ 3 ];

                        break;
                    }

                    case ( packedOrder_t::poRGBX ): {
                        l_mask.red = l_masks[ 0 ];
                        l_mask.green = l_masks[ 1 ];
                        l_mask.blue = l_masks[ 2 ];

                        break;
                    }

                    case ( packedOrder_t::poARGB ): {
                        l_mask.alpha = l_masks[ 0 ];
                        l_mask.red = l_masks[ 1 ];
                        l_mask.green = l_masks[ 2 ];
                        l_mask.blue = l_masks[ 3 ];

                        break;
                    }

                    case ( packedOrder_t::poRGBA ): {
                        l_mask.red = l_masks[ 0 ];
                        l_mask.green = l_masks[ 1 ];
                        l_mask.blue = l_masks[ 2 ];
                        l_mask.alpha = l_masks[ 3 ];

                        break;
                    }

                    case ( packedOrder_t::poXBGR ): {
                        l_mask.blue = l_masks[ 1 ];
                        l_mask.green = l_masks[ 2 ];
                        l_mask.red = l_masks[ 3 ];

                        break;
                    }

                    case ( packedOrder_t::poBGRX ): {
                        l_mask.blue = l_masks[ 0 ];
                        l_mask.green = l_masks[ 1 ];
                        l_mask.red = l_masks[ 2 ];

                        break;
                    }

                    case ( packedOrder_t::poBGRA ): {
                        l_mask.blue = l_masks[ 0 ];
                        l_mask.green = l_masks[ 1 ];
                        l_mask.red = l_masks[ 2 ];
                        l_mask.alpha = l_masks[ 3 ];

                        break;
                    }

                    case ( packedOrder_t::poABGR ): {
                        l_mask.alpha = l_masks[ 0 ];
                        l_mask.blue = l_masks[ 1 ];
                        l_mask.green = l_masks[ 2 ];
                        l_mask.red = l_masks[ 3 ];

                        break;
                    }

                    default: {
                        return ( std::nullopt );
                    }
                }

                return ( l_mask );
            },
            []( auto&& ) constexpr -> std::optional< mask_t > {
                return ( std::nullopt );
            },
        } );

    } while ( false );

    return ( l_returnValue );
}

// Convert a BPP value and RGBA masks to an enumerated pixel format.
[[nodiscard]] constexpr auto formatForMasks( mask_t _mask ) -> format_t {
    switch ( _mask.bitsPerPixel ) {
        case 1: {
            // SDL defaults to MSB ordering
            return ( format_t::index1MSB );
        }

        case 2: {
            // SDL defaults to MSB ordering
            return ( format_t::index2MSB );
        }

        case 4: {
            // SDL defaults to MSB ordering
            return ( format_t::index4MSB );
        }

        case 8: {
            if ( ( _mask.red == 0xE0 ) && ( _mask.green == 0x1C ) &&
                 ( _mask.blue == 0x03 ) && ( _mask.alpha == 0x00 ) ) {
                return ( format_t::fRGB332 );
            }

            return ( format_t::index8 );
        }

        case 12: {
            if ( _mask.red == 0 ) {
                return ( format_t::fXRGB4444 );
            }

            if ( ( _mask.red == 0x0F00 ) && ( _mask.green == 0x00F0 ) &&
                 ( _mask.blue == 0x000F ) && ( _mask.alpha == 0x0000 ) ) {
                return ( format_t::fXRGB4444 );
            }

            if ( ( _mask.red == 0x000F ) && ( _mask.green == 0x00F0 ) &&
                 ( _mask.blue == 0x0F00 ) && ( _mask.alpha == 0x0000 ) ) {
                return ( format_t::fXBGR4444 );
            }

            break;
        }

        case 15: {
            if ( _mask.red == 0 ) {
                return ( format_t::fXRGB1555 );
            }

            // FIX: Maybe reduntant
            [[fallthrough]];
        }

        case 16: {
            if ( _mask.red == 0 ) {
                return ( format_t::fRGB565 );
            }

            if ( ( _mask.red == 0x7C00 ) && ( _mask.green == 0x03E0 ) &&
                 ( _mask.blue == 0x001F ) && ( _mask.alpha == 0x0000 ) ) {
                return ( format_t::fXRGB1555 );
            }

            if ( ( _mask.red == 0x001F ) && ( _mask.green == 0x03E0 ) &&
                 ( _mask.blue == 0x7C00 ) && ( _mask.alpha == 0x0000 ) ) {
                return ( format_t::fXBGR1555 );
            }

            if ( ( _mask.red == 0x0F00 ) && ( _mask.green == 0x00F0 ) &&
                 ( _mask.blue == 0x000F ) && ( _mask.alpha == 0xF000 ) ) {
                return ( format_t::fARGB4444 );
            }

            if ( ( _mask.red == 0xF000 ) && ( _mask.green == 0x0F00 ) &&
                 ( _mask.blue == 0x00F0 ) && ( _mask.alpha == 0x000F ) ) {
                return ( format_t::fRGBA4444 );
            }

            if ( ( _mask.red == 0x000F ) && ( _mask.green == 0x00F0 ) &&
                 ( _mask.blue == 0x0F00 ) && ( _mask.alpha == 0xF000 ) ) {
                return ( format_t::fABGR4444 );
            }

            if ( ( _mask.red == 0x00F0 ) && ( _mask.green == 0x0F00 ) &&
                 ( _mask.blue == 0xF000 ) && ( _mask.alpha == 0x000F ) ) {
                return ( format_t::fBGRA4444 );
            }

            if ( ( _mask.red == 0x7C00 ) && ( _mask.green == 0x03E0 ) &&
                 ( _mask.blue == 0x001F ) && ( _mask.alpha == 0x8000 ) ) {
                return ( format_t::fARGB1555 );
            }

            if ( ( _mask.red == 0xF800 ) && ( _mask.green == 0x07C0 ) &&
                 ( _mask.blue == 0x003E ) && ( _mask.alpha == 0x0001 ) ) {
                return ( format_t::fRGBA5551 );
            }

            if ( ( _mask.red == 0x001F ) && ( _mask.green == 0x03E0 ) &&
                 ( _mask.blue == 0x7C00 ) && ( _mask.alpha == 0x8000 ) ) {
                return ( format_t::fABGR1555 );
            }

            if ( ( _mask.red == 0x003E ) && ( _mask.green == 0x07C0 ) &&
                 ( _mask.blue == 0xF800 ) && ( _mask.alpha == 0x0001 ) ) {
                return ( format_t::fBGRA5551 );
            }

            if ( ( _mask.red == 0xF800 ) && ( _mask.green == 0x07E0 ) &&
                 ( _mask.blue == 0x001F ) && ( _mask.alpha == 0x0000 ) ) {
                return ( format_t::fRGB565 );
            }

            if ( ( _mask.red == 0x001F ) && ( _mask.green == 0x07E0 ) &&
                 ( _mask.blue == 0xF800 ) && ( _mask.alpha == 0x0000 ) ) {
                return ( format_t::fBGR565 );
            }

            if ( ( _mask.red == 0x003F ) && ( _mask.green == 0x07C0 ) &&
                 ( _mask.blue == 0xF800 ) && ( _mask.alpha == 0x0000 ) ) {
                // Technically this would be BGR556, but Witek says this works
                // in bug 3158
                return ( format_t::fRGB565 );
            }

            break;
        }

        case 24: {
            switch ( _mask.red ) {
                case 0:
                case 0x00FF0000: {
#if __BYTE_ORDER == SDL_BIG_ENDIAN

                    return ( format_t::fRGB24 );

#else

                    return ( format_t::fBGR24 );

#endif
                }

                case 0x000000FF: {
#if __BYTE_ORDER == SDL_BIG_ENDIAN

                    return ( format_t::fBGR24 );

#else

                    return ( format_t::fRGB24 );

#endif
                }

                default: {
                }
            }

            break;
        }

        case 30: {
            if ( ( _mask.red == 0x3FF00000 ) && ( _mask.green == 0x000FFC00 ) &&
                 ( _mask.blue == 0x000003FF ) &&
                 ( _mask.alpha == 0x00000000 ) ) {
                return ( format_t::fXRGB2101010 );
            }

            if ( ( _mask.red == 0x000003FF ) && ( _mask.green == 0x000FFC00 ) &&
                 ( _mask.blue == 0x3FF00000 ) &&
                 ( _mask.alpha == 0x00000000 ) ) {
                return format_t::fXBGR2101010;
            }

            break;
        }

        case 32: {
            if ( _mask.red == 0 ) {
                return ( format_t::fXRGB8888 );
            }

            if ( ( _mask.red == 0x00FF0000 ) && ( _mask.green == 0x0000FF00 ) &&
                 ( _mask.blue == 0x000000FF ) &&
                 ( _mask.alpha == 0x00000000 ) ) {
                return ( format_t::fXRGB8888 );
            }

            if ( ( _mask.red == 0xFF000000 ) && ( _mask.green == 0x00FF0000 ) &&
                 ( _mask.blue == 0x0000FF00 ) &&
                 ( _mask.alpha == 0x00000000 ) ) {
                return ( format_t::fRGBX8888 );
            }

            if ( ( _mask.red == 0x000000FF ) && ( _mask.green == 0x0000FF00 ) &&
                 ( _mask.blue == 0x00FF0000 ) &&
                 ( _mask.alpha == 0x00000000 ) ) {
                return ( format_t::fXBGR8888 );
            }

            if ( ( _mask.red == 0x0000FF00 ) && ( _mask.green == 0x00FF0000 ) &&
                 ( _mask.blue == 0xFF000000 ) &&
                 ( _mask.alpha == 0x00000000 ) ) {
                return ( format_t::fBGRX8888 );
            }

            if ( ( _mask.red == 0x00FF0000 ) && ( _mask.green == 0x0000FF00 ) &&
                 ( _mask.blue == 0x000000FF ) &&
                 ( _mask.alpha == 0xFF000000 ) ) {
                return ( format_t::fARGB8888 );
            }

            if ( ( _mask.red == 0xFF000000 ) && ( _mask.green == 0x00FF0000 ) &&
                 ( _mask.blue == 0x0000FF00 ) &&
                 ( _mask.alpha == 0x000000FF ) ) {
                return ( format_t::fRGBA8888 );
            }

            if ( ( _mask.red == 0x000000FF ) && ( _mask.green == 0x0000FF00 ) &&
                 ( _mask.blue == 0x00FF0000 ) &&
                 ( _mask.alpha == 0xFF000000 ) ) {
                return ( format_t::fABGR8888 );
            }

            if ( ( _mask.red == 0x0000FF00 ) && ( _mask.green == 0x00FF0000 ) &&
                 ( _mask.blue == 0xFF000000 ) &&
                 ( _mask.alpha == 0x000000FF ) ) {
                return ( format_t::fBGRA8888 );
            }

            if ( ( _mask.red == 0x3FF00000 ) && ( _mask.green == 0x000FFC00 ) &&
                 ( _mask.blue == 0x000003FF ) &&
                 ( _mask.alpha == 0x00000000 ) ) {
                return ( format_t::fXRGB2101010 );
            }

            if ( ( _mask.red == 0x000003FF ) && ( _mask.green == 0x000FFC00 ) &&
                 ( _mask.blue == 0x3FF00000 ) &&
                 ( _mask.alpha == 0x00000000 ) ) {
                return ( format_t::fXBGR2101010 );
            }

            if ( ( _mask.red == 0x3FF00000 ) && ( _mask.green == 0x000FFC00 ) &&
                 ( _mask.blue == 0x000003FF ) &&
                 ( _mask.alpha == 0xC0000000 ) ) {
                return ( format_t::fARGB2101010 );
            }

            if ( ( _mask.red == 0x000003FF ) && ( _mask.green == 0x000FFC00 ) &&
                 ( _mask.blue == 0x3FF00000 ) &&
                 ( _mask.alpha == 0xC0000000 ) ) {
                return ( format_t::fABGR2101010 );
            }

            break;
        }

        default: {
        }
    }

    return ( format_t::unknown );
}

// Returned structure may come from a shared global cache (i.e. not newly
// allocated), and hence should not be modified, especially the palette. Weird
// errors such as 'Blit combination not supported' may occur.
[[nodiscard]] auto pixelFormatDetails( format_t _format )
    -> const pixelFormatDetails_t {
    return (
        std::bit_cast< SDL_PixelFormatDetails* >( SDL_GetPixelFormatDetails(
            static_cast< SDL_PixelFormat >( _format ) ) ) );
}

} // namespace pixels

} // namespace slickdl
