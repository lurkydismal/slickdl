#pragma once

#if __has_include( "SDL3_ttf/SDL_ttf.h" )

#include <SDL3_ttf/SDL_ttf.h>

#include <gsl/pointers>
#include <span>

#include "slickdl.hpp"
#include "slickdl/color.hpp"
#include "slickdl/surface.hpp"
#include "stddebug.hpp"

namespace slickdl::text {

void init() {
    const bool l_result = TTF_Init();

    stdfunc::assert( l_result, "Initializing: {}", SDL_GetError() );
}

void quit() {
    TTF_Quit();
}

using font_t = struct font {
    font() = delete;
    font( const font& ) = default;
    font( font&& ) = default;

    font( std::span< const std::byte > _data, float _size = 18 )
        : _atlas(
              TTF_OpenFontIO( SDL_IOFromConstMem( _data.data(), _data.size() ),
                              true,
                              _size ) ) {
        stdfunc::assert( !_data.empty() );
    }

    ~font() { TTF_CloseFont( _atlas ); }

    auto operator=( const font& ) -> font& = default;
    auto operator=( font&& ) -> font& = default;

    [[nodiscard]] auto render( slickdl::renderer_t _renderer,
                               std::string_view _text,
                               color_t _color = {} ) const
        -> slickdl::texture_t {
        stdfunc::assert( !_text.empty() );

        slickdl::surface_t l_text = TTF_RenderText_Blended(
            _atlas, _text.data(), _text.length(), _color );

        return ( SDL_CreateTextureFromSurface( _renderer, l_text ) );
    }

    // Variables
private:
    using TTF_t = gsl::not_null< TTF_Font* >;

    TTF_t _atlas;
};

} // namespace slickdl::text

#endif
