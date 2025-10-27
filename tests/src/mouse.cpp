#include "slickdl/mouse.hpp"

#include <SDL3/SDL.h>

#include "test.hpp"

using namespace slickdl;

struct mouseTest : testing::Test {
    void SetUp() override { ASSERT_TRUE( SDL_Init( SDL_INIT_VIDEO ) ); }

    void TearDown() override { SDL_Quit(); }
};

TEST_F( mouseTest, MaskValuesAndMaskFunction ) {
    // Constants
    EXPECT_EQ( mouse::g_leftMask, 1u );
    EXPECT_EQ( mouse::g_middleMask, 2u );
    EXPECT_EQ( mouse::g_rightMask, 4u );
    EXPECT_EQ( mouse::g_x1Mask, 8u );
    EXPECT_EQ( mouse::g_x2Mask, 16u );

    // mask(...) behaviour
    EXPECT_EQ( mouse::mask( mouse::buttonFlags::left ), 1u );
    EXPECT_EQ( mouse::mask( mouse::buttonFlags::middle ), 2u );
    EXPECT_EQ( mouse::mask( mouse::buttonFlags::right ), 4u );
    EXPECT_EQ( mouse::mask( mouse::buttonFlags::x1 ), 8u );
    EXPECT_EQ( mouse::mask( mouse::buttonFlags::x2 ), 16u );
}

TEST_F( mouseTest, SystemCursorRoundtripAndActive ) {
    // roundtrip conversion
    const auto l_in = mouse::systemCursor_t::pointer;
    const auto l_legacy = mouse::toLegacy( l_in );
    const auto l_out = mouse::fromLegacy( l_legacy );
    EXPECT_EQ( l_in, l_out );

    // create system cursor and set it active
    mouse::cursor l_c( mouse::systemCursor_t::pointer );
    ASSERT_NO_THROW( l_c.set() );

    const auto l_active = mouse::cursor::active();
    ASSERT_TRUE( l_active.has_value() );
    // basic sanity: pointer returned should not be null as optional had value
    EXPECT_NE( static_cast< SDL_Cursor* >( *l_active ), nullptr );
}

TEST_F( mouseTest, ShowHideVisibility ) {
    // Ensure show/hide APIs run and visibility toggles
    // Show
    ASSERT_NO_THROW( mouse::cursor::show() );
    EXPECT_TRUE( mouse::cursor::isVisible() );

    // Hide
    ASSERT_NO_THROW( mouse::cursor::hide() );
    EXPECT_FALSE( mouse::cursor::isVisible() );
}

TEST_F( mouseTest, StateFunctionsReturnReasonableValues ) {
    {
        const auto [ flags, pt ] = mouse::state();
        // flags is an enum class; convert to integer for checks
        const auto l_val = static_cast< uint32_t >( flags );
        // bitmask should be within a small range (no crazy large values)
        EXPECT_LE( l_val, ( 1u << 8 ) );
        EXPECT_TRUE( std::isfinite( pt.x ) );
        EXPECT_TRUE( std::isfinite( pt.y ) );
    }

    {
        const auto [ flags, pt ] = mouse::stateGlobal();
        const auto l_val = static_cast< uint32_t >( flags );
        EXPECT_LE( l_val, ( 1u << 8 ) );
        EXPECT_TRUE( std::isfinite( pt.x ) );
        EXPECT_TRUE( std::isfinite( pt.y ) );
    }

    {
        const auto [ flags, pt ] = mouse::stateRelative();
        const auto l_val = static_cast< uint32_t >( flags );
        EXPECT_LE( l_val, ( 1u << 8 ) );
        EXPECT_TRUE( std::isfinite( pt.x ) );
        EXPECT_TRUE( std::isfinite( pt.y ) );
    }
}

TEST_F( mouseTest, WarpWindowNoWindowAndRelativeTransformNull ) {
    // Warp without providing a window (calls
    // SDL_WarpMouseInWindow(nullptr,...)) This function has no return. Just
    // ensure it doesn't crash.
    ASSERT_NO_THROW( mouse::warpWindow( { 10.0f, 20.0f } ) );

    // Setting relative transform to null (default) should be allowed.
    ASSERT_NO_THROW(
        mouse::relativeTransformHandler( std::nullopt, std::nullopt ) );
}
