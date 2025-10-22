#pragma once

#include <SDL3/SDL_guid.h>

#include <algorithm>
#include <array>
#include <string>
#include <string_view>

#include "stddebug.hpp"

// A GUID is a 128-bit value that represents something that is uniquely
// identifiable by this value: "globally unique."
//
// SDL provides functions to convert a GUID to/from a string.
namespace slickdl {

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

    GUID() = delete;

    GUID( const GUID& ) = default;
    GUID( GUID&& ) = default;

    constexpr GUID( native_t _GUID ) : _data( std::to_array( _GUID.data ) ) {}
    constexpr GUID( native_t& _GUID ) : _data( std::to_array( _GUID.data ) ) {}

    // Convert a GUID string into a SDL_GUID structure.
    //
    // Performs no error checking. If this function is given a string containing
    // an invalid GUID, the function will silently succeed, but the GUID
    // generated will not be useful.
    //
    // An ASCII representation of a GUID.
    constexpr GUID( std::string_view _compiled )
        : _data( decltype( _data ){} ) {
        // Require exactly 32 hex digits
        stdfunc::assert( _compiled.size() == ( 2UZ * 16 ) );

        size_t l_idx = 0;

        for ( size_t _index = 0; ( _index < _compiled.size() ); _index += 2 ) {
            char l_hi = _nibble( _compiled[ _index ] );
            char l_lo = _nibble( _compiled[ _index + 1 ] );

            _data.at( l_idx++ ) = ( ( l_hi << 4 ) | l_lo );
        }
    }

    ~GUID() = default;
    auto operator=( const GUID& ) -> GUID& = default;
    auto operator=( GUID&& ) -> GUID& = default;

    [[nodiscard]] constexpr operator native_t() const {
        native_t l_GUID;

        std::ranges::copy( _data, std::ranges::begin( l_GUID.data ) );

        return ( l_GUID );
    }

    [[nodiscard]] constexpr auto empty() const -> bool {
        return ( std::ranges::all_of(
            _data, []( uint8_t _byte ) -> bool { return ( _byte == 0 ); } ) );
    }

    // Get an ASCII string representation for a given SDL_GUID.
    //
    // The size of pszGUID, should be at least 33 bytes.
    [[nodiscard]] auto toString() const -> std::string {
        static constexpr auto l_hexLUT = std::to_array( "0123456789abcdef" );

        std::string l_returnValue;

        l_returnValue.resize( 16UZ * 2 );

        for ( size_t _index = 0; ( _index < 16 ); ++_index ) {
            uint8_t l_byte = _data.at( _index );

            l_returnValue[ 2 * _index ] = l_hexLUT.at( l_byte >> 4 );
            l_returnValue[ 2 * _index + 1 ] = l_hexLUT.at( l_byte & 0xF );
        }

        return ( l_returnValue );
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
            stdfunc::assert( false );

            return ( 0 );
        }
    }

    // Variables
private:
    // TODO: Make cosnt
    std::array< uint8_t, 16 > _data;
};

} // namespace slickdl
