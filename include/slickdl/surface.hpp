#pragma once

#include <SDL3/SDL_surface.h>

#include <gsl/pointers>
#include <type_traits>
#include <utility>

namespace slickdl {

using surface_t = struct surface {
    surface() = delete;
    surface( const surface& ) = default;
    surface( surface&& ) = default;

    template < typename OtherType >
        requires std::is_convertible_v< OtherType, SDL_Surface* >
    constexpr surface( OtherType&& _other )
        : _data( std::forward< OtherType >( _other ) ) {}

    constexpr ~surface() { SDL_DestroySurface( _data ); }

    auto operator=( const surface& ) -> surface& = default;
    auto operator=( surface&& ) -> surface& = default;

    constexpr operator SDL_Surface*() const { return ( _data ); }

    // Variables
private:
    gsl::not_null< SDL_Surface* > _data;
};

} // namespace slickdl
