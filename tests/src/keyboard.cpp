#include "slickdl/keyboard.hpp"

#include <SDL3/SDL.h>

#include "test.hpp"

using namespace slickdl;
using namespace slickdl::keyboard;

struct keyboardTest : testing::Test {
    void SetUp() override {
        // video subsystem is needed for windows and text input
        ASSERT_TRUE( SDL_Init( SDL_INIT_VIDEO ) );
        m_window = SDL_CreateWindow( "test", 64, 64, 0 );
        ASSERT_NE( m_window, nullptr );
    }

    void TearDown() override {
        if ( m_window ) {
            SDL_DestroyWindow( m_window );
            m_window = nullptr;
        }
        SDL_Quit();
    }

    SDL_Window* m_window = nullptr;
};

TEST_F( keyboardTest, EnumRoundtrip ) {
    // code_t <-> legacy
    {
        auto l_legacy = toLegacy( code_t::a );
        auto l_back = fromLegacy( l_legacy );
        EXPECT_EQ( l_back, code_t::a );
    }

    // modifier roundtrip
    {
        auto l_legacy = toLegacy( modifier_t::leftCtrl );
        auto l_back = fromLegacy( l_legacy );
        EXPECT_EQ( l_back, modifier_t::leftCtrl );
    }
}

TEST_F( keyboardTest, BasicQueries ) {
    // any and screen keyboard support are simple booleans
    bool l_has = any();
    ( void )l_has;

    bool l_screen = hasScreenKeyboard();
    ( void )l_screen;
}

TEST_F( keyboardTest, KeyNameAndFromName ) {
    // keyFromName should return a valid code for a normal key name
    const auto l_code = keyFromName( "a" );
    EXPECT_NE( l_code, code_t::unknown );

    const auto l_name = keyName( l_code );
    EXPECT_FALSE( l_name.empty() );
}

TEST_F( keyboardTest, ScancodeConversions ) {
    // Convert from key to scancode and back-ish
    const auto l_code = keyFromName( "a" );
    const auto l_sc = scancodeFromKey( l_code );
    EXPECT_NE( l_sc, scancode_t::unknown );

    // scancodeToKeycode should set a single "extended" bit and preserve the
    // scancode bits.
    const auto l_arbitrarySc = static_cast< scancode_t >( 5 );
    const auto l_k = scancodeToKeycode( l_arbitrarySc );
    const auto l_kv = static_cast< codeUnderlying_t >( l_k );
    const auto l_scVal = static_cast< codeUnderlying_t >( l_arbitrarySc );

    // keycode must not equal raw scancode
    EXPECT_NE( l_kv, l_scVal );

    // difference should be a single bit (power of two)
    const auto l_diff = l_kv ^ l_scVal;
    EXPECT_NE( l_diff, 0u );
    EXPECT_EQ( l_diff & ( l_diff - 1u ), 0u ); // ensures diff is a power of two

    // preserved lower bits equal the original scancode
    EXPECT_EQ( ( l_kv & ~l_diff ), l_scVal );
}

TEST_F( keyboardTest, ModifierSetGet ) {
    // preserve original then restore
    const auto l_orig = modifier();
    modifier( modifier_t::leftCtrl );
    EXPECT_EQ( modifier(), modifier_t::leftCtrl );
    modifier( l_orig );
    EXPECT_EQ( modifier(), l_orig );
}

TEST_F( keyboardTest, TextInputStartStop ) {
    // start/stop text input on a window should not crash
    EXPECT_NO_FATAL_FAILURE( text_input::start( m_window ) );
    // Query active state (may be true or false depending on platform)
    bool l_active = text_input::isActive( m_window );
    ( void )l_active;
    EXPECT_NO_FATAL_FAILURE( text_input::stop( m_window ) );
}
