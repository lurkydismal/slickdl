#include "slickdl/render_texture.hpp"

namespace slickdl {

void texture_t::update( std::span< const std::byte > _pixels,
                        const std::optional< const box_t< int > >& _box ) {
    const std::vector< uint8_t > l_pixels =
        stdfunc::spanToVector< std::byte, uint8_t >( _pixels );

    bool l_result = false;

    if ( _box ) {
        const SDL_Rect l_box = _box.value();

        l_result = SDL_UpdateTexture( _data, &l_box, l_pixels.data(),
                                      l_pixels.size() );

    } else {
        l_result = SDL_UpdateTexture( _data, nullptr, l_pixels.data(),
                                      l_pixels.size() );
    }

    assert( l_result );
}

void texture_t::update(
    std::pair< std::span< const std::byte >, size_t > _pixelsY,
    std::pair< std::span< const std::byte >, size_t > _pixelsU,
    std::pair< std::span< const std::byte >, size_t > _pixelsV,
    const std::optional< const box_t< int > >& _box ) {
    const std::vector< uint8_t > l_pixelsY =
        stdfunc::spanToVector< std::byte, uint8_t >( _pixelsY.first );
    const std::vector< uint8_t > l_pixelsU =
        stdfunc::spanToVector< std::byte, uint8_t >( _pixelsU.first );
    const std::vector< uint8_t > l_pixelsV =
        stdfunc::spanToVector< std::byte, uint8_t >( _pixelsV.first );

    bool l_result = false;

    if ( _box ) {
        const SDL_Rect l_box = _box.value();

        l_result = SDL_UpdateYUVTexture(
            _data, &l_box, l_pixelsY.data(), _pixelsY.second, l_pixelsU.data(),

            _pixelsU.second, l_pixelsV.data(), _pixelsV.second );
    } else {
        l_result = SDL_UpdateYUVTexture(
            _data, nullptr, l_pixelsY.data(), _pixelsY.second, l_pixelsU.data(),
            _pixelsU.second, l_pixelsV.data(), _pixelsV.second );
    }

    assert( l_result );
}

void texture_t::update(
    std::pair< std::span< const std::byte >, size_t > _pixelsY,
    std::pair< std::span< const std::byte >, size_t > _pixelsU,
    const std::optional< const box_t< int > >& _box ) {
    const std::vector< uint8_t > l_pixelsY =
        stdfunc::spanToVector< std::byte, uint8_t >( _pixelsY.first );
    const std::vector< uint8_t > l_pixelsU =
        stdfunc::spanToVector< std::byte, uint8_t >( _pixelsU.first );

    bool l_result = false;

    if ( _box ) {
        const SDL_Rect l_box = _box.value();

        l_result = SDL_UpdateNVTexture( _data, &l_box, l_pixelsY.data(),
                                        _pixelsY.second, l_pixelsU.data(),
                                        _pixelsU.second );

    } else {
        l_result = SDL_UpdateNVTexture( _data, nullptr, l_pixelsY.data(),
                                        _pixelsY.second, l_pixelsU.data(),
                                        _pixelsU.second );
    }

    assert( l_result );
}

[[nodiscard]] auto texture_t::lock(
    const std::optional< const box_t< int > >& _box )
    -> std::pair< std::span< const std::byte >, size_t > {
    std::pair< std::span< const std::byte >, size_t > l_pixels;

    // Fill pixels
    {
        std::byte* l_temp = nullptr;

        bool l_result = false;

        if ( _box ) {
            const SDL_Rect l_box = _box.value();

            l_result = SDL_LockTexture(
                _data, &l_box, std::bit_cast< void** >( &l_temp ),
                std::bit_cast< int* >( &l_pixels.second ) );

        } else {
            l_result = SDL_LockTexture(
                _data, nullptr, std::bit_cast< void** >( &l_temp ),
                std::bit_cast< int* >( &l_pixels.second ) );
        }

        assert( l_result );

        const volume_t l_volume = volume();

        l_pixels.first = { l_temp, static_cast< size_t >(
                                       ( l_volume.width * l_volume.height ) ) };
    }

    return ( l_pixels );
}

} // namespace slickdl
