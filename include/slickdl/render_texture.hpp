#pragma once

#include <SDL3/SDL_render.h>
#include <sys/types.h>

#include <cstdint>
#include <gsl/pointers>

#include "slickdl.hpp"
#include "slickdl/blend.hpp"
#include "slickdl/pixels_palette.hpp"
#include "slickdl/surface.hpp"
#include "stdfunc.hpp"
#include "surface.hpp"

namespace slickdl {

using access_t = enum class access : uint8_t {
    aStatic,   /**< Changes rarely, not lockable */
    streaming, /**< Changes frequently, lockable */
    target,    /**< Texture can be used as a render target */
};

// This affects how texture coordinates are interpreted outside of [0, 1]
using addressMode_t = enum class addressMode : int8_t {
    invalid = -1,
    amAuto, /**< Wrapping is enabled if texture coordinates are outside [0,
               1], this is the default */
    clamp,  /**< Texture coordinates are clamped to the [0, 1] range */
    wrap,   /**< The texture is repeated (tiled) */
};

// How the logical size is mapped to the output.
using logicalPresentation_t = enum class rendererLogicalPresentation : uint8_t {
    disabled, /**< There is no logical size in effect */
    stretch,  /**< The rendered content is stretched to the output resolution */
    letterbox, /**< The rendered content is fit to the largest dimension and the
                  other dimension is letterboxed with black bars */
    overscan, /**< The rendered content is fit to the smallest dimension and the
                 other dimension extends beyond the output bounds */
    integerScale, /**< The rendered content is scaled up by integer multiples to
                     fit the output resolution */
};

// Legacy
[[nodiscard]] constexpr auto toLegacy( access_t _access ) -> SDL_TextureAccess {
    return ( static_cast< SDL_TextureAccess >( _access ) );
}

[[nodiscard]] constexpr auto toLegacy( addressMode_t _addressMode )
    -> SDL_TextureAddressMode {
    return ( static_cast< SDL_TextureAddressMode >( _addressMode ) );
}

[[nodiscard]] constexpr auto toLegacy(
    logicalPresentation_t _logicalPresentation )
    -> SDL_RendererLogicalPresentation {
    return ( static_cast< SDL_RendererLogicalPresentation >(
        _logicalPresentation ) );
}

// Renderer
// Get the number of 2D rendering drivers available for the current display.
//
// A render driver is a set of code that handles rendering and texture
// management on a particular display. Normally there is only one, but some
// drivers may have several available with different capabilities.
//
// There may be none if SDL was compiled without render support.
[[nodiscard]] auto renderDrivers() -> size_t {
    const size_t l_amount = SDL_GetNumRenderDrivers();

    stdfunc::assert( l_amount );

    return ( l_amount );
}

// Use this function to get the name of a built in 2D rendering driver.
//
// The list of rendering drivers is given in the order that they are normally
// initialized by default; the drivers that seem more reasonable to choose
// first (as far as the SDL developers believe) are earlier in the list.
//
// The names of drivers are all simple, low-ASCII identifiers, like "opengl",
// "direct3d12" or "metal". These never have Unicode characters, and are not
// meant to be proper names.
[[nodiscard]] auto renderDriver( size_t _index ) -> std::string_view {
    const char* l_name = SDL_GetRenderDriver( _index );

    slickdl::assert( l_name );

    return ( l_name );
}

using renderer_t = struct renderer {
    static constexpr std::string_view g_createNameString =
        "SDL.renderer.create.name";
    static constexpr std::string_view g_createWindowPointer =
        "SDL.renderer.create.window";
    static constexpr std::string_view g_createSurfacePointer =
        "SDL.renderer.create.surface";
    static constexpr std::string_view g_createOutputColorspaceNumber =
        "SDL.renderer.create.output_colorspace";
    static constexpr std::string_view g_createPresentVsyncNumber =
        "SDL.renderer.create.present_vsync";
    static constexpr std::string_view g_createGPUShadersSpirvBoolean =
        "SDL.renderer.create.gpu.shaders_spirv";
    static constexpr std::string_view g_createGPUShadersDxilBoolean =
        "SDL.renderer.create.gpu.shaders_dxil";
    static constexpr std::string_view g_createGPUShadersMslBoolean =
        "SDL.renderer.create.gpu.shaders_msl";
    static constexpr std::string_view g_createVulkanInstancePointer =
        "SDL.renderer.create.vulkan.instance";
    static constexpr std::string_view g_createVulkanSurfaceNumber =
        "SDL.renderer.create.vulkan.surface";
    static constexpr std::string_view g_createVulkanPhysicalDevicePointer =
        "SDL.renderer.create.vulkan.physical_device";
    static constexpr std::string_view g_createVulkanDevicePointer =
        "SDL.renderer.create.vulkan.device";
    static constexpr std::string_view
        g_createVulkanGraphicsQueueFamilyIndexNumber =
            "SDL.renderer.create.vulkan.graphics_queue_family_index";
    static constexpr std::string_view
        g_createVulkanPresentQueueFamilyIndexNumber =
            "SDL.renderer.create.vulkan.present_queue_family_index";

    renderer() = delete;

    // Create a 2D rendering context for a window.
    //
    // If you want a specific renderer, you can specify its name here. A list of
    // available renderers can be obtained by calling SDL_GetRenderDriver()
    // multiple times, with indices from 0 to SDL_GetNumRenderDrivers()-1. If
    // you don't need a specific renderer, specify NULL and SDL will attempt to
    // choose the best option for you, based on what is available on the user's
    // system.
    //
    // If `name` is a comma-separated list, SDL will try each name, in the order
    // listed, until one succeeds or all of them fail.
    //
    // By default the rendering size matches the window size in pixels, but you
    // can call SDL_SetRenderLogicalPresentation() to change the content size
    // and scaling options.
    //
    // Should only be called on the main thread.
    renderer( window_t _window, std::string_view _name )
        : _data( SDL_CreateRenderer( _window, std::string( _name ).c_str() ) ) {
    }

    // Create a 2D rendering context for a window, with the specified
    // properties.
    //
    // These are the supported properties:
    //
    // - `SDL_PROP_RENDERER_CREATE_NAME_STRING`: the name of the rendering
    // driver
    //   to use, if a specific one is desired
    // - `SDL_PROP_RENDERER_CREATE_WINDOW_POINTER`: the window where rendering
    // is
    //   displayed, required if this isn't a software renderer using a surface
    // - `SDL_PROP_RENDERER_CREATE_SURFACE_POINTER`: the surface where rendering
    //   is displayed, if you want a software renderer without a window
    // - `SDL_PROP_RENDERER_CREATE_OUTPUT_COLORSPACE_NUMBER`: an SDL_Colorspace
    //   value describing the colorspace for output to the display, defaults to
    //   SDL_COLORSPACE_SRGB. The direct3d11, direct3d12, and metal renderers
    //   support SDL_COLORSPACE_SRGB_LINEAR, which is a linear color space and
    //   supports HDR output. If you select SDL_COLORSPACE_SRGB_LINEAR, drawing
    //   still uses the sRGB colorspace, but values can go beyond 1.0 and float
    //   (linear) format textures can be used for HDR content.
    // - `SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER`: non-zero if you want
    //   present synchronized with the refresh rate. This property can take any
    //   value that is supported by SDL_SetRenderVSync() for the renderer.
    //
    // With the SDL GPU renderer:
    //
    // - `SDL_PROP_RENDERER_CREATE_GPU_SHADERS_SPIRV_BOOLEAN`: the app is able
    // to
    //   provide SPIR-V shaders to SDL_GPURenderState, optional.
    // - `SDL_PROP_RENDERER_CREATE_GPU_SHADERS_DXIL_BOOLEAN`: the app is able to
    //   provide DXIL shaders to SDL_GPURenderState, optional.
    // - `SDL_PROP_RENDERER_CREATE_GPU_SHADERS_MSL_BOOLEAN`: the app is able to
    //   provide MSL shaders to SDL_GPURenderState, optional.
    //
    // With the vulkan renderer:
    //
    // - `SDL_PROP_RENDERER_CREATE_VULKAN_INSTANCE_POINTER`: the VkInstance to
    // use
    //   with the renderer, optional.
    // - `SDL_PROP_RENDERER_CREATE_VULKAN_SURFACE_NUMBER`: the VkSurfaceKHR to
    // use
    //   with the renderer, optional.
    // - `SDL_PROP_RENDERER_CREATE_VULKAN_PHYSICAL_DEVICE_POINTER`: the
    //   VkPhysicalDevice to use with the renderer, optional.
    // - `SDL_PROP_RENDERER_CREATE_VULKAN_DEVICE_POINTER`: the VkDevice to use
    //   with the renderer, optional.
    // - `SDL_PROP_RENDERER_CREATE_VULKAN_GRAPHICS_QUEUE_FAMILY_INDEX_NUMBER`:
    // the
    //   queue family index used for rendering.
    // - `SDL_PROP_RENDERER_CREATE_VULKAN_PRESENT_QUEUE_FAMILY_INDEX_NUMBER`:
    // the
    //   queue family index used for presentation.
    //
    // Should only be called on the main thread.
    renderer( SDL_PropertiesID _properties )
        : _data( SDL_CreateRendererWithProperties( _properties ) ) {}

    renderer( const renderer& ) = default;
    renderer( renderer&& ) = default;

    template < typename OtherType >
        requires std::is_convertible_v< OtherType, SDL_Renderer* >
    constexpr renderer( OtherType&& _other )
        : _data( std::forward< OtherType >( _other ) ) {}

    ~renderer() { SDL_DestroyRenderer( _data ); }

    auto operator=( const renderer& ) -> renderer& = default;
    auto operator=( renderer&& ) -> renderer& = default;

    constexpr operator SDL_Renderer*() const { return ( _data ); }

    // Variables
private:
    gsl::not_null< SDL_Renderer* > _data;
};

// Texture
using texture_t = struct texture {
    texture() = delete;

    // The contents of a texture when first created are not defined.
    //
    // Should only be called on the main thread.
    texture( const renderer_t& _renderer,
             pixels::format_t _format,
             access_t _access,
             volume_t _volume )
        : _data( SDL_CreateTexture( _renderer,
                                    pixels::toLegacy( _format ),
                                    toLegacy( _access ),
                                    _volume.width,
                                    _volume.height ) ) {}

    // The surface is not modified or freed by this function.
    //
    // The access_t hint for the created texture is
    // `access_t::aStatic`.
    //
    // The pixel format of the created texture may be different from the pixel
    // format of the surface, and can be queried using the
    // SDL_PROP_TEXTURE_FORMAT_NUMBER property.
    //
    // Should only be called on the main thread.
    texture( const renderer_t& _renderer, const surface_t& _surface )
        : _data( SDL_CreateTextureFromSurface( _renderer, _surface ) ) {}

    // These are the supported properties:
    // - `SDL_PROP_TEXTURE_CREATE_COLORSPACE_NUMBER`: an SDL_Colorspace value
    //   describing the texture colorspace, defaults to
    //   SDL_COLORSPACE_SRGB_LINEAR for floating point textures,
    //   SDL_COLORSPACE_HDR10 for 10-bit textures, SDL_COLORSPACE_SRGB for other
    //   RGB textures and SDL_COLORSPACE_JPEG for YUV textures.
    // - `SDL_PROP_TEXTURE_CREATE_FORMAT_NUMBER`: one of the enumerated values
    // in
    //   SDL_PixelFormat, defaults to the best RGBA format for the renderer
    // - `SDL_PROP_TEXTURE_CREATE_ACCESS_NUMBER`: one of the enumerated values
    // in
    //   access_t, defaults to access_t::aStatic
    // - `SDL_PROP_TEXTURE_CREATE_WIDTH_NUMBER`: the width of the texture in
    //   pixels, required
    // - `SDL_PROP_TEXTURE_CREATE_HEIGHT_NUMBER`: the height of the texture in
    //   pixels, required
    // - `SDL_PROP_TEXTURE_CREATE_SDR_WHITE_POINT_FLOAT`: for HDR10 and floating
    //   point textures, this defines the value of 100% diffuse white, with
    //   higher values being displayed in the High Dynamic Range headroom. This
    //   defaults to 100 for HDR10 textures and 1.0 for floating point textures.
    // - `SDL_PROP_TEXTURE_CREATE_HDR_HEADROOM_FLOAT`: for HDR10 and floating
    //   point textures, this defines the maximum dynamic range used by the
    //   content, in terms of the SDR white point. This would be equivalent to
    //   maxCLL / SDL_PROP_TEXTURE_CREATE_SDR_WHITE_POINT_FLOAT for HDR10
    //   content. If this is defined, any values outside the range supported by
    //   the display will be scaled into the available HDR headroom, otherwise
    //   they are clipped.
    //
    // With the direct3d11 renderer:
    //
    // - `SDL_PROP_TEXTURE_CREATE_D3D11_TEXTURE_POINTER`: the ID3D11Texture2D
    //   associated with the texture, if you want to wrap an existing texture.
    // - `SDL_PROP_TEXTURE_CREATE_D3D11_TEXTURE_U_POINTER`: the ID3D11Texture2D
    //   associated with the U plane of a YUV texture, if you want to wrap an
    //   existing texture.
    // - `SDL_PROP_TEXTURE_CREATE_D3D11_TEXTURE_V_POINTER`: the ID3D11Texture2D
    //   associated with the V plane of a YUV texture, if you want to wrap an
    //   existing texture.
    //
    // With the direct3d12 renderer:
    //
    // - `SDL_PROP_TEXTURE_CREATE_D3D12_TEXTURE_POINTER`: the ID3D12Resource
    //   associated with the texture, if you want to wrap an existing texture.
    // - `SDL_PROP_TEXTURE_CREATE_D3D12_TEXTURE_U_POINTER`: the ID3D12Resource
    //   associated with the U plane of a YUV texture, if you want to wrap an
    //   existing texture.
    // - `SDL_PROP_TEXTURE_CREATE_D3D12_TEXTURE_V_POINTER`: the ID3D12Resource
    //   associated with the V plane of a YUV texture, if you want to wrap an
    //   existing texture.
    //
    // With the metal renderer:
    //
    // - `SDL_PROP_TEXTURE_CREATE_METAL_PIXELBUFFER_POINTER`: the
    // CVPixelBufferRef
    //   associated with the texture, if you want to create a texture from an
    //   existing pixel buffer.
    //
    // With the opengl renderer:
    //
    // - `SDL_PROP_TEXTURE_CREATE_OPENGL_TEXTURE_NUMBER`: the GLuint texture
    //   associated with the texture, if you want to wrap an existing texture.
    // - `SDL_PROP_TEXTURE_CREATE_OPENGL_TEXTURE_UV_NUMBER`: the GLuint texture
    //   associated with the UV plane of an NV12 texture, if you want to wrap an
    //   existing texture.
    // - `SDL_PROP_TEXTURE_CREATE_OPENGL_TEXTURE_U_NUMBER`: the GLuint texture
    //   associated with the U plane of a YUV texture, if you want to wrap an
    //   existing texture.
    // - `SDL_PROP_TEXTURE_CREATE_OPENGL_TEXTURE_V_NUMBER`: the GLuint texture
    //   associated with the V plane of a YUV texture, if you want to wrap an
    //   existing texture.
    //
    // With the opengles2 renderer:
    //
    // - `SDL_PROP_TEXTURE_CREATE_OPENGLES2_TEXTURE_NUMBER`: the GLuint texture
    //   associated with the texture, if you want to wrap an existing texture.
    // - `SDL_PROP_TEXTURE_CREATE_OPENGLES2_TEXTURE_NUMBER`: the GLuint texture
    //   associated with the texture, if you want to wrap an existing texture.
    // - `SDL_PROP_TEXTURE_CREATE_OPENGLES2_TEXTURE_UV_NUMBER`: the GLuint
    // texture
    //   associated with the UV plane of an NV12 texture, if you want to wrap an
    //   existing texture.
    // - `SDL_PROP_TEXTURE_CREATE_OPENGLES2_TEXTURE_U_NUMBER`: the GLuint
    // texture
    //   associated with the U plane of a YUV texture, if you want to wrap an
    //   existing texture.
    // - `SDL_PROP_TEXTURE_CREATE_OPENGLES2_TEXTURE_V_NUMBER`: the GLuint
    // texture
    //   associated with the V plane of a YUV texture, if you want to wrap an
    //   existing texture.
    //
    // With the vulkan renderer:
    //
    // - `SDL_PROP_TEXTURE_CREATE_VULKAN_TEXTURE_NUMBER`: the VkImage with
    // layout
    //   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL associated with the texture,
    //   if you want to wrap an existing texture.
    //
    // Should only be called on the main thread.
    texture( const renderer_t& _renderer, SDL_PropertiesID _props )
        : _data( SDL_CreateTextureWithProperties( _renderer, _props ) ) {}

    texture( const texture& ) = default;
    texture( texture&& ) = default;

    template < typename OtherType >
        requires std::is_convertible_v< OtherType, SDL_Texture* >
    constexpr texture( OtherType&& _other )
        : _data( std::forward< OtherType >( _other ) ) {}

    ~texture() { SDL_DestroyTexture( _data ); }

    auto operator=( const texture& ) -> texture& = default;
    auto operator=( texture&& ) -> texture& = default;

    constexpr operator SDL_Texture*() const { return ( _data ); }

    static constexpr std::string_view g_colorspaceNumber =
        "SDL.texture.colorspace";
    static constexpr std::string_view g_formatNumber = "SDL.texture.format";
    static constexpr std::string_view g_accessNumber = "SDL.texture.access";
    static constexpr std::string_view g_widthNumber = "SDL.texture.width";
    static constexpr std::string_view g_heightNumber = "SDL.texture.height";
    static constexpr std::string_view g_whitePointFloatSDR =
        "SDL.texture.SDR_white_point";
    static constexpr std::string_view g_headroomFloatHDR =
        "SDL.texture.HDR_headroom";
    static constexpr std::string_view g_d3d11TexturePointer =
        "SDL.texture.d3d11.texture";
    static constexpr std::string_view g_d3d11TextureUPointer =
        "SDL.texture.d3d11.texture_u";
    static constexpr std::string_view g_d3d11TextureVPointer =
        "SDL.texture.d3d11.texture_v";
    static constexpr std::string_view g_d3d12TexturePointer =
        "SDL.texture.d3d12.texture";
    static constexpr std::string_view g_d3d12TextureUPointer =
        "SDL.texture.d3d12.texture_u";
    static constexpr std::string_view g_d3d12TextureVPointer =
        "SDL.texture.d3d12.texture_v";
    static constexpr std::string_view g_openGLTextureNumber =
        "SDL.texture.opengl.texture";
    static constexpr std::string_view g_openGLTextureUvNumber =
        "SDL.texture.opengl.texture_uv";
    static constexpr std::string_view g_openGLTextureUNumber =
        "SDL.texture.opengl.texture_u";
    static constexpr std::string_view g_openGLTextureVNumber =
        "SDL.texture.opengl.texture_v";
    static constexpr std::string_view g_openGLTextureTargetNumber =
        "SDL.texture.opengl.target";
    static constexpr std::string_view g_openGLTexWFloat =
        "SDL.texture.opengl.tex_w";
    static constexpr std::string_view g_openGLTexHFloat =
        "SDL.texture.opengl.tex_h";
    static constexpr std::string_view g_openGLES2TextureNumber =
        "SDL.texture.opengles2.texture";
    static constexpr std::string_view g_openGLES2TextureUvNumber =
        "SDL.texture.opengles2.texture_uv";
    static constexpr std::string_view g_openGLES2TextureUNumber =
        "SDL.texture.opengles2.texture_u";
    static constexpr std::string_view g_openGLES2TextureVNumber =
        "SDL.texture.opengles2.texture_v";
    static constexpr std::string_view g_openGLES2TextureTargetNumber =
        "SDL.texture.opengles2.target";
    static constexpr std::string_view g_vulkanTextureNumber =
        "SDL.texture.vulkan.texture";

    // Get the properties associated with a texture.
    //
    // The following read-only properties are provided by SDL:
    // - `SDL_PROP_TEXTURE_COLORSPACE_NUMBER`: an SDL_Colorspace value
    // describing
    //   the texture colorspace.
    // - `SDL_PROP_TEXTURE_FORMAT_NUMBER`: one of the enumerated values in
    //   SDL_PixelFormat.
    // - `SDL_PROP_TEXTURE_ACCESS_NUMBER`: one of the enumerated values in
    //   access_t.
    // - `SDL_PROP_TEXTURE_WIDTH_NUMBER`: the width of the texture in pixels.
    // - `SDL_PROP_TEXTURE_HEIGHT_NUMBER`: the height of the texture in pixels.
    // - `SDL_PROP_TEXTURE_SDR_WHITE_POINT_FLOAT`: for HDR10 and floating point
    //   textures, this defines the value of 100% diffuse white, with higher
    //   values being displayed in the High Dynamic Range headroom. This
    //   defaults to 100 for HDR10 textures and 1.0 for other textures.
    // - `SDL_PROP_TEXTURE_HDR_HEADROOM_FLOAT`: for HDR10 and floating point
    //   textures, this defines the maximum dynamic range used by the content,
    //   in terms of the SDR white point. If this is defined, any values outside
    //   the range supported by the display will be scaled into the available
    //   HDR headroom, otherwise they are clipped. This defaults to 1.0 for SDR
    //   textures, 4.0 for HDR10 textures, and no default for floating point
    //   textures.
    //
    // With the direct3d11 renderer:
    //
    // - `SDL_PROP_TEXTURE_D3D11_TEXTURE_POINTER`: the ID3D11Texture2D
    // associated
    //   with the texture
    // - `SDL_PROP_TEXTURE_D3D11_TEXTURE_U_POINTER`: the ID3D11Texture2D
    //   associated with the U plane of a YUV texture
    // - `SDL_PROP_TEXTURE_D3D11_TEXTURE_V_POINTER`: the ID3D11Texture2D
    //   associated with the V plane of a YUV texture
    //
    // With the direct3d12 renderer:
    //
    // - `SDL_PROP_TEXTURE_D3D12_TEXTURE_POINTER`: the ID3D12Resource associated
    //   with the texture
    // - `SDL_PROP_TEXTURE_D3D12_TEXTURE_U_POINTER`: the ID3D12Resource
    // associated
    //   with the U plane of a YUV texture
    // - `SDL_PROP_TEXTURE_D3D12_TEXTURE_V_POINTER`: the ID3D12Resource
    // associated
    //   with the V plane of a YUV texture
    //
    // With the vulkan renderer:
    //
    // - `SDL_PROP_TEXTURE_VULKAN_TEXTURE_NUMBER`: the VkImage associated with
    // the
    //   texture
    //
    // With the opengl renderer:
    //
    // - `SDL_PROP_TEXTURE_OPENGL_TEXTURE_NUMBER`: the GLuint texture associated
    //   with the texture
    // - `SDL_PROP_TEXTURE_OPENGL_TEXTURE_UV_NUMBER`: the GLuint texture
    //   associated with the UV plane of an NV12 texture
    // - `SDL_PROP_TEXTURE_OPENGL_TEXTURE_U_NUMBER`: the GLuint texture
    // associated
    //   with the U plane of a YUV texture
    // - `SDL_PROP_TEXTURE_OPENGL_TEXTURE_V_NUMBER`: the GLuint texture
    // associated
    //   with the V plane of a YUV texture
    // - `SDL_PROP_TEXTURE_OPENGL_TEXTURE_TARGET_NUMBER`: the GLenum for the
    //   texture target (`GL_TEXTURE_2D`, `GL_TEXTURE_RECTANGLE_ARB`, etc)
    // - `SDL_PROP_TEXTURE_OPENGL_TEX_W_FLOAT`: the texture coordinate width of
    //   the texture (0.0 - 1.0)
    // - `SDL_PROP_TEXTURE_OPENGL_TEX_H_FLOAT`: the texture coordinate height of
    //   the texture (0.0 - 1.0)
    //
    // With the opengles2 renderer:
    //
    // - `SDL_PROP_TEXTURE_OPENGLES2_TEXTURE_NUMBER`: the GLuint texture
    //   associated with the texture
    // - `SDL_PROP_TEXTURE_OPENGLES2_TEXTURE_UV_NUMBER`: the GLuint texture
    //   associated with the UV plane of an NV12 texture
    // - `SDL_PROP_TEXTURE_OPENGLES2_TEXTURE_U_NUMBER`: the GLuint texture
    //   associated with the U plane of a YUV texture
    // - `SDL_PROP_TEXTURE_OPENGLES2_TEXTURE_V_NUMBER`: the GLuint texture
    //   associated with the V plane of a YUV texture
    // - `SDL_PROP_TEXTURE_OPENGLES2_TEXTURE_TARGET_NUMBER`: the GLenum for the
    //   texture target (`GL_TEXTURE_2D`, `GL_TEXTURE_EXTERNAL_OES`, etc)
    [[nodiscard]] auto properties() const -> SDL_PropertiesID {
        const SDL_PropertiesID l_properties = SDL_GetTextureProperties( _data );

        slickdl::assert( l_properties );

        return ( l_properties );
    }

    [[nodiscard]] auto renderer() const -> renderer_t {
        return ( SDL_GetRendererFromTexture( _data ) );
    }

    // Should only be called on the main thread.
    [[nodiscard]] constexpr auto volume() const -> volume_t {
        stdfunc::assert( _data->w );
        stdfunc::assert( _data->h );

        return {
            static_cast< float >( _data->w ),
            static_cast< float >( _data->h ),
        };
    }

    // When this texture is rendered, during the copy operation each source
    // color channel is modulated by the appropriate color value according to
    // the following formula:
    //
    // `srcC = srcC * (color / 255)`
    //
    // Color modulation is not always supported by the renderer; it will
    // assert if color modulation is not supported.
    //
    // When this texture is rendered, during the copy operation the source alpha
    // value is modulated by this alpha value according to the following
    // formula:
    //
    // `srcA = srcA * (alpha / 255)`
    //
    // Alpha modulation is not always supported by the renderer; it will
    // assert if alpha modulation is not supported.
    //
    // Should only be called on the main thread.
    void modulator( color_t _color ) {
        bool l_result = SDL_SetTextureColorMod( _data, _color.red, _color.green,
                                                _color.blue );

        slickdl::assert( l_result );

        l_result = SDL_SetTextureAlphaMod( _data, _color.alpha );

        slickdl::assert( l_result );
    }

#if 0
    void colorModulate(color_t<float> _color);
#endif

    // Should only be called on the main thread.
    [[nodiscard]] auto modulator() const -> color_t {
        color_t l_color;

        bool l_result = SDL_GetTextureColorMod( _data, &l_color.red,
                                                &l_color.green, &l_color.blue );

        slickdl::assert( l_result );

        l_result = SDL_GetTextureAlphaMod( _data, &l_color.alpha );

        slickdl::assert( l_result );

        return ( l_color );
    }

#if 0
    [[nodiscard]] auto colorModulator() const -> color_t<float> {]
#endif

    // If the blend mode is not supported, the closest supported mode is chosen
    // and this function asserts.
    //
    // Should only be called on the main thread.
    void blend( blend_t _blend ) {
        const bool l_result =
            SDL_SetTextureBlendMode( _data, toLegacy( _blend ) );

        slickdl::assert( l_result );
    }

    // Should only be called on the main thread.
    [[nodiscard]] auto blend() const -> blend_t {
        blend_t l_blend = blend_t::none;

        const bool l_result = SDL_GetTextureBlendMode(
            _data, std::bit_cast< SDL_BlendMode* >( &l_blend ) );

        slickdl::assert( l_result );

        return ( l_blend );
    }

    // If the scale mode is not supported, the closest supported mode is chosen.
    //
    // Should only be called on the main thread.
    void scale( scale_t _scale ) {
        const bool l_result =
            SDL_SetTextureScaleMode( _data, toLegacy( _scale ) );

        slickdl::assert( l_result );
    }

    // Should only be called on the main thread.
    [[nodiscard]] auto scale() const -> scale_t {
        scale_t l_scale = scale_t::linear;

        const bool l_result = SDL_GetTextureScaleMode(
            _data, std::bit_cast< SDL_ScaleMode* >( &l_scale ) );

        slickdl::assert( l_result );

        return ( l_scale );
    }

    // Update the given texture rectangle with new pixel data.
    //
    // The pixel data must be in the pixel format of the texture, which can be
    // queried using the SDL_PROP_TEXTURE_FORMAT_NUMBER property.
    //
    // This is a fairly slow function, intended for use with static textures
    // that do not change often.
    //
    // If the texture is intended to be updated often, it is preferred to create
    // the texture as streaming and use the locking functions referenced below.
    // While this function will work with streaming textures, for optimization
    // reasons you may not get the pixels back if you lock the texture
    // afterward.
    //
    // _pixels should include padding
    //
    // Should only be called on the main thread.
    void update(
        std::span< const std::byte > _pixels,
        const std::optional< const box_t< int > >& _box = std::nullopt ) {
        const std::vector< uint8_t > l_pixels =
            stdfunc::spanToVector< std::byte, uint8_t >( _pixels );

        const SDL_Rect l_box = _box.value();

        const bool l_result = SDL_UpdateTexture( _data, &l_box, l_pixels.data(),
                                                 l_pixels.size() );

        slickdl::assert( l_result );
    }

    // You can use SDL_UpdateTexture() as long as your pixel data is a
    // contiguous block of Y and U/V planes in the proper order, but this
    // function is available if your pixel data is not contiguous.
    //
    // Size is the number of bytes between rows of pixel data for the Y plane.
    //
    // Should only be called on the main thread.
    void update(
        std::pair< std::span< const std::byte >, size_t > _pixelsY,
        std::pair< std::span< const std::byte >, size_t > _pixelsU,
        std::pair< std::span< const std::byte >, size_t > _pixelsV,
        const std::optional< const box_t< int > >& _box = std::nullopt ) {
        const std::vector< uint8_t > l_pixelsY =
            stdfunc::spanToVector< std::byte, uint8_t >( _pixelsY.first );
        const std::vector< uint8_t > l_pixelsU =
            stdfunc::spanToVector< std::byte, uint8_t >( _pixelsU.first );
        const std::vector< uint8_t > l_pixelsV =
            stdfunc::spanToVector< std::byte, uint8_t >( _pixelsV.first );

        const SDL_Rect l_box = _box.value();

        const bool l_result = SDL_UpdateYUVTexture(
            _data, &l_box, l_pixelsY.data(), _pixelsY.second, l_pixelsU.data(),
            _pixelsU.second, l_pixelsV.data(), _pixelsV.second );

        slickdl::assert( l_result );
    }

    // You can use SDL_UpdateTexture() as long as your pixel data is a
    // contiguous block of NV12/21 planes in the proper order, but this function
    // is available if your pixel data is not contiguous.
    //
    // Size is the number of bytes between rows of pixel data for the Y plane.
    //
    // Should only be called on the main thread.
    void update(
        std::pair< std::span< const std::byte >, size_t > _pixelsY,
        std::pair< std::span< const std::byte >, size_t > _pixelsU,
        const std::optional< const box_t< int > >& _box = std::nullopt ) {
        const std::vector< uint8_t > l_pixelsY =
            stdfunc::spanToVector< std::byte, uint8_t >( _pixelsY.first );
        const std::vector< uint8_t > l_pixelsU =
            stdfunc::spanToVector< std::byte, uint8_t >( _pixelsU.first );

        const SDL_Rect l_box = _box.value();

        const bool l_result = SDL_UpdateNVTexture(
            _data, &l_box, l_pixelsY.data(), _pixelsY.second, l_pixelsU.data(),
            _pixelsU.second );

        slickdl::assert( l_result );
    }

    // Lock a portion of the texture for **write-only** pixel access.
    //
    // As an optimization, the pixels made available for editing don't
    // necessarily contain the old texture data. This is a write-only operation,
    // and if you need to keep a copy of the texture data you should do that at
    // the application level.
    //
    // You must use SDL_UnlockTexture() to unlock the pixels and apply any
    // changes.
    //
    // TODO: What is this
    // \param pixels this is filled in with a pointer to the locked pixels,
    // appropriately offset by the locked area.
    //
    // Asserts if the texture is not valid or was not created with
    // `SDL_TEXTUREACCESS_STREAMING`
    //
    // Should only be called on the main thread.
    [[nodiscard]] auto lock(
        const std::optional< const box_t< int > >& _box = std::nullopt )
        -> std::pair< std::span< const std::byte >, size_t > {
        const SDL_Rect l_box = _box.value();

        std::pair< std::span< const std::byte >, size_t > l_pixels;

        {
            std::byte* l_temp = nullptr;

            const bool l_result = SDL_LockTexture(
                _data, &l_box, std::bit_cast< void** >( &l_temp ),
                std::bit_cast< int* >( &l_pixels.second ) );

            slickdl::assert( l_result );

            const volume_t l_volume = volume();

            l_pixels.first = {
                l_temp,
                static_cast< size_t >( ( l_volume.width * l_volume.height ) ) };
        }

        return ( l_pixels );
    }

    // Lock a portion of the texture for **write-only** pixel access, and expose
    // it as a SDL surface.
    //
    // Besides providing an SDL_Surface instead of raw pixel data, this function
    // operates like SDL_LockTexture.
    //
    // As an optimization, the pixels made available for editing don't
    // necessarily contain the old texture data. This is a write-only operation,
    // and if you need to keep a copy of the texture data you should do that at
    // the application level.
    //
    // You must use SDL_UnlockTexture() to unlock the pixels and apply any
    // changes.
    //
    // The returned surface is freed internally after calling
    // SDL_UnlockTexture() or SDL_DestroyTexture(). The caller should not free
    // it.
    //
    // Surface of size **_box**. Don't assume any specific pixel content.
    //
    // Asserts if the texture is not valid or was not created with
    // `SDL_TEXTUREACCESS_STREAMING`
    //
    // Should only be called on the main thread.
    void lock(
        surface_t& _surface,
        const std::optional< const box_t< int > >& _box = std::nullopt ) {
        const SDL_Rect l_box = _box.value();

        SDL_Surface* l_surface = _surface;

        const bool l_result =
            SDL_LockTextureToSurface( _data, &l_box, &l_surface );

        slickdl::assert( l_result );

        _surface = l_surface;
    }

    // Unlock a texture, uploading the changes to video memory, if needed.
    //
    // **Warning**: Please note that SDL_LockTexture() is intended to be
    // write-only; it will not guarantee the previous contents of the texture
    // will be provided. You must fully initialize any area of a texture that
    // you lock before unlocking it, as the pixels might otherwise be
    // uninitialized memory.
    //
    // Which is to say: locking and immediately unlocking a texture can result
    // in corrupted textures, depending on the renderer in use.
    //
    // Should only be called on the main thread.
    void unlock() { SDL_UnlockTexture( _data ); }

    // Variables
private:
    gsl::not_null< SDL_Texture* > _data;
};

// Create a window and default renderer.
//
// Should only be called on the main thread
[[nodiscard]] auto windowAndRenderer( std::string_view _title,
                                      volume_t _volume,
                                      SDL_WindowFlags _windowFlags )
    -> std::pair< window_t, renderer_t > {
    SDL_Window* l_window = nullptr;
    SDL_Renderer* l_renderer = nullptr;

    const bool l_result = SDL_CreateWindowAndRenderer(
        std::string( _title ).c_str(), _volume.width, _volume.height,
        _windowFlags, &l_window, &l_renderer );

    slickdl::assert( l_result );

    return { l_window, l_renderer };
}

} // namespace slickdl
