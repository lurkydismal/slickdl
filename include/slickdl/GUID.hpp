#pragma once

#include <SDL3/SDL_guid.h>

#include <array>
#include <string>
#include <string_view>

// A GUID is a 128-bit value that represents something that is uniquely
// identifiable by this value: "globally unique."
//
// SDL provides functions to convert a GUID to/from a string.
namespace slickdl::GUID {

// An SDL_GUID is a 128-bit identifier for an input device that identifies
// that device across runs of SDL programs on the same platform.
//
// If the device is detached and then re-attached to a different port, or if
// the base system is rebooted, the device should still report the same GUID.
//
// GUIDs are as precise as possible but are not guaranteed to distinguish
// physically distinct but equivalent devices. For example, two game
// controllers from the same vendor with the same product ID and revision may
// have the same GUID.
//
// GUIDs may be platform-dependent (i.e., the same device may report different
// GUIDs on different operating systems).
using GUID_t = struct GUID {
    using native_t = SDL_GUID;

    GUID() = default;
    GUID( const GUID& ) = default;
    GUID( GUID&& ) = default;

    constexpr GUID( native_t& _GUID ) : _data( std::to_array( _GUID.data ) ) {}

    // Convert a GUID string into a SDL_GUID structure.
    //
    // Performs no error checking. If this function is given a string containing
    // an invalid GUID, the function will silently succeed, but the GUID
    // generated will not be useful.
    //
    // An ASCII representation of a GUID.
    constexpr GUID( std::string_view _compiled ) {
        SDL_GUID l_guid;
        int l_maxOutputBytes = sizeof( l_guid );
        size_t l_len = _compiled.length();
        uint8_t* l_p = nullptr;
        size_t l_i = 0;

        // Make sure it's even
        l_len = ( l_len ) & ~0x1;

        __builtin_memset( &l_guid, 0, sizeof( l_guid ) );

        l_p = std::bit_cast< uint8_t* >( &l_guid );

        for ( l_i = 0; ( l_i < l_len ) &&
                       ( ( l_p - std::bit_cast< uint8_t* >( &l_guid ) ) <
                         l_maxOutputBytes );
              l_i += 2, std::advance( l_p, 1 ) ) {
            *l_p = ( _nibble( static_cast< char >( _compiled[ l_i ] ) ) << 4 ) |
                   _nibble( static_cast< char >( _compiled[ l_i + 1 ] ) );
        }

        *this = l_guid;
    }

    ~GUID() = default;
    auto operator=( const GUID& ) -> GUID& = default;
    auto operator=( GUID&& ) -> GUID& = default;

    [[nodiscard]] constexpr operator native_t() const {
        native_t l_GUID;

        std::ranges::copy_n( _data.begin(), _data.size(),
                             std::ranges::begin( l_GUID.data ) );

        return ( l_GUID );
    }

    [[nodiscard]] constexpr auto empty() const -> bool {
        return ( _data.empty() );
    }

    // Get an ASCII string representation for a given SDL_GUID.
    //
    // The size of pszGUID, should be at least 33 bytes.
    [[nodiscard]] constexpr auto toString() const -> std::string {
        std::array< char, 33 > l_string{};
        size_t l_cbGuid = 33;

        {
            constexpr auto l_rgchHexToASCII =
                std::to_array( "0123456789abcdef" );

            auto l_iterator = l_string.begin();
            size_t l_i = 0;

            for ( l_i = 0; l_i < _data.size() && l_i < ( l_cbGuid - 1 ) / 2;
                  l_i++ ) {
                // each input byte writes 2 ascii chars, and might write a null
                // byte. If we don't have room for next input byte, stop
                char l_c = _data.at( l_i );

                *l_iterator = l_rgchHexToASCII.at( l_c >> 4 );
                std::advance( l_iterator, 1 );

                *l_iterator = l_rgchHexToASCII.at( l_c & 0x0F );
                std::advance( l_iterator, 1 );
            }

            *l_iterator = '\0';
        }

        return { l_string.data() };
    }

    // Helpers
private:
    // Returns the 4-bit nibble for a hex character
    [[nodiscard]] constexpr auto _nibble( char _c ) const -> char {
        if ( ( _c >= '0' ) && ( _c <= '9' ) ) {
            return ( _c - '0' );

        } else if ( ( _c >= 'A' ) && ( _c <= 'F' ) ) {
            return ( _c - 'A' + 0xA );

        } else if ( ( _c >= 'a' ) && ( _c <= 'f' ) ) {
            return ( _c - 'a' + 0xA );

        } else {
            // FIX: Error
            return ( 0 );
        }
    }

    // Variables
private:
    std::array< uint8_t, 16 > _data{};
};

} // namespace slickdl::GUID
