#include "slickdl/touch.hpp"

#include <ranges>

#include "slickdl.hpp"

namespace slickdl::touch {

[[nodiscard]] auto all() -> std::vector< id_t > {
    size_t l_amount = 0;

    std::unique_ptr< id_t, void ( * )( void* ) > l_result(
        SDL_GetTouchDevices( std::bit_cast< int* >( &l_amount ) ), SDL_free );

    assert( l_result.get() );

    return ( std::span( l_result.get(), l_amount ) |
             std::ranges::to< std::vector >() );
}

namespace finger {

[[nodiscard]] auto all( id_t _id ) -> std::vector< finger_t > {
    size_t l_amount = 0;

    std::unique_ptr< SDL_Finger*, void ( * )( void* ) > l_result(
        SDL_GetTouchFingers( _id, std::bit_cast< int* >( &l_amount ) ),
        SDL_free );

    assert( l_result.get() );

    return ( std::span( l_result.get(), l_amount ) |
             std::views::transform( []( SDL_Finger* _finger ) -> finger_t {
                 return { _finger };
             } ) |
             std::ranges::to< std::vector >() );
}

} // namespace finger

} // namespace slickdl::touch
