#pragma once

#include <SDL3/SDL_guid.h>

#include <algorithm>
#include <array>
#include <ranges>
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

    static constexpr size_t g_dataSize = 16;

    GUID() = delete;

    GUID( const GUID& ) = default;
    GUID( GUID&& ) = default;

    constexpr GUID( native_t _GUID ) : _data( std::to_array( _GUID.data ) ) {}

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
        stdfunc::assert( _compiled.size() == ( g_dataSize * 2 ) );

        size_t l_byteIndex = 0;

        for ( const auto [ _highNibble, _lowNibble ] :
              std::views::iota( size_t{}, ( g_dataSize * 2 ) ) |
                  std::views::stride( 2 ) |
                  std::views::transform(
                      [ & ]( size_t _index ) -> std::pair< char, char > {
                          return {
                              _nibble( _compiled[ _index ] ),
                              _nibble( _compiled[ _index + 1 ] ),
                          };
                      } ) ) {
            _data.at( l_byteIndex ) = ( ( _highNibble << 4 ) | _lowNibble );

            l_byteIndex++;
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

    // Get an ASCII string representation for a given GUID.
    [[nodiscard]] auto toString() const -> std::string {
        static constexpr auto l_hexLUT = std::to_array( "0123456789abcdef" );

        std::array< char, ( g_dataSize * 2 ) > l_returnValue{};

        for ( size_t _index : std::views::iota( size_t{}, g_dataSize ) ) {
            uint8_t l_byte = _data.at( _index );

            l_returnValue.at( 2 * _index ) = l_hexLUT.at( l_byte >> 4 );
            l_returnValue.at( 2 * _index + 1 ) = l_hexLUT.at( l_byte & 0xF );
        }

        return { l_returnValue.data(), l_returnValue.size() };
    }

    [[nodiscard]] constexpr auto data() const -> const auto& {
        return ( _data );
    };

    // Helpers
private:
    // Returns the 4-bit nibble for a hex character
    [[nodiscard]] static constexpr auto _nibble( char _character ) -> char {
        const bool l_isDigit =
            ( ( _character >= '0' ) && ( _character <= '9' ) );
        const bool l_isUpper =
            ( ( _character >= 'A' ) && ( _character <= 'F' ) );
        const bool l_isLower =
            ( ( _character >= 'a' ) && ( _character <= 'f' ) );

        stdfunc::assert( l_isDigit || l_isUpper || l_isLower, "Character: '{}'",
                         _character );

        return ( static_cast< uint8_t >(
            ( l_isDigit ) ? ( _character - '0' )
                          : ( ( _character & ~0x20 ) - 'A' +
                              0xA ) // Fold lowercase to uppercase
            ) );
    }

    // Variables
private:
    std::array< uint8_t, g_dataSize > _data;
};

} // namespace slickdl

template <>
struct std::hash< slickdl::GUID_t > {
    constexpr auto operator()( const slickdl::GUID_t& _GUID ) const -> size_t {
        const auto& l_data = _GUID.data();

        return ( std::hash< std::string_view >{}( std::string_view{
            std::bit_cast< const char* >( l_data.data() ), l_data.size() } ) );
    }
};
