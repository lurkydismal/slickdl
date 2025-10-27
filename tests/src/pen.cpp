#include "slickdl/pen.hpp"

#include <SDL3/SDL.h>

#include "test.hpp"

using namespace slickdl::pen;

struct penTest : testing::Test {
    void SetUp() override { ASSERT_TRUE( SDL_Init( SDL_INIT_VIDEO ) ); }
    void TearDown() override { SDL_Quit(); }
};

TEST_F( penTest, ConstantsMatchSDL ) {
    EXPECT_EQ( g_mouseId, SDL_PEN_MOUSEID );
    EXPECT_EQ( g_touchId, SDL_PEN_TOUCHID );

    // eraser tip should be (1u << 30)
    constexpr uint32_t l_expectedEraser = ( 1u << 30 );
    EXPECT_EQ( static_cast< inputFlagsUnderlying_t >( inputFlags::eraserTip ),
               l_expectedEraser );
}

TEST_F( penTest, BitwiseOperatorsOrAnd ) {
    const inputFlags_t l_a = inputFlags::down;
    const inputFlags_t l_b = inputFlags::button1;

    const inputFlags_t l_orv = ( l_a | l_b );
    const auto l_orvRaw = static_cast< inputFlagsUnderlying_t >( l_orv );

    EXPECT_EQ(
        l_orvRaw,
        static_cast< inputFlagsUnderlying_t >( inputFlags::down ) |
            static_cast< inputFlagsUnderlying_t >( inputFlags::button1 ) );

    const inputFlags_t l_andv = ( l_orv & l_a );
    EXPECT_EQ( static_cast< inputFlagsUnderlying_t >( l_andv ),
               static_cast< inputFlagsUnderlying_t >( inputFlags::down ) );

    // non-overlap yields zero underlying
    const inputFlags_t l_none = ( l_a & inputFlags::button2 );
    EXPECT_EQ( static_cast< inputFlagsUnderlying_t >( l_none ), 0u );
}

TEST_F( penTest, LegacyRoundtripForFlags ) {
    const inputFlags_t l_flags =
        inputFlags::down | inputFlags::button3 | inputFlags::eraserTip;

    const SDL_PenInputFlags l_legacy = toLegacy( l_flags );
    const inputFlags_t l_back = fromLegacy( l_legacy );

    EXPECT_EQ( static_cast< inputFlagsUnderlying_t >( l_flags ),
               static_cast< inputFlagsUnderlying_t >( l_back ) );
}

TEST_F( penTest, AxisEnumRoundtripAndRange ) {
    const auto l_cnt = static_cast< axisUnderlying_t >( axis::count );
    EXPECT_GT( l_cnt, 0 );

    for ( axisUnderlying_t l_i = 0; l_i < l_cnt; ++l_i ) {
        const auto l_a = static_cast< axis_t >( l_i );
        const SDL_PenAxis l_legacy = toLegacy( l_a );
        const axis_t l_back = fromLegacy( l_legacy );

        EXPECT_EQ( static_cast< axisUnderlying_t >( l_a ),
                   static_cast< axisUnderlying_t >( l_back ) );
    }
}
