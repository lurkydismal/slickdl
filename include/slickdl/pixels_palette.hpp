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
auto get( pixelValue_t< U > _value, pixelFormatDetails_t _format ) -> color_t {
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

} // namespace pixels

} // namespace slickdl
