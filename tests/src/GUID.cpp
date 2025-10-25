#include "slickdl/GUID.hpp"

#include <SDL3/SDL_guid.h>

#include "test.hpp"

using namespace slickdl;

TEST( GUIDTest, ConstructFromNativeMatchesSDLToString ) {
    SDL_GUID l_sdlGuid{};

    for ( int l_i = 0; l_i < 16; ++l_i ) {
        l_sdlGuid.data[ l_i ] = static_cast< Uint8 >( l_i * 7 + 3 );
    }

    GUID_t l_guid( l_sdlGuid );

    // SDL’s own string conversion
    std::array< char, 33 > l_sdlString{};

    SDL_GUIDToString( l_sdlGuid, l_sdlString.data(), sizeof( l_sdlString ) );

    // Wrapper string
    auto l_str = l_guid.toString();

    EXPECT_EQ( l_str, std::string( l_sdlString.data() ) );
}

TEST( GUIDTest, ConstructFromStringMatchesSDL ) {
    // Known valid GUID string (32 hex chars)
    constexpr std::string_view l_kGuidString =
        "00112233445566778899aabbccddeeff";

    SDL_GUID l_sdlGuid =
        SDL_StringToGUID( std::string( l_kGuidString ).c_str() );
    GUID_t l_guid( l_kGuidString );

    // Back to native SDL_GUID
    SDL_GUID l_native = l_guid;

    EXPECT_EQ( memcmp( &l_sdlGuid, &l_native, sizeof( SDL_GUID ) ), 0 );
}

TEST( GUIDTest, EmptyDetection ) {
    SDL_GUID l_emptySdl{};
    GUID_t l_guid( l_emptySdl );
    EXPECT_TRUE( l_guid.empty() );

    SDL_GUID l_nonEmpty{};
    l_nonEmpty.data[ 5 ] = 1;
    GUID_t l_nonEmptyGuid( l_nonEmpty );
    EXPECT_FALSE( l_nonEmptyGuid.empty() );
}

TEST( GUIDTest, RoundTripToStringAndBack ) {
    SDL_GUID l_sdlGuid{};

    for ( int l_i = 0; l_i < 16; ++l_i ) {
        l_sdlGuid.data[ l_i ] = static_cast< Uint8 >( 255 - l_i );
    }

    GUID_t l_guid( l_sdlGuid );
    auto l_str = l_guid.toString();
    GUID_t l_parsed( l_str );
    SDL_GUID l_sdlParsed = l_parsed;

    EXPECT_EQ( memcmp( &l_sdlGuid, &l_sdlParsed, sizeof( SDL_GUID ) ), 0 );
}

TEST( GUIDTest, InvalidHexCharacterTriggersAssert ) {
    EXPECT_DEATH( { GUID_t l_bad( "00112233445566778899AABBCCDDEEFG" ); }, "" );
}
