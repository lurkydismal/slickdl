#pragma once

#include <SDL3/SDL_blendmode.h>

namespace slickdl {

// These predefined blend modes are supported everywhere.
//
// Additional values may be obtained from composeCustomBlendMode.
using blend_t = enum class blend : uint32_t {
    none = 0,     /**< no blending: destinationRGBA = sourceRGBA */
    blend = 0x1U, /**< alpha blending: destinationRGB = (sourceRGB * sourceA) +
                   * (destinationRGB (1-sourceA)), destinationA = sourceA +
                   * (destinationA * (1-sourceA)) */
    blendPremultiplied =
        0x10U,  /**< pre-multiplied alpha blending: destinationRGBA =
                   sourceRGBA + (destinationRGBA * (1-sourceA)) */
    add = 0x2U, /**< additive blending: destinationRGB = (sourceRGB * sourceA) +
                          destinationRGB, destinationA = destinationA */
    addPremultiplied = 0x20U, /**< pre-multiplied additive blending:
                                       destinationRGB = sourceRGB +
                                 destinationRGB, destinationA = destinationA */
    modulate = 0x4U,          /**< color modulate: destinationRGB = sourceRGB *
                            destinationRGB,               destinationA = destinationA */
    multiply =
        0x8U, /**< color multiply: destinationRGB = (sourceRGB * destinationRGB)
               * + (destinationRGB (1-sourceA)), destinationA = destinationA */
    invalid = 0x7FFFFFFFU,
};

// The blend operation used when combining source and destination pixel
// components.
using blendOperation_t = enum class blendOperation : uint8_t {
    add = 0x1,         /**< destination + source: supported by all renderers */
    subtract = 0x2,    /**< source - destination : supported by D3D, OpenGL,
                          OpenGLES,    and Vulkan */
    revSubtract = 0x3, /**< destination - source : supported by D3D, OpenGL,
                          OpenGLES, and Vulkan */
    minimum = 0x4,     /**< min(destination, source) : supported by D3D, OpenGL,
                          OpenGLES, and     Vulkan */
    maximum = 0x5,     /**< max(destination, source) : supported by D3D, OpenGL,
                         OpenGLES, and     Vulkan */
};

// The blend factors are multiplied with the pixels from a drawing operation
// (source) and the pixels from the render target (destination) before the blend
// operation. The comma-separated factors listed above are always applied in
// the component order red, green, blue, and alpha.
using blendFactor_t = enum class blendFactor : uint8_t {
    zero = 0x1,        /**< 0, 0, 0, 0 */
    one = 0x2,         /**< 1, 1, 1, 1 */
    sourceColor = 0x3, /**< sourceR, sourceG, sourceB, sourceA */
    oneMinusSourceColor =
        0x4,           /**< 1-sourceR, 1-sourceG, 1-sourceB, 1-sourceA */
    sourceAlpha = 0x5, /**< sourceA, sourceA, sourceA, sourceA */
    oneMinusSourceAlpha =
        0x6, /**< 1-sourceA, 1-sourceA, 1-sourceA, 1-sourceA */
    destinationColor =
        0x7, /**< destinationR, destinationG, destinationB, destinationA */
    oneMinusDestinationColor = 0x8, /**< 1-destinationR, 1-destinationG,
                                       1-destinationB, 1-destinationA */
    destinationAlpha =
        0x9, /**< destinationA, destinationA, destinationA, destinationA */
    oneMinusDestinationAlpha = 0xA, /**< 1-destinationA, 1-destinationA,
                                       1-destinationA, 1-destinationA */
};

// Legacy
[[nodiscard]] constexpr auto toLegacy( blend_t _blend ) -> SDL_BlendMode {
    return ( static_cast< SDL_BlendMode >( _blend ) );
}

[[nodiscard]] constexpr auto toLegacy( blendOperation_t _blendOperation )
    -> SDL_BlendOperation {
    return ( static_cast< SDL_BlendOperation >( _blendOperation ) );
}

[[nodiscard]] constexpr auto toLegacy( blendFactor_t _blendFactor )
    -> SDL_BlendFactor {
    return ( static_cast< SDL_BlendFactor >( _blendFactor ) );
}

// The functions SDL_SetRenderDrawBlendMode and SDL_SetTextureBlendMode accept
// the blend_t:: returned by this function if the renderer supports it.
//
// A blend mode controls how the pixels from a drawing operation (source) get
// combined with the pixels from the render target (destination). First, the
// components of the source and destination pixels get multiplied with their
// blend factors. Then, the blend operation takes the two products and
// calculates the result that will get stored in the render target.
//
// Expressed in pseudocode, it would look like this:
//
// ```c
// destinationRGB = colorOperation(sourceRGB * sourceColorFactor, destinationRGB
// * destinationColorFactor); destinationA = alphaOperation(sourceA *
// sourceAlphaFactor, destinationA * destinationAlphaFactor);
// ```
//
// Where the functions `colorOperation(source, destination)` and
// `alphaOperation(source, destination)` can return one of the following:
//
// - `source + destination`
// - `source - destination`
// - `destination - source`
// - `min(source, destination)`
// - `max(source, destination)`
//
// The red, green, and blue components are always multiplied with the first,
// second, and third components of the SDL_BlendFactor, respectively. The
// fourth component is not used.
//
// The alpha component is always multiplied with the fourth component of the
// SDL_BlendFactor. The other components are not used in the alpha
// calculation.
//
// Support for these blend modes varies for each renderer. To check if a
// specific blend_t:: is supported, create a renderer and pass it to
// either SDL_SetRenderDrawBlendMode or SDL_SetTextureBlendMode. They will
// return with an error if the blend mode is not supported.
//
// This list describes the support of custom blend modes for each renderer.
// All renderers support the four blend modes listed in the blend_t::
// enumeration.
//
// - **direct3d**: Supports all operations with all factors. However, some
//   factors produce unexpected results with `blendOperation_t::minimum` and
//   `blendOperation_t::maximum`.
// - **direct3d11**: Same as Direct3D 9.
// - **opengl**: Supports the `blendOperation_t::add` operation with all
//   factors. OpenGL versions 1.1, 1.2, and 1.3 do not work correctly here.
// - **opengles2**: Supports the `blendOperation_t::add`,
//   `blendOperation_t::SUBTRACT`, `blendOperation_t::rev_subtract`
//   operations with all factors.
// - **psp**: No custom blend mode support.
// - **software**: No custom blend mode support.
//
// Some renderers do not provide an alpha component for the default render
// target. The `blendFactor_t::destinationAlpha` and
// `blendFactor_t::oneMinusDestinationAlpha` factors do not have an effect in
// this case.
[[nodiscard]] auto composeCustomBlendMode(
    blendFactor_t _sourceColorFactor,
    blendFactor_t _destinationColorFactor,
    blendOperation_t _colorOperation,
    blendFactor_t _sourceAlphaFactor,
    blendFactor_t _destinationAlphaFactor,
    blendOperation_t _alphaOperation ) -> blend_t {
    return ( static_cast< blend_t >( SDL_ComposeCustomBlendMode(
        toLegacy( _sourceColorFactor ), toLegacy( _destinationColorFactor ),
        toLegacy( _colorOperation ), toLegacy( _sourceAlphaFactor ),
        toLegacy( _destinationAlphaFactor ), toLegacy( _alphaOperation ) ) ) );
}

} // namespace slickdl
