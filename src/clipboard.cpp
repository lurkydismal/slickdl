#include "slickdl/clipboard.hpp"

#include <ranges>

namespace slickdl::clipboard {

[[nodiscard]] auto text() -> std::string {
    std::unique_ptr< char, void ( * )( void* ) > l_result(
        SDL_GetClipboardText(), SDL_free );

    assert( l_result.get() );

    return { l_result.get() };
}

namespace primary_selection {

[[nodiscard]] auto text() -> std::string {
    std::unique_ptr< char, void ( * )( void* ) > l_result(
        SDL_GetPrimarySelectionText(), SDL_free );

    assert( l_result.get() );

    return { l_result.get() };
}

} // namespace primary_selection

[[nodiscard]] auto mimeTypes() -> std::vector< std::string > {
    size_t l_amount = 0;

    SDL_GetClipboardMimeTypes( &l_amount );

    std::unique_ptr< char*, void ( * )( void* ) > l_result(
        SDL_GetClipboardMimeTypes( &l_amount ), SDL_free );

    assert( l_result.get() );

    return ( std::span( l_result.get(), l_amount ) |
             std::views::transform( []( char* _mimeType ) -> std::string {
                 return { _mimeType };
             } ) |
             std::ranges::to< std::vector >() );
}

} // namespace slickdl::clipboard
