#pragma once

#include <SDL3/SDL_render.h>
#include <sys/types.h>

#include <cstdint>
#include <gsl/pointers>

#include "slickdl.hpp"
#include "slickdl/blend.hpp"
#include "slickdl/line_box.hpp"
#include "slickdl/pixels_palette.hpp"
#include "slickdl/surface.hpp"
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
[[nodiscard]] inline auto renderDrivers() -> size_t {
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
[[nodiscard]] inline auto renderDriver( size_t _index ) -> std::string_view {
    const char* l_name = SDL_GetRenderDriver( _index );

    assert( l_name );

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

#if 0
    // TODO: Implement
    // Create a 2D GPU rendering context for a window, with support for the
    // specified shader format.
    //
    // This is a convenience function to create a SDL GPU backed renderer,
    // intended to be used with SDL_GPURenderState. The resulting renderer will
    // support shaders in one of the specified shader formats.
    //
    // If no available GPU driver supports any of the specified shader formats,
    // this function will fail.
    //
    // \param device a pointer filled with the associated GPU device, or NULL on
    //               error.
    // \returns a valid rendering context or NULL if there was an error; call
    //          SDL_GetError() for more information.
    //
    // Should only be called on the main thread.
    renderer( window_t _window, SDL_GPUShaderFormat _formatFlags ) {
        SDL_GPUDevice* l_GPUDevice = nullptr;

        const bool l_result =
            SDL_CreateGPURenderer( _window, _formatFlags, &l_GPUDevice );

        assert( l_result );

        _GPUDevice = l_GPUDevice;
    }
#endif

    // Create a 2D software rendering context for a surface.
    //
    // Two other API which can be used to create SDL_Renderer:
    // SDL_CreateRenderer() and SDL_CreateWindowAndRenderer(). These can _also_
    // create a software renderer, but they are intended to be used with an
    // SDL_Window as the final destination and not an SDL_Surface.
    //
    // \param surface the SDL_Surface structure representing the surface where
    //                rendering is done.
    //
    // Should only be called on the main thread.
    renderer( surface_t& _surface )
        : _data( SDL_CreateSoftwareRenderer( _surface ) ) {}

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

    [[nodiscard]] auto window() const -> window_t {
        return ( SDL_GetRenderWindow( _data ) );
    }

    [[nodiscard]] auto name() const -> std::string_view {
        return ( SDL_GetRendererName( _data ) );
    }

    static constexpr std::string_view g_nameString = "SDL.renderer.name";
    static constexpr std::string_view g_windowPointer = "SDL.renderer.window";
    static constexpr std::string_view g_surfacePointer = "SDL.renderer.surface";
    static constexpr std::string_view g_vsyncNumber = "SDL.renderer.vsync";
    static constexpr std::string_view g_maxTextureSizeNumber =
        "SDL.renderer.max_texture_size";
    static constexpr std::string_view g_textureFormatsPointer =
        "SDL.renderer.texture_formats";
    static constexpr std::string_view g_outputColorspaceNumber =
        "SDL.renderer.output_colorspace";
    static constexpr std::string_view g_enabledBooleanHDR =
        "SDL.renderer.HDR_enabled";
    static constexpr std::string_view g_whitePointFloatSDR =
        "SDL.renderer.SDR_white_point";
    static constexpr std::string_view g_headroomFloatHDR =
        "SDL.renderer.HDR_headroom";
    static constexpr std::string_view g_d3D9DevicePointer =
        "SDL.renderer.d3d9.device";
    static constexpr std::string_view g_d3D11DevicePointer =
        "SDL.renderer.d3d11.device";
    static constexpr std::string_view g_d3D11SwapchainPointer =
        "SDL.renderer.d3d11.swap_chain";
    static constexpr std::string_view g_d3D12DevicePointer =
        "SDL.renderer.d3d12.device";
    static constexpr std::string_view g_d3D12SwapchainPointer =
        "SDL.renderer.d3d12.swap_chain";
    static constexpr std::string_view g_d3D12CommandQueuePointer =
        "SDL.renderer.d3d12.command_queue";
    static constexpr std::string_view g_vulkanInstancePointer =
        "SDL.renderer.vulkan.instance";
    static constexpr std::string_view g_vulkanSurfaceNumber =
        "SDL.renderer.vulkan.surface";
    static constexpr std::string_view g_vulkanPhysicalDevicePointer =
        "SDL.renderer.vulkan.physical_device";
    static constexpr std::string_view g_vulkanDevicePointer =
        "SDL.renderer.vulkan.device";
    static constexpr std::string_view g_vulkanGraphicsQueueFamilyIndexNumber =
        "SDL.renderer.vulkan.graphics_queue_family_index";
    static constexpr std::string_view g_vulkanPresentQueueFamilyIndexNumber =
        "SDL.renderer.vulkan.present_queue_family_index";
    static constexpr std::string_view g_vulkanSwapchainImageCountNumber =
        "SDL.renderer.vulkan.swapchain_image_count";
    static constexpr std::string_view g_devicePointerGPU =
        "SDL.renderer.gpu.device";

    // The following read-only properties are provided by SDL:
    //
    // - `SDL_PROP_RENDERER_NAME_STRING`: the name of the rendering driver
    // - `SDL_PROP_RENDERER_WINDOW_POINTER`: the window where rendering is
    //   displayed, if any
    // - `SDL_PROP_RENDERER_SURFACE_POINTER`: the surface where rendering is
    //   displayed, if this is a software renderer without a window
    // - `SDL_PROP_RENDERER_VSYNC_NUMBER`: the current vsync setting
    // - `SDL_PROP_RENDERER_MAX_TEXTURE_SIZE_NUMBER`: the maximum texture width
    //   and height
    // - `SDL_PROP_RENDERER_TEXTURE_FORMATS_POINTER`: a (const SDL_PixelFormat
    // *)
    //   array of pixel formats, terminated with SDL_PIXELFORMAT_UNKNOWN,
    //   representing the available texture formats for this renderer.
    // - `SDL_PROP_RENDERER_OUTPUT_COLORSPACE_NUMBER`: an SDL_Colorspace value
    //   describing the colorspace for output to the display, defaults to
    //   SDL_COLORSPACE_SRGB.
    // - `SDL_PROP_RENDERER_HDR_ENABLED_BOOLEAN`: true if the output colorspace
    // is
    //   SDL_COLORSPACE_SRGB_LINEAR and the renderer is showing on a display
    //   with HDR enabled. This property can change dynamically when
    //   SDL_EVENT_WINDOW_HDR_STATE_CHANGED is sent.
    // - `SDL_PROP_RENDERER_SDR_WHITE_POINT_FLOAT`: the value of SDR white in
    // the
    //   SDL_COLORSPACE_SRGB_LINEAR colorspace. When HDR is enabled, this value
    //   is automatically multiplied into the color scale. This property can
    //   change dynamically when SDL_EVENT_WINDOW_HDR_STATE_CHANGED is sent.
    // - `SDL_PROP_RENDERER_HDR_HEADROOM_FLOAT`: the additional high dynamic
    // range
    //   that can be displayed, in terms of the SDR white point. When HDR is not
    //   enabled, this will be 1.0. This property can change dynamically when
    //   SDL_EVENT_WINDOW_HDR_STATE_CHANGED is sent.
    //
    // With the direct3d renderer:
    //
    // - `SDL_PROP_RENDERER_D3D9_DEVICE_POINTER`: the IDirect3DDevice9
    // associated
    //   with the renderer
    //
    // With the direct3d11 renderer:
    //
    // - `SDL_PROP_RENDERER_D3D11_DEVICE_POINTER`: the ID3D11Device associated
    //   with the renderer
    // - `SDL_PROP_RENDERER_D3D11_SWAPCHAIN_POINTER`: the IDXGISwapChain1
    //   associated with the renderer. This may change when the window is
    //   resized.
    //
    // With the direct3d12 renderer:
    //
    // - `SDL_PROP_RENDERER_D3D12_DEVICE_POINTER`: the ID3D12Device associated
    //   with the renderer
    // - `SDL_PROP_RENDERER_D3D12_SWAPCHAIN_POINTER`: the IDXGISwapChain4
    //   associated with the renderer.
    // - `SDL_PROP_RENDERER_D3D12_COMMAND_QUEUE_POINTER`: the ID3D12CommandQueue
    //   associated with the renderer
    //
    // With the vulkan renderer:
    //
    // - `SDL_PROP_RENDERER_VULKAN_INSTANCE_POINTER`: the VkInstance associated
    //   with the renderer
    // - `SDL_PROP_RENDERER_VULKAN_SURFACE_NUMBER`: the VkSurfaceKHR associated
    //   with the renderer
    // - `SDL_PROP_RENDERER_VULKAN_PHYSICAL_DEVICE_POINTER`: the
    // VkPhysicalDevice
    //   associated with the renderer
    // - `SDL_PROP_RENDERER_VULKAN_DEVICE_POINTER`: the VkDevice associated with
    //   the renderer
    // - `SDL_PROP_RENDERER_VULKAN_GRAPHICS_QUEUE_FAMILY_INDEX_NUMBER`: the
    // queue
    //   family index used for rendering
    // - `SDL_PROP_RENDERER_VULKAN_PRESENT_QUEUE_FAMILY_INDEX_NUMBER`: the queue
    //   family index used for presentation
    // - `SDL_PROP_RENDERER_VULKAN_SWAPCHAIN_IMAGE_COUNT_NUMBER`: the number of
    //   swapchain images, or potential frames in flight, used by the Vulkan
    //   renderer
    //
    // With the gpu renderer:
    //
    // - `SDL_PROP_RENDERER_GPU_DEVICE_POINTER`: the SDL_GPUDevice associated
    // with
    //   the renderer
    //
    // \returns a valid property ID on success or 0 on failure; call
    //          SDL_GetError() for more information.
    [[nodiscard]] auto properties() const -> SDL_PropertiesID {
        return ( SDL_GetRendererProperties( _data ) );
    }

    // Get the output size in pixels of a rendering context.
    //
    // This returns the true output size in pixels, ignoring any render targets
    // or logical size and presentation.
    //
    // For the output size of the current rendering target, with logical size
    // adjustments, use SDL_GetCurrentRenderOutputSize() instead.
    //
    // Should only be called on the main thread.
    [[nodiscard]] auto outputSize() const -> volume_t {
        volume_t l_volume;

        const bool l_result = SDL_GetRenderOutputSize(
            _data, std::bit_cast< int* >( &l_volume.width ),
            std::bit_cast< int* >( &l_volume.height ) );

        assert( l_result );

        return ( l_volume );
    }

    // Get the current output size in pixels of a rendering context.
    //
    // If a rendering target is active, this will return the size of the
    // rendering target in pixels, otherwise return the value of
    // SDL_GetRenderOutputSize().
    //
    // Rendering target or not, the output will be adjusted by the current
    // logical presentation state, dictated by
    // SDL_SetRenderLogicalPresentation().
    //
    // Should only be called on the main thread.
    [[nodiscard]] auto currentOutputSize() const -> volume_t {
        volume_t l_volume;

        const bool l_result = SDL_GetCurrentRenderOutputSize(
            _data, std::bit_cast< int* >( &l_volume.width ),
            std::bit_cast< int* >( &l_volume.height ) );

        assert( l_result );

        return ( l_volume );
    }

#if 0
    // TODO: Implement
    // Create a texture for a rendering context.
    //
    // The contents of a texture when first created are not defined.
    //
    // Should only be called on the main thread.
    texture_t texture( pixels::format_t _format, texture_t::access_t _access, volume_t _volume);
#endif

    // Set a device-independent resolution and presentation mode for rendering.
    //
    // This function sets the width and height of the logical rendering output.
    // The renderer will act as if the current render target is always the
    // requested dimensions, scaling to the actual resolution as necessary.
    //
    // This can be useful for games that expect a fixed size, but would like to
    // scale the output to whatever is available, regardless of how a user
    // resizes a window, or if the display is high DPI.
    //
    // Logical presentation can be used with both render target textures and the
    // renderer's window; the state is unique to each render target, and this
    // function sets the state for the current render target. It might be useful
    // to draw to a texture that matches the window dimensions with logical
    // presentation enabled, and then draw that texture across the entire window
    // with logical presentation disabled. Be careful not to render both with
    // logical presentation enabled, however, as this could produce
    // double-letterboxing, etc.
    //
    // You can disable logical coordinates by setting the mode to
    // SDL_LOGICAL_PRESENTATION_DISABLED, and in that case you get the full
    // pixel resolution of the render target; it is safe to toggle logical
    // presentation during the rendering of a frame: perhaps most of the
    // rendering is done to specific dimensions but to make fonts look sharp,
    // the app turns off logical presentation while drawing text, for example.
    //
    // For the renderer's window, letterboxing is drawn into the framebuffer if
    // logical presentation is enabled during SDL_RenderPresent; be sure to
    // reenable it before presenting if you were toggling it, otherwise the
    // letterbox areas might have artifacts from previous frames (or artifacts
    // from external overlays, etc). Letterboxing is never drawn into texture
    // render targets; be sure to call SDL_RenderClear() before drawing into the
    // texture so the letterboxing areas are cleared, if appropriate.
    //
    // You can convert coordinates in an event into rendering coordinates using
    // SDL_ConvertEventToRenderCoordinates().
    //
    // Should only be called on the main thread.
    void logicalPresentation( volume_t _volume, logicalPresentation_t _mode ) {
        const bool l_result = SDL_SetRenderLogicalPresentation(
            _data, _volume.width, _volume.height, toLegacy( _mode ) );

        assert( l_result );
    }

    // Get device independent resolution and presentation mode for rendering.
    //
    // This function gets the width and height of the logical rendering output,
    // or the output size in pixels if a logical resolution is not enabled.
    //
    // Each render target has its own logical presentation state. This function
    // gets the state for the current render target.
    //
    // Should only be called on the main thread.
    [[nodiscard]] auto logicalPresentation() const
        -> std::pair< volume_t, logicalPresentation_t > {
        volume_t l_volume;
        logicalPresentation_t l_logicalPresentation =
            logicalPresentation_t::disabled;

        const bool l_result = SDL_GetRenderLogicalPresentation(
            _data, std::bit_cast< int* >( &l_volume.width ),
            std::bit_cast< int* >( &l_volume.height ),
            std::bit_cast< SDL_RendererLogicalPresentation* >(
                &l_logicalPresentation ) );

        assert( l_result );

        return { l_volume, l_logicalPresentation };
    }

    // Get the final presentation rectangle for rendering.
    //
    // This function returns the calculated rectangle used for logical
    // presentation, based on the presentation mode and output size. If logical
    // presentation is disabled, it will fill the rectangle with the output
    // size, in pixels.
    //
    // Each render target has its own logical presentation state. This function
    // gets the rectangle for the current render target.
    //
    // Should only be called on the main thread.
    [[nodiscard]] auto logicalPresentationFinal() const -> box_t< float > {
        SDL_FRect l_box;

        const bool l_result =
            SDL_GetRenderLogicalPresentationRect( _data, &l_box );

        assert( l_result );

        return { l_box };
    }

    // Get a point in render coordinates when given a point in window
    // coordinates.
    //
    // This takes into account several states:
    //
    // - The window dimensions.
    // - The logical presentation settings (SDL_SetRenderLogicalPresentation)
    // - The scale (SDL_SetRenderScale)
    // - The viewport (SDL_SetRenderViewport)
    //
    // Should only be called on the main thread.
    [[nodiscard]] auto coordinatesFromWindow(
        point_t< float > _windowPoint ) const -> point_t< float > {
        point_t< float > l_point;

        const bool l_result = SDL_RenderCoordinatesFromWindow(
            _data, _windowPoint.x, _windowPoint.y, &l_point.x, &l_point.y );

        assert( l_result );

        return ( l_point );
    }

    // Get a point in window coordinates when given a point in render
    // coordinates.
    //
    // This takes into account several states:
    //
    // - The window dimensions.
    // - The logical presentation settings (SDL_SetRenderLogicalPresentation)
    // - The scale (SDL_SetRenderScale)
    // - The viewport (SDL_SetRenderViewport)
    //
    // Should only be called on the main thread.
    [[nodiscard]] auto coordinatesToWindow(
        point_t< float > _renderPoint ) const -> point_t< float > {
        point_t< float > l_point;

        const bool l_result = SDL_RenderCoordinatesToWindow(
            _data, _renderPoint.x, _renderPoint.y, &l_point.x, &l_point.y );

        assert( l_result );

        return ( l_point );
    }

    // Convert the coordinates in an event to render coordinates.
    //
    // This takes into account several states:
    //
    // - The window dimensions.
    // - The logical presentation settings (SDL_SetRenderLogicalPresentation)
    // - The scale (SDL_SetRenderScale)
    // - The viewport (SDL_SetRenderViewport)
    //
    // Various event types are converted with this function: mouse, touch, pen,
    // etc.
    //
    // Touch coordinates are converted from normalized coordinates in the window
    // to non-normalized rendering coordinates.
    //
    // Relative mouse coordinates (xrel and yrel event fields) are _also_
    // converted. Applications that do not want these fields converted should
    // use SDL_RenderCoordinatesFromWindow() on the specific event fields
    // instead of converting the entire event structure.
    //
    // Once converted, coordinates may be outside the rendering area.
    //
    // Should only be called on the main thread.
    [[nodiscard]] auto coordinatesFromEvent() const -> event_t {
        event_t l_event;

        const bool l_result =
            SDL_ConvertEventToRenderCoordinates( _data, &l_event );

        assert( l_result );

        return ( l_event );
    }

    // Set the drawing area for rendering on the current target.
    //
    // Drawing will clip to this area (separately from any clipping done with
    // SDL_SetRenderClipRect), and the top left of the area will become
    // coordinate (0, 0) for future drawing commands.
    //
    // The area's width and height must be >= 0.
    //
    // Each render target has its own viewport. This function sets the viewport
    // for the current render target.
    //
    // Should only be called on the main thread.
    void viewport( const std::optional< box_t< int > >& _box ) {
        bool l_result = false;

        if ( _box ) {
            SDL_Rect l_box = _box.value();

            l_result = SDL_SetRenderViewport( _data, &l_box );

        } else {
            l_result = SDL_SetRenderViewport( _data, nullptr );
        }

        assert( l_result );
    }

    // Get the drawing area for the current target.
    //
    // Each render target has its own viewport. This function gets the viewport
    // for the current render target.
    //
    // Should only be called on the main thread.
    [[nodiscard]] auto viewport() const -> box_t< int > {
        SDL_Rect l_box;

        const bool l_result = SDL_GetRenderViewport( _data, &l_box );

        assert( l_result );

        return { l_box };
    }

    // Return whether an explicit rectangle was set as the viewport.
    //
    // This is useful if you're saving and restoring the viewport and want to
    // know whether you should restore a specific rectangle or NULL.
    //
    // Each render target has its own viewport. This function checks the
    // viewport for the current render target.
    //
    // Should only be called on the main thread.
    [[nodiscard]] auto isViewportLimited() const -> bool {
        return ( SDL_RenderViewportSet( _data ) );
    }

    // Get the safe area for rendering within the current viewport.
    //
    // Some devices have portions of the screen which are partially obscured or
    // not interactive, possibly due to on-screen controls, curved edges, camera
    // notches, TV overscan, etc. This function provides the area of the current
    // viewport which is safe to have interactible content. You should continue
    // rendering into the rest of the render target, but it should not contain
    // visually important or interactible content.
    //
    // Should only be called on the main thread.
    [[nodiscard]] auto safeArea() const -> box_t< int > {
        SDL_Rect l_box;

        const bool l_result = SDL_GetRenderSafeArea( _data, &l_box );

        assert( l_result );

        return { l_box };
    }

    // Set the clip rectangle for rendering on the specified target.
    //
    // Each render target has its own clip rectangle. This function sets the
    // cliprect for the current render target.
    //
    // Should only be called on the main thread.
    void clip( const std::optional< clippingZone_t< int > >& _zone ) {
        bool l_result = false;

        if ( _zone ) {
            const SDL_Rect l_zone = _zone.value();

            l_result = SDL_SetRenderClipRect( _data, &l_zone );

        } else {
            l_result = SDL_SetRenderClipRect( _data, nullptr );
        }

        assert( l_result );
    }

    // Get the clip rectangle for the current target.
    //
    // Each render target has its own clip rectangle. This function gets the
    // cliprect for the current render target.
    //
    // \param rect an SDL_Rect structure filled in with the current clipping
    // area or an empty rectangle if clipping is disabled.
    //
    // Should only be called on the main thread.
    [[nodiscard]] auto clip() const -> clippingZone_t< int > {
        SDL_Rect l_zone;

        const bool l_result = SDL_GetRenderClipRect( _data, &l_zone );

        assert( l_result );

        return { l_zone };
    }

    // Get whether clipping is enabled on the given render target.
    //
    // Each render target has its own clip rectangle. This function checks the
    // cliprect for the current render target.
    //
    // Should only be called on the main thread.
    [[nodiscard]] auto isClipEnabled() const -> bool {
        // TODO: Improve
        return ( SDL_RenderClipEnabled( _data ) );
    }

    // Set the drawing scale for rendering on the current target.
    //
    // The drawing coordinates are scaled by the x/y scaling factors before they
    // are used by the renderer. This allows resolution independent drawing with
    // a single coordinate system.
    //
    // If this results in scaling or subpixel drawing by the rendering backend,
    // it will be handled using the appropriate quality hints. For best results
    // use integer scaling factors.
    //
    // Each render target has its own scale. This function sets the scale for
    // the current render target.
    //
    // Should only be called on the main thread.
    void scale( float _scaleX, float _scaleY ) {
        const bool l_result = SDL_SetRenderScale( _data, _scaleX, _scaleY );

        assert( l_result );
    }

    // Get the drawing scale for the current target.
    //
    // Each render target has its own scale. This function gets the scale for
    // the current render target.
    //
    // Should only be called on the main thread.
    [[nodiscard]] auto scale() const -> std::pair< float, float > {
        float l_scaleX = NAN;
        float l_scaleY = NAN;

        const bool l_result = SDL_GetRenderScale( _data, &l_scaleX, &l_scaleY );

        assert( l_result );

        return { l_scaleX, l_scaleY };
    }

    // Set the color used for drawing operations.
    //
    // Set the color for drawing or filling rectangles, lines, and points, and
    // for SDL_RenderClear().
    //
    // Use SDL_SetRenderDrawBlendMode to specify how the alpha channel is used.
    //
    // Should only be called on the main thread.
    void color( color_t _color ) {
        const bool l_result = SDL_SetRenderDrawColor(
            _data, _color.red, _color.green, _color.blue, _color.alpha );

        assert( l_result );
    }

#if 0
    // Set the color used for drawing operations (Rect, Line and Clear).
    //
    // Set the color for drawing or filling rectangles, lines, and points, and
    // for SDL_RenderClear().
    //
    // Use SDL_SetRenderDrawBlendMode to specify how the alpha channel is used.
    //
    // Should only be called on the main thread.
    void color( color_t< float > _color ) {
        const bool l_result = SDL_SetRenderDrawColorFloat(
            _data, _color.red, _color.green, _color.blue, _color.alpha );

        assert( l_result );
    }
#endif

    // Get the color used for drawing operations (Rect, Line and Clear).
    //
    // Should only be called on the main thread.
    [[nodiscard]] auto color() const -> color_t {
        color_t l_color;

        const bool l_result =
            SDL_GetRenderDrawColor( _data, &l_color.red, &l_color.green,
                                    &l_color.blue, &l_color.alpha );

        assert( l_result );

        return ( l_color );
    }

#if 0
    // Get the color used for drawing operations (Rect, Line and Clear).
    //
    // Should only be called on the main thread.
    [[nodiscard]] auto color() const -> color_t< float > {
        color_t< float > l_color;

        const bool l_result =
            SDL_GetRenderDrawColorFloat( _data, &l_color.red, &l_color.green,
                                         &l_color.blue, &l_color.alpha );

        assert( l_result );

        return ( l_color );
    }
#endif

    // Set the color scale used for render operations.
    //
    // The color scale is an additional scale multiplied into the pixel color
    // value while rendering. This can be used to adjust the brightness of
    // colors during HDR rendering, or changing HDR video brightness when
    // playing on an SDR display.
    //
    // The color scale does not affect the alpha channel, only the color
    // brightness.
    //
    // \param scale the color scale value.
    //
    // Should only be called on the main thread.
    void colorScale( float _scale ) {
        const bool l_result = SDL_SetRenderColorScale( _data, _scale );

        assert( l_result );
    }

    // Get the color scale used for render operations.
    //
    // \param scale a pointer filled in with the current color scale value.
    //
    // Should only be called on the main thread.
    [[nodiscard]] auto colorScale() const -> float {
        float l_scale = NAN;

        const bool l_result = SDL_GetRenderColorScale( _data, &l_scale );

        assert( l_result );

        return ( l_scale );
    }

    // Set the blend mode used for drawing operations (Fill and Line).
    //
    // If the blend mode is not supported, the closest supported mode is chosen.
    //
    // Should only be called on the main thread.
    void drawBlendMode( blend_t _blend ) {
        const bool l_result =
            SDL_SetRenderDrawBlendMode( _data, toLegacy( _blend ) );

        assert( l_result );
    }

    // Get the blend mode used for drawing operations.
    //
    // Should only be called on the main thread.
    [[nodiscard]] auto drawBlendMode() const -> blend_t {
        blend_t l_blend = blend_t::none;

        const bool l_result = SDL_GetRenderDrawBlendMode(
            _data, std::bit_cast< SDL_BlendMode* >( &l_blend ) );

        assert( l_result );

        return ( l_blend );
    }

    // Clear the current rendering target with the drawing color.
    //
    // This function clears the entire rendering target, ignoring the viewport
    // and the clip rectangle. Note, that clearing will also set/fill all pixels
    // of the rendering target to current renderer draw color, so make sure to
    // invoke SDL_SetRenderDrawColor() when needed.
    //
    // Should only be called on the main thread.
    void clear() {
        const bool l_result = SDL_RenderClear( _data );

        assert( l_result );
    }

    // Draw a point on the current rendering target at subpixel precision.
    //
    // Should only be called on the main thread.
    void point( point_t< float > _coordinates ) {
        const bool l_result =
            SDL_RenderPoint( _data, _coordinates.x, _coordinates.y );

        assert( l_result );
    }

    // Draw multiple points on the current rendering target at subpixel
    // precision.
    //
    // Should only be called on the main thread.
    void points( std::span< const point_t< float > > _points ) {
        const auto l_points =
            stdfunc::spanToVector< point_t< float >, SDL_FPoint >( _points );

        const bool l_result =
            SDL_RenderPoints( _data, l_points.data(), _points.size() );

        assert( l_result );
    }

    // Draw a line on the current rendering target at subpixel precision.
    //
    // Should only be called on the main thread.
    void line( const line_t< float >& _line ) {
        const bool l_result = SDL_RenderLine(
            _data, _line.start.x, _line.start.y, _line.end.x, _line.end.y );

        assert( l_result );
    }

    // Draw a series of connected lines on the current rendering target at
    // subpixel precision.
    //
    // TODO: What is this?
    // \param count the number of points, drawing count-1 lines.
    //
    // Should only be called on the main thread.
    void lines( std::span< const line_t< float > > _lines ) {
        const auto l_points =
            stdfunc::spanToVector< point_t< float >, SDL_FPoint >(
                toPoints( _lines ) );

        const bool l_result =
            SDL_RenderLines( _data, l_points.data(), l_points.size() );

        assert( l_result );
    }

    // Draw a rectangle on the current rendering target at subpixel precision.
    //
    // A destination box, or NULL to outline the entire rendering target.
    //
    // Should only be called on the main thread.
    void box( const std::optional< box_t< float > >& _box = std::nullopt,
              bool _needFill = false ) {
        bool l_result = false;

        auto l_renderBox = [ & ]( auto _renderFunction,
                                  auto _argument ) -> bool {
            return ( _renderFunction( _data, _argument ) );
        };

        const auto& l_renderFunction =
            ( ( _needFill ) ? ( SDL_RenderFillRect ) : ( SDL_RenderRect ) );

        if ( _box ) {
            const SDL_FRect l_box = _box.value();

            l_result = l_renderBox( l_renderFunction, &l_box );

        } else {
            l_result = l_renderBox( l_renderFunction, nullptr );
        }

        assert( l_result );
    }

    // Draw some number of rectangles on the current rendering target at
    // subpixel precision.
    //
    // Should only be called on the main thread.
    void boxes( std::span< const box_t< float > > _boxes,
                bool _needFill = false ) {
        const auto l_boxes =
            stdfunc::spanToVector< box_t< float >, SDL_FRect >( _boxes );

        auto l_renderBoxes =
            [ & ]( auto _renderFunction,
                   std::span< const SDL_FRect > _argument ) -> bool {
            return (
                _renderFunction( _data, _argument.data(), _argument.size() ) );
        };

        const auto& l_renderFunction =
            ( ( _needFill ) ? ( SDL_RenderFillRects ) : ( SDL_RenderRects ) );

        const bool l_result = l_renderBoxes( l_renderFunction, l_boxes );

        assert( l_result );
    }

    // Variables
private:
    gsl::not_null< SDL_Renderer* > _data;
#if 0
    GPUDevice_t _GPUDevice;
#endif
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

        assert( l_properties );

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

        assert( l_result );

        l_result = SDL_SetTextureAlphaMod( _data, _color.alpha );

        assert( l_result );
    }

#if 0
    void colorModulate(color_t<float> _color);
#endif

    // Should only be called on the main thread.
    [[nodiscard]] auto modulator() const -> color_t {
        color_t l_color;

        bool l_result = SDL_GetTextureColorMod( _data, &l_color.red,
                                                &l_color.green, &l_color.blue );

        assert( l_result );

        l_result = SDL_GetTextureAlphaMod( _data, &l_color.alpha );

        assert( l_result );

        return ( l_color );
    }

#if 0
    [[nodiscard]] auto colorModulator() const -> color_t<float> {}
#endif

    // If the blend mode is not supported, the closest supported mode is chosen
    // and this function asserts.
    //
    // Should only be called on the main thread.
    void blend( blend_t _blend ) {
        const bool l_result =
            SDL_SetTextureBlendMode( _data, toLegacy( _blend ) );

        assert( l_result );
    }

    // Should only be called on the main thread.
    [[nodiscard]] auto blend() const -> blend_t {
        blend_t l_blend = blend_t::none;

        const bool l_result = SDL_GetTextureBlendMode(
            _data, std::bit_cast< SDL_BlendMode* >( &l_blend ) );

        assert( l_result );

        return ( l_blend );
    }

    // If the scale mode is not supported, the closest supported mode is chosen.
    //
    // Should only be called on the main thread.
    void scale( scale_t _scale ) {
        const bool l_result =
            SDL_SetTextureScaleMode( _data, toLegacy( _scale ) );

        assert( l_result );
    }

    // Should only be called on the main thread.
    [[nodiscard]] auto scale() const -> scale_t {
        scale_t l_scale = scale_t::linear;

        const bool l_result = SDL_GetTextureScaleMode(
            _data, std::bit_cast< SDL_ScaleMode* >( &l_scale ) );

        assert( l_result );

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
        const std::optional< const box_t< int > >& _box = std::nullopt );

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
        const std::optional< const box_t< int > >& _box = std::nullopt );

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
        const std::optional< const box_t< int > >& _box = std::nullopt );

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
        -> std::pair< std::span< const std::byte >, size_t >;

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
        bool l_result = false;

        SDL_Surface* l_surface = _surface;

        if ( _box ) {
            const SDL_Rect l_box = _box.value();

            l_result = SDL_LockTextureToSurface( _data, &l_box, &l_surface );

        } else {
            l_result = SDL_LockTextureToSurface( _data, nullptr, &l_surface );
        }

        assert( l_result );

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

    // Copy a portion of the texture to the current rendering target at subpixel
    // precision.
    //
    // Source box, or NULL for the entire texture.
    // Destination box, or NULL for the entire rendering target.
    //
    // Should only be called on the main thread.
    // TODO: Implement
    void render(
        renderer_t& _renderer,
        const std::optional< box_t< float > >& _source = std::nullopt,
        const std::optional< box_t< float > >& _destination = std::nullopt ) {
        const SDL_FRect l_source = _source.value();
        const SDL_FRect l_destination = _destination.value();

        const bool l_result =
            SDL_RenderTexture( _renderer, _data, &l_source, &l_destination );

        assert( l_result );
    }

    // Copy a portion of the source texture to the current rendering target,
    // with rotation and flipping, at subpixel precision.
    //
    // Source box, or NULL for the entire texture.
    // Destination box, or NULL for the entire rendering target.
    //
    // Rotation will be applied to dstrect, rotating it in a clockwise
    // direction.
    //
    // If center is NULL, rotation will be done around 50% / 50%
    //
    // Should only be called on the main thread.
    // TODO: Implement
    void renderRotated(
        renderer_t& _renderer,
        double _angle,
        const std::optional< box_t< float > >& _source = std::nullopt,
        const std::optional< box_t< float > >& _destination = std::nullopt,
        const std::optional< point_t< float > >& _center = std::nullopt,
        flip_t _flip = flip_t::none ) {
        const SDL_FRect l_source = _source.value();
        const SDL_FRect l_destination = _destination.value();
        const SDL_FPoint l_center = _center.value();

        const bool l_result = SDL_RenderTextureRotated(
            _renderer, _data, &l_source, &l_destination, _angle, &l_center,
            toLegacy( _flip ) );

        assert( l_result );
    }

    // Copy a portion of the source texture to the current rendering target,
    // with affine transform, at subpixel precision.
    //
    // \param renderer the renderer which should copy parts of a texture.
    // \param texture the source texture.
    // \param srcrect a pointer to the source rectangle, or NULL for the entire
    // texture.
    // \param origin a pointer to a point indicating where the top-left corner
    // of srcrect should be mapped to, or NULL for the rendering target's
    // origin.
    // \param right a pointer to a point indicating where the top-right corner
    // of srcrect should be mapped to, or NULL for the rendering target's
    // top-right corner.
    // \param down a pointer to a point indicating where the bottom-left corner
    // of srcrect should be mapped to, or NULL for the rendering target's
    // bottom-left corner.
    //
    // You may only call this function from the main thread.
    // TODO: Implement
    void renderAffine(
        renderer_t& _renderer,
        const std::optional< box_t< float > >& _source = std::nullopt,
        const std::optional< point_t< float > >& _origin = std::nullopt,
        const std::optional< point_t< float > >& _right = std::nullopt,
        const std::optional< point_t< float > >& _down = std::nullopt ) {
        const SDL_FRect l_source = _source.value();
        const SDL_FPoint l_origin = _origin.value();
        const SDL_FPoint l_right = _right.value();
        const SDL_FPoint l_down = _down.value();

        const bool l_result = SDL_RenderTextureAffine(
            _renderer, _data, &l_source, &l_origin, &l_right, &l_down );

        assert( l_result );
    }

    // Variables
private:
    gsl::not_null< SDL_Texture* > _data;
};

// Create a window and default renderer.
//
// Should only be called on the main thread
[[nodiscard]] inline auto windowAndRenderer( std::string_view _title,
                                             volume_t _volume,
                                             SDL_WindowFlags _windowFlags )
    -> std::pair< window_t, renderer_t > {
    SDL_Window* l_window = nullptr;
    SDL_Renderer* l_renderer = nullptr;

    const bool l_result = SDL_CreateWindowAndRenderer(
        std::string( _title ).c_str(), _volume.width, _volume.height,
        _windowFlags, &l_window, &l_renderer );

    assert( l_result );

    return { l_window, l_renderer };
}

[[nodiscard]] inline auto renderer( window_t _window ) -> renderer_t {
    return ( SDL_GetRenderer( _window ) );
}

// TODO: Move to renderer_t
// Set a texture as the current rendering target.
//
// The default render target is the window for which the renderer was
// created. To stop rendering to a texture and render to the window again,
// call this function with a NULL `texture`.
//
// Viewport, cliprect, scale, and logical presentation are unique to each
// render target. Get and set functions for these states apply to the
// current render target set by this function, and those states persist on
// each target when the current render target changes.
//
// The targeted texture must be created with the `SDL_TEXTUREACCESS_TARGET`
// flag, or NULL to render to the window instead of a texture.
//
// Should only be called on the main thread.
inline void renderTarget(
    const renderer_t& _renderer,
    const std::optional< texture_t >& _texture = std::nullopt ) {
    bool l_result = false;

    if ( _texture ) {
        l_result = SDL_SetRenderTarget( _renderer, _texture.value() );

    } else {
        l_result = SDL_SetRenderTarget( _renderer, nullptr );
    }

    assert( l_result );
}

// Get the current render target.
//
// The default render target is the window for which the renderer was
// created, and is reported a NULL here.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto renderTarget( const renderer_t& _renderer )
    -> texture_t {
    return ( SDL_GetRenderTarget( _renderer ) );
}

} // namespace slickdl
