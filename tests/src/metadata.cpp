#include "slickdl/metadata.hpp"

#include <SDL3/SDL.h>

#include "test.hpp"

using namespace slickdl;
using namespace slickdl::metadata;

struct metadataTest : testing::Test {
    void SetUp() override { ASSERT_TRUE( SDL_Init( SDL_INIT_VIDEO ) ); }

    void TearDown() override { SDL_Quit(); }
};

TEST_F( metadataTest, PropertySetGetRoundtrip ) {
    // Set a property and read it back.
    property::set( property::g_name, std::string_view( "My Test App" ) );
    const auto l_val = property::get( property::g_name );
    EXPECT_EQ( l_val, std::string_view( "My Test App" ) );

    // Remove it. Name has a default, so get should still succeed but not equal
    // our previously set value.
    property::set( property::g_name, std::nullopt );
    const auto l_val2 = property::get( property::g_name );
    EXPECT_FALSE( l_val2 == std::string_view( "My Test App" ) );
    EXPECT_FALSE( l_val2.empty() );
}

TEST_F( metadataTest, ShortcutSetFunction ) {
    // Use the simplified setter to assign name, version and identifier.
    metadata::set( std::string_view( "AppX" ), std::string_view( "v2" ),
                   std::string_view( "com.example.appx" ) );

    EXPECT_EQ( property::get( property::g_name ), std::string_view( "AppX" ) );
    EXPECT_EQ( property::get( property::g_version ), std::string_view( "v2" ) );
    EXPECT_EQ( property::get( property::g_identifier ),
               std::string_view( "com.example.appx" ) );

    // Clear them using the simplified setter.
    metadata::set( std::nullopt, std::nullopt, std::nullopt );

    // Name has a default so get should still succeed and not equal "AppX".
    const auto l_nameAfterClear = property::get( property::g_name );
    EXPECT_FALSE( l_nameAfterClear == std::string_view( "AppX" ) );
    EXPECT_FALSE( l_nameAfterClear.empty() );
}
