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

namespace window {

[[nodiscard]] auto ICCProfile( window_t _window ) -> std::vector< std::byte > {
    size_t l_size = 0;

    std::unique_ptr< void, void ( * )( void* ) > l_result(
        SDL_GetWindowICCProfile( _window, &l_size ), SDL_free );

    assert( l_result.get() );

    std::vector< std::byte > l_returnValue( l_size );

    // TODO: Improve
    __builtin_memmove( l_returnValue.data(), l_result.get(), l_size );

    return { l_returnValue };
}

[[nodiscard]] auto all() -> std::vector< window_t > {
    size_t l_amount = 0;

    std::unique_ptr< SDL_Window*, void ( * )( void* ) > l_result(
        SDL_GetWindows( std::bit_cast< int* >( &l_amount ) ), SDL_free );

    assert( l_result.get() );

    return ( std::span( l_result.get(), l_amount ) |
             std::views::transform( []( SDL_Window* _window ) -> window_t {
                 return { _window };
             } ) |
             std::ranges::to< std::vector >() );
}

void updateSurface( window_t _window, std::span< const box_t< int > > _boxes ) {
    const auto l_boxes =
        stdfunc::spanToVector< box_t< int >, SDL_Rect >( _boxes );

    const bool l_result =
        SDL_UpdateWindowSurfaceRects( _window, l_boxes.data(), l_boxes.size() );

    assert( l_result );
}

} // namespace window

} // namespace slickdl::video
