#include "slickdl/video.hpp"

#include <ranges>

namespace slickdl::video {

namespace display {

[[nodiscard]] auto all() -> std::vector< id_t > {
    size_t l_amount = 0;

    std::unique_ptr< id_t, void ( * )( void* ) > l_result(
        SDL_GetDisplays( std::bit_cast< int* >( &l_amount ) ), SDL_free );

    assert( l_result.get() );

    return ( std::span( l_result.get(), l_amount ) |
             std::ranges::to< std::vector >() );
}

[[nodiscard]] auto fullscreenModes( id_t _id )
    -> std::vector< gsl::not_null< mode_t* > > {
    size_t l_amount = 0;

    std::unique_ptr< mode_t*, void ( * )( void* ) > l_result(
        SDL_GetFullscreenDisplayModes( _id,
                                       std::bit_cast< int* >( &l_amount ) ),
        SDL_free );

    assert( l_result.get() );

    return (
        std::span( l_result.get(), l_amount ) |
        std::views::transform( []( mode_t* _mode ) -> gsl::not_null< mode_t* > {
            return { _mode };
        } ) |
        std::ranges::to< std::vector >() );
}

} // namespace display

} // namespace slickdl::video
