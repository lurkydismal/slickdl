#include "slickdl/scancode.hpp"

#include <SDL3/SDL_scancode.h>

#include "test.hpp"

using namespace slickdl;

TEST( ScancodeTest, RoundtripLegacy ) {
    constexpr scancode_t l_samples[] = {
        scancode::a,         scancode::z,     scancode::number1,
        scancode::enter,     scancode::space, scancode::leftCtrl,
        scancode::rightMeta, scancode::count,
    };

    for ( auto l_s : l_samples ) {
        auto l_legacy = toLegacy( l_s );
        auto l_back = fromLegacy( l_legacy );
        EXPECT_EQ( l_back, l_s );
    }
}

#if 0
TEST( ScancodeTest, PointerToLegacyAltersOriginal ) {
    scancode_t l_v = scancode::enter;
    SDL_Scancode* l_p = toLegacy( &l_v );

    // underlying numeric values must match
    EXPECT_EQ( static_cast< scancodeUnderlying_t >( *l_p ),
               static_cast< scancodeUnderlying_t >( l_v ) );

    // Change through legacy pointer and see original change
    *l_p = SDL_SCANCODE_SPACE;
    EXPECT_EQ( l_v, scancode::space );

    // restore
    *l_p = static_cast< SDL_Scancode >( scancode::enter );
    EXPECT_EQ( l_v, scancode::enter );
}
#endif

TEST( ScancodeTest, BitwiseOperatorsProduceExpectedUnderlyingValue ) {
    scancode_t l_a = scancode::a;         // 4
    scancode_t l_b = scancode::b;         // 5
    scancode_t l_left = scancode::left;   // 80
    scancode_t l_right = scancode::right; // 79

    auto l_orAb = ( l_a | l_b );
    auto l_andLr = ( l_left & l_right );

    EXPECT_EQ( static_cast< scancodeUnderlying_t >( l_orAb ),
               static_cast< scancodeUnderlying_t >( l_a ) |
                   static_cast< scancodeUnderlying_t >( l_b ) );

    EXPECT_EQ( static_cast< scancodeUnderlying_t >( l_andLr ),
               static_cast< scancodeUnderlying_t >( l_left ) &
                   static_cast< scancodeUnderlying_t >( l_right ) );
}

TEST( ScancodeTest, CountValueIsExpected ) {
    EXPECT_EQ( static_cast< scancodeUnderlying_t >( scancode::count ), 512u );
}
