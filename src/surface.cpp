#include "slickdl/surface.hpp"

namespace slickdl {

[[nodiscard]] auto surface_t::alternates() -> std::vector< surface > {
    size_t l_amount = 0;

    std::unique_ptr< SDL_Surface*, void ( * )( void* ) > l_result(
        SDL_GetSurfaceImages( _data, std::bit_cast< int* >( &l_amount ) ),
        SDL_free );

    assert( l_result.get() );

    return ( std::span( l_result.get(), l_amount ) |
             std::views::transform( []( SDL_Surface* _surface ) -> surface {
                 return ( _surface );
             } ) |
             std::ranges::to< std::vector >() );
}

} // namespace slickdl
