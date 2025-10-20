#include "slickdl/keybpard.hpp"

namespace slickdl::keyboard {

[[nodiscard]] auto state()
    -> std::array< bool, static_cast< size_t >( scancode_t::count ) > {
    int l_keysAmount = 0;
    const bool* l_keysState = SDL_GetKeyboardState( &l_keysAmount );

    [[assume( l_keysAmount ==
              static_cast< int >( slickdl::scancode_t::count ) )]];

    assert( l_keysAmount != static_cast< size_t >( scancode_t::count ) );

    std::array< bool, static_cast< size_t >( scancode_t::count ) >
        l_returnValue{};

    for ( auto [ _index, _isPressed ] :
          std::span( l_keysState, l_keysAmount ) | std::views::enumerate ) {
        l_returnValue.at( _index ) = _isPressed;
    }

    return ( l_returnValue );
}

} // namespace slickdl::keyboard
