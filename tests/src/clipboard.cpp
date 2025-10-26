#include "slickdl/clipboard.hpp"

#include <SDL3/SDL.h>

#include <string>

#include "test.hpp"

using namespace slickdl::clipboard;

struct clipboardTest : testing::Test {
    void SetUp() override {
        // init video subsystem; clipboard APIs require SDL initialized on most
        // platforms
        ASSERT_TRUE( SDL_Init( SDL_INIT_VIDEO ) );
        // Ensure clean slate
        clear();
    }

    void TearDown() override {
        clear();
        SDL_Quit();
    }
};

TEST_F( clipboardTest, SetGetText ) {
    const std::string l_text = "hello from test";
    slickdl::clipboard::text( l_text );
    auto l_got = slickdl::clipboard::text();
    ASSERT_EQ( l_got, l_text );
    EXPECT_TRUE( slickdl::clipboard::hasText() );

    // clear and verify empty
    slickdl::clipboard::clear();
    EXPECT_FALSE( slickdl::clipboard::hasText() );
    EXPECT_EQ( slickdl::clipboard::text(), std::string{} );
}

TEST_F( clipboardTest, PrimarySelectionSetGet ) {
    const std::string l_ps = "primary selection text";
    slickdl::clipboard::primary_selection::text( l_ps );
    auto l_got = slickdl::clipboard::primary_selection::text();
    ASSERT_EQ( l_got, l_ps );
    EXPECT_TRUE( slickdl::clipboard::primary_selection::hasText() );

    slickdl::clipboard::primary_selection::text( "" );
    EXPECT_FALSE( slickdl::clipboard::primary_selection::hasText() );
}

TEST_F( clipboardTest, ClearClipboardDataDoesNotCrash ) {
    // Put something and then clear. Should not crash and hasText should be
    // false.
    slickdl::clipboard::text( "temporary" );
    ASSERT_TRUE( slickdl::clipboard::hasText() );
    slickdl::clipboard::clear();
    EXPECT_FALSE( slickdl::clipboard::hasText() );
}

TEST_F( clipboardTest, HasTextReflectsEmptyString ) {
    // Explicitly set empty string and ensure hasText is false per wrapper
    // behavior
    slickdl::clipboard::text( "" );
    EXPECT_FALSE( slickdl::clipboard::hasText() );
    EXPECT_EQ( slickdl::clipboard::text(), std::string{} );
}
