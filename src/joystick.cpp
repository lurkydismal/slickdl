#include "slickdl/joystick.hpp"

#include <ranges>

namespace slickdl::joystick {

[[nodiscard]] auto id_t::all() -> std::vector< id > {
    size_t l_amount = 0;

    std::unique_ptr< id_t::native_t, void ( * )( void* ) > l_result(
        SDL_GetJoysticks( std::bit_cast< int* >( &l_amount ) ), SDL_free );

    assert( l_result.get() );

    return ( std::span( l_result.get(), l_amount ) |
             std::views::transform(
                 []( id_t::native_t _id ) -> id_t { return { _id }; } ) |
             std::ranges::to< std::vector >() );
}

} // namespace slickdl::joystick
