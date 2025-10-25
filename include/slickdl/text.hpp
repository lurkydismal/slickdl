#pragma once

#if __has_include( "SDL3_ttf/SDL_ttf.h" )

#include <SDL3_ttf/SDL_ttf.h>

#include <bit>
#include <gsl/pointers>
#include <span>

#include "slickdl.hpp"
#include "slickdl/color.hpp"
#include "slickdl/render_texture.hpp"
#include "slickdl/surface.hpp"

namespace slickdl::text {

void init() {
    const bool l_result = TTF_Init();

    assert( l_result );
}

void quit() {
    TTF_Quit();
}

using font_t = struct font {
    using native_t = TTF_Font*;

    font() = delete;

    font( const font& ) = delete;
    font( font&& ) = default;

    font( std::span< const std::byte > _data, float _size = 18 )
        : _data(
              TTF_OpenFontIO( SDL_IOFromConstMem( _data.data(), _data.size() ),
                              true,
                              _size ) ) {
        stdfunc::assert( !_data.empty() );
    }

    ~font() { TTF_CloseFont( _data ); }

    auto operator=( const font& ) -> font& = default;
    auto operator=( font&& ) -> font& = default;

    [[nodiscard]] constexpr operator native_t() const {
        static_assert( sizeof( decltype( _data ) ) == sizeof( native_t ) );

        return ( std::bit_cast< native_t >( _data ) );
    }

    [[nodiscard]] auto render( slickdl::renderer_t _renderer,
                               std::string_view _text,
                               color_t _color = {} ) const
        -> slickdl::texture_t {
        stdfunc::assert( !_text.empty() );

        slickdl::surface_t l_text = TTF_RenderText_Blended(
            _data, _text.data(), _text.length(), _color );

        return ( SDL_CreateTextureFromSurface( _renderer, l_text ) );
    }

    // Variables
private:
    gsl::not_null< native_t > _data;
};

} // namespace slickdl::text

#endif
