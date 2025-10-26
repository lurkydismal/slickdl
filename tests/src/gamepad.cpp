#include "slickdl/gamepad.hpp"

#include <SDL3/SDL.h>

#include "test.hpp"

using namespace slickdl;
using namespace slickdl::gamepad;

struct gamepadTest : testing::Test {
    void SetUp() override { ASSERT_TRUE( SDL_Init( SDL_INIT_GAMEPAD ) ); }
    void TearDown() override { SDL_Quit(); }
};

TEST_F( gamepadTest, EnumToLegacyRoundtrip ) {
    // type_t
    {
        auto l_legacy = toLegacy( type_t::xbox360 );
        auto l_back = fromLegacy( l_legacy );
        EXPECT_EQ( l_back, type_t::xbox360 );
    }

    // axis_t
    {
        auto l_legacy = toLegacy( axis_t::leftX );
        auto l_back = fromLegacy( l_legacy );
        EXPECT_EQ( l_back, axis_t::leftX );
    }

    // button_t
    {
        auto l_legacy = toLegacy( button_t::south );
        auto l_back = fromLegacy( l_legacy );
        EXPECT_EQ( l_back, button_t::south );
    }

    // bindingType_t
    {
        auto l_legacy = toLegacy( bindingType_t::axis );
        auto l_back = fromLegacy( l_legacy );
        EXPECT_EQ( l_back, bindingType_t::axis );
    }
}

TEST_F( gamepadTest, TypeStringRoundtrip ) {
    // roundtrip: enum -> string -> enum
    const auto l_s = toString( type_t::standard );
    EXPECT_FALSE( l_s.empty() );
    const auto l_t = type( l_s );
    EXPECT_EQ( l_t, type_t::standard );
}

TEST_F( gamepadTest, AxisStringRoundtrip ) {
    const auto l_s = toString( axis_t::leftX );
    EXPECT_FALSE( l_s.empty() );
    const auto l_a = axis( l_s );
    EXPECT_EQ( l_a, axis_t::leftX );
}

TEST_F( gamepadTest, ButtonStringRoundtrip ) {
    const auto l_s = toString( button_t::south );
    EXPECT_FALSE( l_s.empty() );
    const auto l_b = button( l_s );
    EXPECT_EQ( l_b, button_t::south );
}

TEST_F( gamepadTest, AxisAndButtonToStringNonEmpty ) {
    // ensure SDL provided strings exist for a few enums
    EXPECT_FALSE( toString( axis_t::rightY ).empty() );
    EXPECT_FALSE( toString( button_t::east ).empty() );
}

TEST_F( gamepadTest, EventsToggleAndQuery ) {
    // start disabled (implementation-defined). Toggle on and off.
    eventsToggle( true );
    EXPECT_TRUE( areEventsEnabled() );
    eventsToggle( false );
    EXPECT_FALSE( areEventsEnabled() );
}

TEST_F( gamepadTest, UpdateDoesNotCrash ) {
    // SDL_UpdateGamepads is safe to call even with no devices.
    EXPECT_NO_FATAL_FAILURE( update() );
}
