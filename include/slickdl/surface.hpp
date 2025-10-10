#pragma once

#include <SDL3/SDL_blendmode.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_surface.h>

#include <gsl/pointers>
#include <type_traits>
#include <utility>

#include "clipping_zone.hpp"
#include "slickdl.hpp"
#include "slickdl/color.hpp"
#include "stddebug.hpp"

namespace slickdl {

// A collection of pixels used in software blitting
//
// Pixels are arranged in memory in rows, with the top row first. Each row
// occupies an amount of memory given by the pitch (sometimes known as the row
// stride in non-SDL APIs)
//
// Within each row, pixels are arranged from left to right until the width is
// reached. Each pixel occupies a number of bits appropriate for its format,
// with most formats representing each pixel as one or more whole bytes (in
// some indexed formats, instead multiple pixels are packed into each byte),
// and a byte order given by the format. After encoding all pixels, any
// remaining bytes to reach the pitch are used as padding to reach a desired
// alignment, and have undefined contents
//
// When a surface holds YUV format data, the planes are assumed to be
// contiguous without padding between them, e.g. a 32x32 surface in NV12
// format with a pitch of 32 would consist of 32x32 bytes of Y plane followed
// by 32x16 bytes of UV plane
//
// When a surface holds MJPG format data, pixels points at the compressed JPEG
// image and pitch is the length of that data
using surface_t = struct surface {
    using flag_t = enum class flag : uint8_t {
        preallocated = 0x1u, // Surface uses preallocated pixel memory
        lockNeeded = 0x2U,   // Surface needs to be locked to access pixels
        locked = 0x4U,       // Surface is currently locked
        aligned = 0x8U,      // Surface uses pixel memory allocated with
                             // SDL_aligned_alloc()
    };

    using scaleMode_t = enum class scaleMode : int8_t {
        nearest = 0, // Nearest pixel sampling
        linear,      // Linear filtering
        pixelArt,    // Nearest pixel sampling with improved scaling
                     // for pixel art
    };

    using flipMode_t = enum class flipMode : uint8_t {
        none,       // Do not flip
        horizontal, // Flip horizontally
        vertical,   // Flip vertically
    };

    surface() = delete;
    surface( const surface& ) = default;
    surface( surface&& ) = default;

    template < typename OtherType >
        requires std::is_convertible_v< OtherType, SDL_Surface* >
    constexpr surface( OtherType&& _other )
        : _data( std::forward< OtherType >( _other ) ) {}

    surface( size_t _width, size_t _height, SDL_PixelFormat _format )
        : _data( SDL_CreateSurface( _width, _height, _format ) ) {
        stdfunc::assert( _width );
        stdfunc::assert( _height );

        SDL_CreateSurfacePalette( _data );
    }

    // Allocate a new surface with a specific pixel format and existing pixel
    // data
    //
    // No copy is made of the pixel data. Pixel data is not managed
    // automatically; you must free the surface before you free the pixel data
    //
    // Pitch is the offset in bytes from one row of pixels to the next, e.g
    // width * 4 for SDL_PIXELFORMAT_RGBA8888
    //
    // \param pitch the number of bytes between each row, including padding
    surface( int _width,
             int _height,
             SDL_PixelFormat _format,
             std::span< const std::byte > _pixels,
             int _pitch )
        : _data(
              SDL_CreateSurfaceFrom( _width,
                                     _height,
                                     _format,
                                     std::bit_cast< void* >( _pixels.data() ),
                                     _pitch ) ) {
        stdfunc::assert( _width );
        stdfunc::assert( _height );
        stdfunc::assert( !_pixels.empty() );
        stdfunc::assert( _pitch );
    }

    ~surface() { SDL_DestroySurface( _data ); }

    auto operator=( const surface& ) -> surface& = default;
    auto operator=( surface&& ) -> surface& = default;

    constexpr operator SDL_Surface*() const { return ( _data ); }

    // Evaluates to true if the surface needs to be locked before access
    [[nodiscard]] constexpr auto mustLock() const -> bool {
        return ( _data->flags ==
                 static_cast< uint32_t >( flag_t::lockNeeded ) );
    }

#if 0
    // TODO: Implement
    // Get the properties associated with a surface
    //
    // The following properties are understood by SDL:
    //
    // - SDL_PROP_SURFACE_SDR_WHITE_POINT_FLOAT: for HDR10 and floating point
    //   surfaces, this defines the value of 100% diffuse white, with higher
    //   values being displayed in the High Dynamic Range headroom. This
    // defaults to 203 for HDR10 surfaces and 1.0 for floating point surfaces
    // - SDL_PROP_SURFACE_HDR_HEADROOM_FLOAT: for HDR10 and floating point
    //   surfaces, this defines the maximum dynamic range used by the content,
    // in terms of the SDR white point. This defaults to 0.0, which disables
    // tone mapping
    // - SDL_PROP_SURFACE_TONEMAP_OPERATOR_STRING: the tone mapping operator
    //   used when compressing from a surface with high dynamic range to another
    //   with lower dynamic range. Currently this supports "chrome", which uses
    //   the same tone mapping that Chrome uses for HDR content, the form "*=N",
    //   where N is a floating point scale factor applied in linear space, and
    //   "none", which disables tone mapping. This defaults to "chrome"
    // - SDL_PROP_SURFACE_HOTSPOT_X_NUMBER: the hotspot pixel offset from the
    //   left edge of the image, if this surface is being used as a cursor
    // - SDL_PROP_SURFACE_HOTSPOT_Y_NUMBER: the hotspot pixel offset from the
    //   top edge of the image, if this surface is being used as a cursor
    auto properties() -> SDL_PropertiesID {
        const SDL_PropertiesID l_properties = SDL_GetSurfaceProperties( _data );

        slickdl::assert(l_properties);

        return ( l_properties );
    }

#define SDL_PROP_SURFACE_SDR_WHITE_POINT_FLOAT "SDL.surface.SDR_white_point"
#define SDL_PROP_SURFACE_HDR_HEADROOM_FLOAT "SDL.surface.HDR_headroom"
#define SDL_PROP_SURFACE_TONEMAP_OPERATOR_STRING "SDL.surface.tonemap"
#define SDL_PROP_SURFACE_HOTSPOT_X_NUMBER "SDL.surface.hotspot.x"
#define SDL_PROP_SURFACE_HOTSPOT_Y_NUMBER "SDL.surface.hotspot.y"
#endif

    // Setting the colorspace doesn't change the pixels, only how they are
    // interpreted in color operations
    //
    // Not thread safe
    void colorspace( SDL_Colorspace _colorspace ) {
        const bool l_result = SDL_SetSurfaceColorspace( _data, _colorspace );

        slickdl::assert( l_result );
    }

    // Get the colorspace used by a surface
    //
    // The colorspace defaults to SRGB_LINEAR for floating point
    // formats, HDR10 for 10-bit formats, SRGB for
    // other RGB surfaces and BT709_FULL for YUV textures
    //
    // Not thread safe
    [[nodiscard]] auto colorspace() const -> SDL_Colorspace {
        return ( SDL_GetSurfaceColorspace( _data ) );
    }

    // Set the palette used by a surface
    //
    // A single palette can be shared with many surfaces
    //
    // Not thread safe
    void palette( palette_t _palette ) {
        const bool l_result = SDL_SetSurfacePalette( _data, _palette );

        slickdl::assert( l_result );
    }

    // Get the palette used by a surface
    [[nodiscard]] auto palette() -> palette_t {
        return ( SDL_GetSurfacePalette( _data ) );
    }

    // Add an alternate version of a surface
    //
    // This function adds an alternate version of this surface, usually used for
    // content with high DPI representations like cursors or icons. The size,
    // format, and content do not need to match the original surface, and these
    // alternate versions will not be updated when the original surface changes
    //
    // This function adds a reference to the alternate version, so you should
    // destruct the image after this call
    //
    // Not thread safe
    void addAlternate( surface& _image ) {
        const bool l_result = SDL_AddSurfaceAlternateImage( _data, _image );

        slickdl::assert( l_result );

        // TODO: Call destructor
    }

    [[nodiscard]] auto hasAlternate() const -> bool {
        return ( SDL_SurfaceHasAlternateImages( _data ) );
    }

    // This returns all versions of a surface, with the surface being queried as
    // the first element in the returned array
    //
    // Not thread safe
    auto getAlternates() -> std::vector< surface > {
        std::vector< surface > l_returnValue;

        {
            // Get native images
            int l_count = 0;
            gsl::not_null< SDL_Surface** > l_images =
                SDL_GetSurfaceImages( _data, &l_count );

            // Convert
            for ( surface _image : std::span( l_images.get(), l_count ) ) {
                l_returnValue.emplace_back( _image );
            }

            SDL_free( l_images );
        }

        return ( l_returnValue );
    }

    // This function removes a reference from all the alternative versions,
    // destroying them if this is the last reference to them
    //
    // Not thread safe
    void removeAlternates() { SDL_RemoveSurfaceAlternateImages( _data ); }

    // Set up a surface for directly accessing the pixels
    //
    // Between calls to lock() / unlock(), you can write to and read from
    // pixels(), using the pixel format stored in format(). Once you are
    // done accessing the surface, you should use unlock() to release it
    //
    // Not all surfaces require locking. If mustLock() is false, then you can
    // read and write to the surface at any time, and the pixel format of the
    // surface will not change
    //
    // Not thread safe
    //
    // The locking referred to by this function is making the
    // pixels available for direct access, not thread-safe locking
    void lock() {
        const bool l_result = SDL_LockSurface( _data );

        slickdl::assert( l_result );
    }

    // Release a surface after directly accessing the pixels
    //
    // Not thread safe
    //
    // The locking referred to by this function is making the pixels available
    // for direct access, not thread-safe locking
    void unlock() { SDL_UnlockSurface( _data ); }

#if 0
    // TODO: Implement
    // If closeIO is true, closes IO stream before returning, even in the case
    // of an error
    auto loadBMP( SDL_IOStream* _sourceStream, bool _closeIO ) -> surface {
    }
#endif

#if 0
    // Save a surface to a seekable SDL data stream in BMP format
    //
    // Surfaces with a 24-bit, 32-bit and paletted 8-bit format get saved in the
    // BMP directly. Other RGB formats with 8-bit or higher get converted to a
    // 24-bit surface or, if they have an alpha mask or a colorkey, to a 32-bit
    // surface before they are saved. YUV and paletted 1-bit and 4-bit formats
    // are not supported
    //
    // If closeIO is true, closes IO stream before returning, even in the case
    // of an error
    //
    // Not thread safe
    void saveBMP( SDL_IOStream& _stream, bool _closeIO ) {
        const bool l_result = SDL_SaveBMP_IO( _data, &_stream, _closeIO );

        slickdl::assert( l_result );
    }
#endif

    // If RLE is enabled, color key and alpha blending blits are much faster,
    // but the surface must be locked before directly accessing the pixels.
    //
    // Not thread safe.
    void setRLE( bool _enabled ) {
        const bool l_result = SDL_SetSurfaceRLE( _data, _enabled );

        slickdl::assert( l_result );
    }

    [[nodiscard]] auto hasRLE() const -> bool {
        return ( SDL_SurfaceHasRLE( _data ) );
    }

    // Set the color key (transparent pixel) in a surface.
    //
    // The color key defines a pixel value that will be treated as transparent
    // in a blit. For example, one can use this to specify that cyan pixels
    // should be considered transparent, and therefore not rendered.
    //
    // It is a pixel of the format used by the surface, as generated by
    // SDL_MapRGB().
    //
    // Not thread safe.
    void colorKey( bool _enabled, color_t _key ) {
        const bool l_result =
            SDL_SetSurfaceColorKey( _data, _enabled, _key.pack() );

        slickdl::assert( l_result );
    }

    // Returns whether the surface has a color key.
    auto hasColorKey() -> bool { return ( SDL_SurfaceHasColorKey( _data ) ); }

    // Get the color key (transparent pixel) for a surface.
    //
    // The color key is a pixel of the format used by the surface, as generated
    // by SDL_MapRGB().
    [[nodiscard]] auto colorKey() const -> color_t {
        uint32_t l_key = 0;

        const bool l_result = SDL_GetSurfaceColorKey( _data, &l_key );

        slickdl::assert( l_result );

        return ( l_key );
    }

    // Set an additional RGB color value multiplied into blit operations.
    //
    // When this surface is blitted, during the blit operation each source color
    // channel is modulated by the appropriate color value according to the
    // following formula:
    //
    // srcC = srcC * (color / 255)
    //
    // Not thread safe.
    void colorMod( color_t _color ) {
        const bool l_result = SDL_SetSurfaceColorMod(
            _data, _color.red, _color.green, _color.blue );

        slickdl::assert( l_result );
    }

    // Get the additional color value multiplied into blit operations.
    //
    // Not thread safe.
    [[nodiscard]] auto colorMod() const -> color_t {
        color_t l_returnValue;

        const bool l_result =
            SDL_GetSurfaceColorMod( _data, &l_returnValue.red,
                                    &l_returnValue.green, &l_returnValue.blue );

        slickdl::assert( l_result );

        return ( l_returnValue );
    }

    // Set an additional alpha value used in blit operations.
    //
    // When this surface is blitted, during the blit operation the source alpha
    // value is modulated by this alpha value according to the following
    // formula:
    //
    // srcA = srcA * (alpha / 255)
    //
    // Not thread safe.
    void alphaMod( uint8_t _alpha ) {
        const bool l_result = SDL_SetSurfaceAlphaMod( _data, _alpha );

        slickdl::assert( l_result );
    }

    // Get the additional alpha value used in blit operations.
    [[nodiscard]] auto alphaMod() const -> uint8_t {
        uint8_t l_alpha = 0;

        const bool l_result = SDL_GetSurfaceAlphaMod( _data, &l_alpha );

        slickdl::assert( l_result );

        return ( l_alpha );
    }

    // Set the blend mode used for blit operations.
    //
    // To copy a surface to another surface (or texture) without blending with
    // the existing data, the blendmode of the SOURCE surface should be set to
    // none.
    //
    // Not thread safe.
    void blendMode( SDL_BlendMode _blendMode ) {
        const bool l_result = SDL_SetSurfaceBlendMode( _data, _blendMode );

        slickdl::assert( l_result );
    }

    // Get the blend mode used for blit operations.
    [[nodiscard]] auto blendMode() const -> SDL_BlendMode {
        SDL_BlendMode l_blendMode = 0;

        const bool l_result = SDL_GetSurfaceBlendMode( _data, &l_blendMode );

        slickdl::assert( l_result );

        return ( l_blendMode );
    }

    // Set the clipping rectangle for a surface.
    //
    // When surface is the destination of a blit, only the area within the clip
    // rectangle is drawn into.
    //
    // Note that blits are automatically clipped to the edges of the source and
    // destination surfaces.
    //
    // \param rect the SDL_Rect structure representing the clipping rectangle,
    // or NULL to disable clipping.
    // \returns true if the rectangle intersects the surface, otherwise false
    // and blits will be completely clipped.
    //
    // Not thread safe.
    void clip( const clippingZone_t< int >& _zone ) {
        const bool l_result = SDL_SetSurfaceClipRect( _data, _zone );

        slickdl::assert( l_result );
    }

    // Variables
private:
    gsl::not_null< SDL_Surface* > _data;
};

} // namespace slickdl
