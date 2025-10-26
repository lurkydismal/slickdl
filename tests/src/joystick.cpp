#include "slickdl/joystick.hpp"

#include <SDL3/SDL.h>

#include "test.hpp"

using namespace slickdl;
using namespace slickdl::joystick;

struct joystickTest : testing::Test {
    void SetUp() override { ASSERT_TRUE( SDL_Init( SDL_INIT_JOYSTICK ) ); }
    void TearDown() override { SDL_Quit(); }
};

TEST_F( joystickTest, EnumRoundtrip ) {
    // type_t roundtrip
    {
        auto l_legacy = toLegacy( type_t::gamepad );
        auto l_back = fromLegacy( l_legacy );
        EXPECT_EQ( l_back, type_t::gamepad );
    }

    // connectionState_t roundtrip
    {
        auto l_legacy = toLegacy( connectionState_t::wired );
        auto l_back = fromLegacy( l_legacy );
        EXPECT_EQ( l_back, connectionState_t::wired );
    }
}

TEST_F( joystickTest, Constants ) {
    EXPECT_EQ( g_axisMax, 32767u );
    EXPECT_EQ( g_axisMin, -32768 );
    // hat constants are simple bitmasks
    EXPECT_EQ( g_rightUp, ( SDL_HAT_RIGHT | SDL_HAT_UP ) );
}

TEST_F( joystickTest, LockUnlockDoesNotCrash ) {
    // Should be safe regardless of connected devices
    EXPECT_NO_FATAL_FAILURE( lock() );
    EXPECT_NO_FATAL_FAILURE( unlock() );
}

TEST_F( joystickTest, EventsToggleAndQuery ) {
    // Toggle on
    eventsToggle( true );
    EXPECT_TRUE( areEventsEnabled() );

    // Toggle off
    eventsToggle( false );
    EXPECT_FALSE( areEventsEnabled() );

    // Restore to enabled (tests should be non-destructive)
    eventsToggle( true );
}

TEST_F( joystickTest, UpdateAndHasAny ) {
    // Safe to call even with no devices
    EXPECT_NO_FATAL_FAILURE( update() );
    // hasAny should return a bool
    bool l_any = hasAny();
    ( void )l_any; // value may be true or false depending on environment
}

TEST_F( joystickTest, IsVirtualWithInvalidId ) {
    // Using an improbable id should be safe and return false
    EXPECT_NO_FATAL_FAILURE( {
        bool l_v = isVirtual( static_cast< id_t >( 0xDEADBEEF ) );
        ( void )l_v;
    } );
}
