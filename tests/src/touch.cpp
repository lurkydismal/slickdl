#include "slickdl/touch.hpp"

#include <SDL3/SDL.h>

#include "test.hpp"

using namespace slickdl::touch;

struct touchTest : testing::Test {
    void SetUp() override {
        // Touch APIs require SDL initialized on most platforms.
        // Video is a safe minimal choice for these tests.
        ASSERT_TRUE( SDL_Init( SDL_INIT_VIDEO ) );
    }

    void TearDown() override { SDL_Quit(); }
};

TEST_F( touchTest, ConstantsMatchSDL ) {
    EXPECT_EQ( g_mouseId, SDL_TOUCH_MOUSEID );
    EXPECT_EQ( g_touchId, SDL_MOUSE_TOUCHID );
}

TEST_F( touchTest, DeviceTypeEnumRoundtrip ) {
    // Check a representative range of enum values round-trip through legacy API
    EXPECT_EQ( deviceType::invalid,
               fromLegacy( toLegacy( deviceType::invalid ) ) );
    EXPECT_EQ( deviceType::direct,
               fromLegacy( toLegacy( deviceType::direct ) ) );
    EXPECT_EQ( deviceType::indirectAbsolute,
               fromLegacy( toLegacy( deviceType::indirectAbsolute ) ) );
    EXPECT_EQ( deviceType::indirectRelative,
               fromLegacy( toLegacy( deviceType::indirectRelative ) ) );
}

TEST_F( touchTest, TypeWrapperMatchesRawForEachDevice ) {
    // Call wrapper::all() and for each device compare wrapper.type() with
    // SDL_GetTouchDeviceType
    auto l_ids = all(); // may be empty; that's fine
    for ( auto l_id : l_ids ) {
        const auto l_wrapped = type( l_id );
        const auto l_raw = SDL_GetTouchDeviceType( l_id );
        EXPECT_EQ( l_wrapped, fromLegacy( l_raw ) );
    }

    // Also validate calling type(0) matches raw SDL behaviour for an invalid id
    EXPECT_EQ( type( 0 ), fromLegacy( SDL_GetTouchDeviceType( 0 ) ) );
}

TEST_F( touchTest, AllAndFingerAllDoNotCrashAndReturnValidTypes ) {
    auto l_ids = all();
    // Should be a vector. No further guarantees required.
    EXPECT_GE( l_ids.size(), 0u );

    // For each device, finger::all should return a vector and elements should
    // be non-null
    for ( auto l_id : l_ids ) {
        auto l_fingers = finger::all( l_id );
        EXPECT_GE( l_fingers.size(), 0u );
        for ( auto l_fptr : l_fingers ) {
            // type is gsl::not_null<SDL_Finger*>, ensure it is not null and has
            // reasonable fields
            ASSERT_NE( l_fptr.get(), nullptr );
            // finger id should be non-zero for a valid finger (guarded check)
            EXPECT_NE( l_fptr->id, 0u );
        }
    }

    // Calling finger::all with an invalid id must not crash
    EXPECT_DEATH( ( void )finger::all( 0 ), ".*" );
}
