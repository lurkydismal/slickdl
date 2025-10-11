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

    // If the original surface has alternate images, the new surface will have a
    // reference to them as well.
    //
    // Not thread safe.
    surface( const surface& _other )
        : _data( SDL_DuplicateSurface( _other ) ) {}

    // TODO: Maybe will assert
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
        auto l_zone = static_cast< SDL_Rect >( _zone );

        const bool l_result = SDL_SetSurfaceClipRect( _data, &l_zone );

        slickdl::assert( l_result );
    }

    // Get the clipping rectangle for a surface.
    //
    // When surface is the destination of a blit, only the area within the clip
    // rectangle is drawn into.
    //
    // Not thread safe.
    [[nodiscard]] auto clip() const -> clippingZone_t< int > {
        SDL_Rect l_zone{};

        const bool l_result = SDL_GetSurfaceClipRect( _data, &l_zone );

        slickdl::assert( l_result );

        return ( static_cast< clippingZone_t< int > >( l_zone ) );
    }

    // Flip a surface vertically or horizontally.
    //
    // Not thread safe.
    void flip( SDL_FlipMode _flip ) {
        const bool l_result = SDL_FlipSurface( _data, _flip );

        slickdl::assert( l_result );
    }

    // Creates a new surface identical to the existing surface, scaled to the
    // desired size.
    //
    // Not thread safe.
    [[nodiscard]] auto scale( int _width,
                              int _height,
                              SDL_ScaleMode _scaleMode ) const -> surface {
        return ( SDL_ScaleSurface( _data, _width, _height, _scaleMode ) );
    }

    // Copy an existing surface to a new surface of the specified format.
    //
    // This function is used to optimize images for faster *repeat* blitting.
    // This is accomplished by converting the original and storing the result as
    // a new surface. The new, optimized surface can then be used as the source
    // for future blits, making them faster.
    //
    // If you are converting to an indexed surface and want to map colors to a
    // palette, you can use SDL_ConvertSurfaceAndColorspace() instead.
    //
    // If the original surface has alternate images, the new surface will have a
    // reference to them as well.
    //
    // Not thread safe.
    [[nodiscard]] auto convert( SDL_PixelFormat _format ) const -> surface {
        return ( SDL_ConvertSurface( _data, _format ) );
    }

    // Copy an existing surface to a new surface of the specified format and
    // colorspace.
    //
    // This function converts an existing surface to a new format and colorspace
    // and returns the new surface. This will perform any pixel format and
    // colorspace conversion needed.
    //
    // If the original surface has alternate images, the new surface will have a
    // reference to them as well.
    //
    // Not thread safe.
    [[nodiscard]] auto convert( SDL_PixelFormat _format,
                                palette_t _palette,
                                SDL_Colorspace _colorspace,
                                SDL_PropertiesID _props = 0 ) const -> surface {
        return ( SDL_ConvertSurfaceAndColorspace( _data, _format, _palette,
                                                  _colorspace, _props ) );
    }

    // Copy an existing surface to a new surface of the specified format and
    // colorspace.
    //
    // This function converts an existing surface to a new format and colorspace
    // and returns the new surface. This will perform any pixel format and
    // colorspace conversion needed.
    //
    // If the original surface has alternate images, the new surface will have a
    // reference to them as well.
    //
    // Not thread safe.
    [[nodiscard]] auto convert( SDL_PixelFormat _format,
                                SDL_Colorspace _colorspace,
                                SDL_PropertiesID _props = 0 ) const -> surface {
        return ( SDL_ConvertSurfaceAndColorspace( _data, _format, nullptr,
                                                  _colorspace, _props ) );
    }

    // Premultiply the alpha in a surface.
    //
    // This is safe to use with src == dst, but not for other overlapping areas.
    //
    // \if linear is true to convert from sRGB to linear space for the alpha
    // multiplication, false to do multiplication in sRGB space.
    //
    // Not thread safe.
    void premultiplyAlpha( bool _linear ) {
        const bool l_result = SDL_PremultiplySurfaceAlpha( _data, _linear );

        stdfunc::assert( l_result );
    }

    // Variables
private:
    gsl::not_null< SDL_Surface* > _data;
};

#if 0

/**
 * Clear a surface with a specific color, with floating point precision.
 *
 * This function handles all surface formats, and ignores any clip rectangle.
 *
 * If the surface is YUV, the color is assumed to be in the sRGB colorspace,
 * otherwise the color is assumed to be in the colorspace of the suface.
 *
 * \param surface the SDL_Surface to clear.
 * \param r the red component of the pixel, normally in the range 0-1.
 * \param g the green component of the pixel, normally in the range 0-1.
 * \param b the blue component of the pixel, normally in the range 0-1.
 * \param a the alpha component of the pixel, normally in the range 0-1.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * Not thread safe.
 */
bool ClearSurface(SDL_Surface *surface, float r, float g, float b, float a);

/**
 * Perform a fast fill of a rectangle with a specific color.
 *
 * `color` should be a pixel of the format used by the surface, and can be
 * generated by SDL_MapRGB() or SDL_MapRGBA(). If the color value contains an
 * alpha component then the destination is simply filled with that alpha
 * information, no blending takes place.
 *
 * If there is a clip rectangle set on the destination (set via
 * SDL_SetSurfaceClipRect()), then this function will fill based on the
 * intersection of the clip rectangle and `rect`.
 *
 * \param dst the SDL_Surface structure that is the drawing target.
 * \param rect the SDL_Rect structure representing the rectangle to fill, or
 *             NULL to fill the entire surface.
 * \param color the color to fill with.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * Not thread safe.
 */
bool FillSurfaceRect(SDL_Surface *dst, const SDL_Rect *rect, Uint32 color);

/**
 * Perform a fast fill of a set of rectangles with a specific color.
 *
 * `color` should be a pixel of the format used by the surface, and can be
 * generated by SDL_MapRGB() or SDL_MapRGBA(). If the color value contains an
 * alpha component then the destination is simply filled with that alpha
 * information, no blending takes place.
 *
 * If there is a clip rectangle set on the destination (set via
 * SDL_SetSurfaceClipRect()), then this function will fill based on the
 * intersection of the clip rectangle and `rect`.
 *
 * \param dst the SDL_Surface structure that is the drawing target.
 * \param rects an array of SDL_Rects representing the rectangles to fill.
 * \param count the number of rectangles in the array.
 * \param color the color to fill with.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * Not thread safe.
 */
bool FillSurfaceRects(SDL_Surface *dst, const SDL_Rect *rects, int count, Uint32 color);

/**
 * Performs a fast blit from the source surface to the destination surface
 * with clipping.
 *
 * If either `srcrect` or `dstrect` are NULL, the entire surface (`src` or
 * `dst`) is copied while ensuring clipping to `dst->clip_rect`.
 *
 * The blit function should not be called on a locked surface.
 *
 * The blit semantics for surfaces with and without blending and colorkey are
 * defined as follows:
 *
 * ```
 *    RGBA->RGB:
 *      Source surface blend mode set to SDL_BLENDMODE_BLEND:
 *       alpha-blend (using the source alpha-channel and per-surface alpha)
 *       SDL_SRCCOLORKEY ignored.
 *     Source surface blend mode set to SDL_BLENDMODE_NONE:
 *       copy RGB.
 *       if SDL_SRCCOLORKEY set, only copy the pixels that do not match the
 *       RGB values of the source color key, ignoring alpha in the
 *       comparison.
 *
 *   RGB->RGBA:
 *     Source surface blend mode set to SDL_BLENDMODE_BLEND:
 *       alpha-blend (using the source per-surface alpha)
 *     Source surface blend mode set to SDL_BLENDMODE_NONE:
 *       copy RGB, set destination alpha to source per-surface alpha value.
 *     both:
 *       if SDL_SRCCOLORKEY set, only copy the pixels that do not match the
 *       source color key.
 *
 *   RGBA->RGBA:
 *     Source surface blend mode set to SDL_BLENDMODE_BLEND:
 *       alpha-blend (using the source alpha-channel and per-surface alpha)
 *       SDL_SRCCOLORKEY ignored.
 *     Source surface blend mode set to SDL_BLENDMODE_NONE:
 *       copy all of RGBA to the destination.
 *       if SDL_SRCCOLORKEY set, only copy the pixels that do not match the
 *       RGB values of the source color key, ignoring alpha in the
 *       comparison.
 *
 *   RGB->RGB:
 *     Source surface blend mode set to SDL_BLENDMODE_BLEND:
 *       alpha-blend (using the source per-surface alpha)
 *     Source surface blend mode set to SDL_BLENDMODE_NONE:
 *       copy RGB.
 *     both:
 *       if SDL_SRCCOLORKEY set, only copy the pixels that do not match the
 *       source color key.
 * ```
 *
 * \param src the SDL_Surface structure to be copied from.
 * \param srcrect the SDL_Rect structure representing the rectangle to be
 *                copied, or NULL to copy the entire surface.
 * \param dst the SDL_Surface structure that is the blit target.
 * \param dstrect the SDL_Rect structure representing the x and y position in
 *                the destination surface, or NULL for (0,0). The width and
 *                height are ignored, and are copied from `srcrect`. If you
 *                want a specific width and height, you should use
 *                SDL_BlitSurfaceScaled().
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety Only one thread should be using the `src` and `dst` surfaces
 *               at any given time.
 */
bool BlitSurface(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, const SDL_Rect *dstrect);

/**
 * Perform low-level surface blitting only.
 *
 * This is a semi-private blit function and it performs low-level surface
 * blitting, assuming the input rectangles have already been clipped.
 *
 * \param src the SDL_Surface structure to be copied from.
 * \param srcrect the SDL_Rect structure representing the rectangle to be
 *                copied, may not be NULL.
 * \param dst the SDL_Surface structure that is the blit target.
 * \param dstrect the SDL_Rect structure representing the target rectangle in
 *                the destination surface, may not be NULL.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety Only one thread should be using the `src` and `dst` surfaces
 *               at any given time.
 */
bool BlitSurfaceUnchecked(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, const SDL_Rect *dstrect);

/**
 * Perform a scaled blit to a destination surface, which may be of a different
 * format.
 *
 * \param src the SDL_Surface structure to be copied from.
 * \param srcrect the SDL_Rect structure representing the rectangle to be
 *                copied, or NULL to copy the entire surface.
 * \param dst the SDL_Surface structure that is the blit target.
 * \param dstrect the SDL_Rect structure representing the target rectangle in
 *                the destination surface, or NULL to fill the entire
 *                destination surface.
 * \param scaleMode the SDL_ScaleMode to be used.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety Only one thread should be using the `src` and `dst` surfaces
 *               at any given time.
 */
bool BlitSurfaceScaled(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, const SDL_Rect *dstrect, SDL_ScaleMode scaleMode);

/**
 * Perform low-level surface scaled blitting only.
 *
 * This is a semi-private function and it performs low-level surface blitting,
 * assuming the input rectangles have already been clipped.
 *
 * \param src the SDL_Surface structure to be copied from.
 * \param srcrect the SDL_Rect structure representing the rectangle to be
 *                copied, may not be NULL.
 * \param dst the SDL_Surface structure that is the blit target.
 * \param dstrect the SDL_Rect structure representing the target rectangle in
 *                the destination surface, may not be NULL.
 * \param scaleMode the SDL_ScaleMode to be used.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety Only one thread should be using the `src` and `dst` surfaces
 *               at any given time.
 */
bool BlitSurfaceUncheckedScaled(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, const SDL_Rect *dstrect, SDL_ScaleMode scaleMode);

/**
 * Perform a stretched pixel copy from one surface to another.
 *
 * \param src the SDL_Surface structure to be copied from.
 * \param srcrect the SDL_Rect structure representing the rectangle to be
 *                copied, or NULL to copy the entire surface.
 * \param dst the SDL_Surface structure that is the blit target.
 * \param dstrect the SDL_Rect structure representing the target rectangle in
 *                the destination surface, or NULL to fill the entire
 *                destination surface.
 * \param scaleMode the SDL_ScaleMode to be used.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety Only one thread should be using the `src` and `dst` surfaces
 *               at any given time.
 */
bool StretchSurface(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, const SDL_Rect *dstrect, SDL_ScaleMode scaleMode);

/**
 * Perform a tiled blit to a destination surface, which may be of a different
 * format.
 *
 * The pixels in `srcrect` will be repeated as many times as needed to
 * completely fill `dstrect`.
 *
 * \param src the SDL_Surface structure to be copied from.
 * \param srcrect the SDL_Rect structure representing the rectangle to be
 *                copied, or NULL to copy the entire surface.
 * \param dst the SDL_Surface structure that is the blit target.
 * \param dstrect the SDL_Rect structure representing the target rectangle in
 *                the destination surface, or NULL to fill the entire surface.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety Only one thread should be using the `src` and `dst` surfaces
 *               at any given time.
 */
bool BlitSurfaceTiled(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, const SDL_Rect *dstrect);

/**
 * Perform a scaled and tiled blit to a destination surface, which may be of a
 * different format.
 *
 * The pixels in `srcrect` will be scaled and repeated as many times as needed
 * to completely fill `dstrect`.
 *
 * \param src the SDL_Surface structure to be copied from.
 * \param srcrect the SDL_Rect structure representing the rectangle to be
 *                copied, or NULL to copy the entire surface.
 * \param scale the scale used to transform srcrect into the destination
 *              rectangle, e.g. a 32x32 texture with a scale of 2 would fill
 *              64x64 tiles.
 * \param scaleMode scale algorithm to be used.
 * \param dst the SDL_Surface structure that is the blit target.
 * \param dstrect the SDL_Rect structure representing the target rectangle in
 *                the destination surface, or NULL to fill the entire surface.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety Only one thread should be using the `src` and `dst` surfaces
 *               at any given time.
 */
bool BlitSurfaceTiledWithScale(SDL_Surface *src, const SDL_Rect *srcrect, float scale, SDL_ScaleMode scaleMode, SDL_Surface *dst, const SDL_Rect *dstrect);

/**
 * Perform a scaled blit using the 9-grid algorithm to a destination surface,
 * which may be of a different format.
 *
 * The pixels in the source surface are split into a 3x3 grid, using the
 * different corner sizes for each corner, and the sides and center making up
 * the remaining pixels. The corners are then scaled using `scale` and fit
 * into the corners of the destination rectangle. The sides and center are
 * then stretched into place to cover the remaining destination rectangle.
 *
 * \param src the SDL_Surface structure to be copied from.
 * \param srcrect the SDL_Rect structure representing the rectangle to be used
 *                for the 9-grid, or NULL to use the entire surface.
 * \param left_width the width, in pixels, of the left corners in `srcrect`.
 * \param right_width the width, in pixels, of the right corners in `srcrect`.
 * \param top_height the height, in pixels, of the top corners in `srcrect`.
 * \param bottom_height the height, in pixels, of the bottom corners in
 *                      `srcrect`.
 * \param scale the scale used to transform the corner of `srcrect` into the
 *              corner of `dstrect`, or 0.0f for an unscaled blit.
 * \param scaleMode scale algorithm to be used.
 * \param dst the SDL_Surface structure that is the blit target.
 * \param dstrect the SDL_Rect structure representing the target rectangle in
 *                the destination surface, or NULL to fill the entire surface.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety Only one thread should be using the `src` and `dst` surfaces
 *               at any given time.
 */
bool BlitSurface9Grid(SDL_Surface *src, const SDL_Rect *srcrect, int left_width, int right_width, int top_height, int bottom_height, float scale, SDL_ScaleMode scaleMode, SDL_Surface *dst, const SDL_Rect *dstrect);

/**
 * Map an RGB triple to an opaque pixel value for a surface.
 *
 * This function maps the RGB color value to the specified pixel format and
 * returns the pixel value best approximating the given RGB color value for
 * the given pixel format.
 *
 * If the surface has a palette, the index of the closest matching color in
 * the palette will be returned.
 *
 * If the surface pixel format has an alpha component it will be returned as
 * all 1 bits (fully opaque).
 *
 * If the pixel format bpp (color depth) is less than 32-bpp then the unused
 * upper bits of the return value can safely be ignored (e.g., with a 16-bpp
 * format the return value can be assigned to a Uint16, and similarly a Uint8
 * for an 8-bpp format).
 *
 * \param surface the surface to use for the pixel format and palette.
 * \param r the red component of the pixel in the range 0-255.
 * \param g the green component of the pixel in the range 0-255.
 * \param b the blue component of the pixel in the range 0-255.
 * \returns a pixel value.
 *
 * \threadsafety It is safe to call this function from any thread.
 */
Uint32 SDL_MapSurfaceRGB(SDL_Surface *surface, Uint8 r, Uint8 g, Uint8 b);

/**
 * Map an RGBA quadruple to a pixel value for a surface.
 *
 * This function maps the RGBA color value to the specified pixel format and
 * returns the pixel value best approximating the given RGBA color value for
 * the given pixel format.
 *
 * If the surface pixel format has no alpha component the alpha value will be
 * ignored (as it will be in formats with a palette).
 *
 * If the surface has a palette, the index of the closest matching color in
 * the palette will be returned.
 *
 * If the pixel format bpp (color depth) is less than 32-bpp then the unused
 * upper bits of the return value can safely be ignored (e.g., with a 16-bpp
 * format the return value can be assigned to a Uint16, and similarly a Uint8
 * for an 8-bpp format).
 *
 * \param surface the surface to use for the pixel format and palette.
 * \param r the red component of the pixel in the range 0-255.
 * \param g the green component of the pixel in the range 0-255.
 * \param b the blue component of the pixel in the range 0-255.
 * \param a the alpha component of the pixel in the range 0-255.
 * \returns a pixel value.
 *
 * \threadsafety It is safe to call this function from any thread.
 */
Uint32 SDL_MapSurfaceRGBA(SDL_Surface *surface, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/**
 * Retrieves a single pixel from a surface.
 *
 * This function prioritizes correctness over speed: it is suitable for unit
 * tests, but is not intended for use in a game engine.
 *
 * Like SDL_GetRGBA, this uses the entire 0..255 range when converting color
 * components from pixel formats with less than 8 bits per RGB component.
 *
 * \param surface the surface to read.
 * \param x the horizontal coordinate, 0 <= x < width.
 * \param y the vertical coordinate, 0 <= y < height.
 * \param r a pointer filled in with the red channel, 0-255, or NULL to ignore
 *          this channel.
 * \param g a pointer filled in with the green channel, 0-255, or NULL to
 *          ignore this channel.
 * \param b a pointer filled in with the blue channel, 0-255, or NULL to
 *          ignore this channel.
 * \param a a pointer filled in with the alpha channel, 0-255, or NULL to
 *          ignore this channel.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * Not thread safe.
 */
bool ReadSurfacePixel(SDL_Surface *surface, int x, int y, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a);

/**
 * Retrieves a single pixel from a surface.
 *
 * This function prioritizes correctness over speed: it is suitable for unit
 * tests, but is not intended for use in a game engine.
 *
 * \param surface the surface to read.
 * \param x the horizontal coordinate, 0 <= x < width.
 * \param y the vertical coordinate, 0 <= y < height.
 * \param r a pointer filled in with the red channel, normally in the range
 *          0-1, or NULL to ignore this channel.
 * \param g a pointer filled in with the green channel, normally in the range
 *          0-1, or NULL to ignore this channel.
 * \param b a pointer filled in with the blue channel, normally in the range
 *          0-1, or NULL to ignore this channel.
 * \param a a pointer filled in with the alpha channel, normally in the range
 *          0-1, or NULL to ignore this channel.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * Not thread safe.
 */
bool ReadSurfacePixelFloat(SDL_Surface *surface, int x, int y, float *r, float *g, float *b, float *a);

/**
 * Writes a single pixel to a surface.
 *
 * This function prioritizes correctness over speed: it is suitable for unit
 * tests, but is not intended for use in a game engine.
 *
 * Like SDL_MapRGBA, this uses the entire 0..255 range when converting color
 * components from pixel formats with less than 8 bits per RGB component.
 *
 * \param surface the surface to write.
 * \param x the horizontal coordinate, 0 <= x < width.
 * \param y the vertical coordinate, 0 <= y < height.
 * \param r the red channel value, 0-255.
 * \param g the green channel value, 0-255.
 * \param b the blue channel value, 0-255.
 * \param a the alpha channel value, 0-255.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * Not thread safe.
 */
bool WriteSurfacePixel(SDL_Surface *surface, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/**
 * Writes a single pixel to a surface.
 *
 * This function prioritizes correctness over speed: it is suitable for unit
 * tests, but is not intended for use in a game engine.
 *
 * \param surface the surface to write.
 * \param x the horizontal coordinate, 0 <= x < width.
 * \param y the vertical coordinate, 0 <= y < height.
 * \param r the red channel value, normally in the range 0-1.
 * \param g the green channel value, normally in the range 0-1.
 * \param b the blue channel value, normally in the range 0-1.
 * \param a the alpha channel value, normally in the range 0-1.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * Not thread safe.
 */
bool WriteSurfacePixelFloat(SDL_Surface *surface, int x, int y, float r, float g, float b, float a);

#endif

#if 0

/**
 * Copy a block of pixels of one format to another format.
 *
 * \param width the width of the block to copy, in pixels.
 * \param height the height of the block to copy, in pixels.
 * \param src_format an SDL_PixelFormat value of the `src` pixels format.
 * \param src a pointer to the source pixels.
 * \param src_pitch the pitch of the source pixels, in bytes.
 * \param dst_format an SDL_PixelFormat value of the `dst` pixels format.
 * \param dst a pointer to be filled in with new pixel data.
 * \param dst_pitch the pitch of the destination pixels, in bytes.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety The same destination pixels should not be used from two
 *               threads at once. It is safe to use the same source pixels
 *               from multiple threads.
 */
bool ConvertPixels(int width, int height, SDL_PixelFormat src_format, const void *src, int src_pitch, SDL_PixelFormat dst_format, void *dst, int dst_pitch);

/**
 * Copy a block of pixels of one format and colorspace to another format and
 * colorspace.
 *
 * \param width the width of the block to copy, in pixels.
 * \param height the height of the block to copy, in pixels.
 * \param src_format an SDL_PixelFormat value of the `src` pixels format.
 * \param src_colorspace an SDL_Colorspace value describing the colorspace of
 *                       the `src` pixels.
 * \param src_properties an SDL_PropertiesID with additional source color
 *                       properties, or 0.
 * \param src a pointer to the source pixels.
 * \param src_pitch the pitch of the source pixels, in bytes.
 * \param dst_format an SDL_PixelFormat value of the `dst` pixels format.
 * \param dst_colorspace an SDL_Colorspace value describing the colorspace of
 *                       the `dst` pixels.
 * \param dst_properties an SDL_PropertiesID with additional destination color
 *                       properties, or 0.
 * \param dst a pointer to be filled in with new pixel data.
 * \param dst_pitch the pitch of the destination pixels, in bytes.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety The same destination pixels should not be used from two
 *               threads at once. It is safe to use the same source pixels
 *               from multiple threads.
 */
bool ConvertPixelsAndColorspace(int width, int height, SDL_PixelFormat src_format, SDL_Colorspace src_colorspace, SDL_PropertiesID src_properties, const void *src, int src_pitch, SDL_PixelFormat dst_format, SDL_Colorspace dst_colorspace, SDL_PropertiesID dst_properties, void *dst, int dst_pitch);

/**
 * Premultiply the alpha on a block of pixels.
 *
 * This is safe to use with src == dst, but not for other overlapping areas.
 *
 * \param width the width of the block to convert, in pixels.
 * \param height the height of the block to convert, in pixels.
 * \param src_format an SDL_PixelFormat value of the `src` pixels format.
 * \param src a pointer to the source pixels.
 * \param src_pitch the pitch of the source pixels, in bytes.
 * \param dst_format an SDL_PixelFormat value of the `dst` pixels format.
 * \param dst a pointer to be filled in with premultiplied pixel data.
 * \param dst_pitch the pitch of the destination pixels, in bytes.
 * \param linear true to convert from sRGB to linear space for the alpha
 *               multiplication, false to do multiplication in sRGB space.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety The same destination pixels should not be used from two
 *               threads at once. It is safe to use the same source pixels
 *               from multiple threads.
 */
bool PremultiplyAlpha(int width, int height, SDL_PixelFormat src_format, const void *src, int src_pitch, SDL_PixelFormat dst_format, void *dst, int dst_pitch, bool linear);

#endif

} // namespace slickdl
