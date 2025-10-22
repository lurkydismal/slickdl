#include "slickdl/sensor.hpp"

#include <ranges>

namespace slickdl::sensors {

[[nodiscard]] auto all() -> std::vector< id_t > {
    size_t l_amount = 0;

    std::unique_ptr< id_t, void ( * )( void* ) > l_result(
        SDL_GetSensors( std::bit_cast< int* >( &l_amount ) ), SDL_free );

    assert( l_result.get() );

    return ( std::span( l_result.get(), l_amount ) |
             std::ranges::to< std::vector >() );
}

} // namespace slickdl::sensors
