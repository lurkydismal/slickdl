#include "slickdl/gamepad.hpp"

#include <ranges>

namespace slickdl::gamepad {

[[nodiscard]] auto mappings() -> std::vector< std::string > {
    size_t l_amount = 0;

    std::unique_ptr< char*, void ( * )( void* ) > l_result(
        SDL_GetGamepadMappings( std::bit_cast< int* >( &l_amount ) ),
        SDL_free );

    assert( l_result.get() );

    return ( std::span( l_result.get(), l_amount ) |
             std::views::transform( []( char* _mapping ) -> std::string {
                 return { _mapping };
             } ) |
             std::ranges::to< std::vector >() );
}

[[nodiscard]] auto mapping( GUID_t _GUID ) -> std::string {
    std::unique_ptr< char, void ( * )( void* ) > l_result(
        SDL_GetGamepadMappingForGUID( _GUID ), SDL_free );

    assert( l_result.get() );

    return { l_result.get() };
}

[[nodiscard]] auto gamepad_t::mapping() -> std::string {
    std::unique_ptr< char, void ( * )( void* ) > l_result(
        SDL_GetGamepadMapping( _data ), SDL_free );

    assert( l_result.get() );

    return { l_result.get() };
}

[[nodiscard]] auto all() -> std::vector< joystick::id_t > {
    size_t l_amount = 0;

    std::unique_ptr< joystick::id_t::native_t, void ( * )( void* ) > l_result(
        SDL_GetGamepads( std::bit_cast< int* >( &l_amount ) ), SDL_free );

    assert( l_result.get() );

    return ( std::span( l_result.get(), l_amount ) |
             std::views::transform(
                 []( joystick::id_t::native_t _id ) -> joystick::id_t {
                     return { _id };
                 } ) |
             std::ranges::to< std::vector >() );
}

[[nodiscard]] auto mapping( joystick::id_t _id ) -> std::string {
    std::unique_ptr< char, void ( * )( void* ) > l_result(
        SDL_GetGamepadMappingForID( _id ), SDL_free );

    assert( l_result.get() );

    return { l_result.get() };
}

[[nodiscard]] auto gamepad_t::bindings() -> std::vector< binding_t > {
    size_t l_amount = 0;

    std::unique_ptr< SDL_GamepadBinding*, void ( * )( void* ) > l_result(
        SDL_GetGamepadBindings( _data, std::bit_cast< int* >( &l_amount ) ),
        SDL_free );

    assert( l_result.get() );

    return (
        std::span( l_result.get(), l_amount ) |
        std::views::transform( []( SDL_GamepadBinding* _binding ) -> binding_t {
            return { _binding };
        } ) |
        std::ranges::to< std::vector >() );
}

} // namespace slickdl::gamepad
