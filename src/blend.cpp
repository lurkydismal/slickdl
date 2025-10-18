#include "slickdl/blend.hpp"

namespace slickdl {

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
