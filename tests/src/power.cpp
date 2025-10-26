#include "slickdl/power.hpp"

#include <SDL3/SDL.h>

#include "test.hpp"

using namespace slickdl::power;

struct powerTest : testing::Test {
    void SetUp() override {
        // safe minimal init for platform APIs
        ASSERT_TRUE( SDL_Init( SDL_INIT_VIDEO ) );
    }
    void TearDown() override { SDL_Quit(); }
};

TEST_F( powerTest, EnumRoundtripLegacy ) {
    EXPECT_EQ( state_t::error, fromLegacy( toLegacy( state_t::error ) ) );
    EXPECT_EQ( state_t::unknown, fromLegacy( toLegacy( state_t::unknown ) ) );
    EXPECT_EQ( state_t::onBattery,
               fromLegacy( toLegacy( state_t::onBattery ) ) );
    EXPECT_EQ( state_t::noBattery,
               fromLegacy( toLegacy( state_t::noBattery ) ) );
    EXPECT_EQ( state_t::charging, fromLegacy( toLegacy( state_t::charging ) ) );
    EXPECT_EQ( state_t::charged, fromLegacy( toLegacy( state_t::charged ) ) );
}

#if 0
// TODO: Fix
TEST_F( powerTest, PointerToLegacyReflectsChange ) {
    state_t l_s = state_t::unknown;
    SDL_PowerState* l_p = toLegacy( &l_s );
    // change via legacy pointer
    *l_p = SDL_POWERSTATE_ON_BATTERY;
    EXPECT_EQ( l_s, state_t::onBattery );

    // restore through pointer
    *l_p = toLegacy( state_t::charged );
    EXPECT_EQ( l_s, state_t::charged );
}
#endif

TEST_F( powerTest, InfoMatchesRawSDL ) {
    int l_rawSecs = -2;
    int l_rawPercent = -2;
    SDL_PowerState l_rawState = SDL_GetPowerInfo( &l_rawSecs, &l_rawPercent );

    auto l_wrapped = info();

    // state must match via conversion
    EXPECT_EQ( l_wrapped.first, fromLegacy( l_rawState ) );

    // SDL returns -1 when unknown/no-battery for seconds/percent.
    if ( l_rawSecs >= 0 && l_rawPercent >= 0 ) {
        ASSERT_TRUE( l_wrapped.second.has_value() );
        auto [ s_secs, s_percent ] = l_wrapped.second.value();
        EXPECT_EQ( s_secs, static_cast< size_t >( l_rawSecs ) );
        EXPECT_EQ( s_percent, static_cast< size_t >( l_rawPercent ) );
    } else {
        EXPECT_FALSE( l_wrapped.second.has_value() );
    }
}
