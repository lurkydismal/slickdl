#pragma once

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_render.h>

#include <gsl/pointers>

#include "stddebug.hpp"

namespace slickdl {

template < typename T >
concept is_int_or_float =
    ( std::is_same_v< T, int > || std::is_same_v< T, float > );

using renderer_t = gsl::not_null< SDL_Renderer* >;
using texture_t = gsl::not_null< SDL_Texture* >;
using palette_t = gsl::not_null< SDL_Palette* >;

// Helpers
template < typename T >
    requires is_int_or_float< T >
[[nodiscard]] constexpr auto inRange1D( T _point, T _min, T _max ) -> bool {
    return ( ( _point >= _min ) && ( _point < _max ) );
}

constexpr void assert( bool _result ) {
    stdfunc::assert( _result, "{}", SDL_GetError() );
}

} // namespace slickdl
