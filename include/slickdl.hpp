#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>

#include <gsl/pointers>

#include "slickdl/error.hpp"
#include "stddebug.hpp"

namespace slickdl {

// Concepts
template < typename T >
concept is_int_or_float =
    ( std::is_same_v< T, int > || std::is_same_v< T, float > );

template < typename T >
concept is_byte_or_float =
    ( std::is_same_v< T, uint8_t > || std::is_same_v< T, float > );

template < is_int_or_float T, typename U, typename W >
using isIntOrFloat_t = std::conditional_t< std::is_same_v< T, int >, U, W >;

// Types
using void_t = gsl::not_null< void* >;
using window_t = gsl::not_null< SDL_Window* >;
using event_t = SDL_Event;

template < is_int_or_float T >
struct volume {
    T width, height;
};

template < typename T >
using volume_t = volume< T >;

// Helpers
template < is_int_or_float T >
[[nodiscard]] constexpr auto inRange1D( T _point, T _min, T _max ) -> bool {
    return ( ( _point >= _min ) && ( _point < _max ) );
}

constexpr void assert( bool _result ) {
    const std::optional l_message = slickdl::error::get();

    stdfunc::assert( _result, "{}", l_message.value_or( "<empty>" ) );
}

} // namespace slickdl
