#include "slickdl/power.hpp"

#include "slickdl.hpp"

namespace slickdl::power {

[[nodiscard]] auto info()
    -> std::pair< state_t, std::optional< std::pair< size_t, size_t > > > {
    ssize_t l_seconds = 0;
    ssize_t l_percent = 0;

    const state_t l_result =
        fromLegacy( SDL_GetPowerInfo( std::bit_cast< int* >( &l_seconds ),
                                      std::bit_cast< int* >( &l_percent ) ) );

    assert( l_result != state_t::error );

    if ( l_result == state_t::noBattery ) {
        return {
            l_result,
            std::nullopt,
        };

    } else {
        return {
            l_result,
            std::pair{
                static_cast< size_t >( ( l_seconds ) ? ( l_seconds ) : ( 0 ) ),
                static_cast< size_t >( ( l_percent ) ? ( l_percent ) : ( 0 ) ),
            },
        };
    }
}

} // namespace slickdl::power
