#include "slickdl/color.hpp"

#include <initializer_list>

#include "test.hpp"

using namespace slickdl;

TEST( ColorTest, PackUnpackRoundTrip ) {
    color_t l_c1( 10u, 20u, 30u, 40u );
    uint32_t l_packed = l_c1.pack();

    color_t l_c2;
    l_c2.unpack( l_packed );

    EXPECT_EQ( l_c2.red, ( uint8_t )10u );
    EXPECT_EQ( l_c2.green, ( uint8_t )20u );
    EXPECT_EQ( l_c2.blue, ( uint8_t )30u );
    EXPECT_EQ( l_c2.alpha, ( uint8_t )40u );
}

TEST( ColorTest, ConstructFromPacked ) {
    color_t l_orig( 255u, 128u, 64u, 32u );
    uint32_t l_p = l_orig.pack();
    color_t l_fromPacked( l_p );

    EXPECT_EQ( l_fromPacked.red, l_orig.red );
    EXPECT_EQ( l_fromPacked.green, l_orig.green );
    EXPECT_EQ( l_fromPacked.blue, l_orig.blue );
    EXPECT_EQ( l_fromPacked.alpha, l_orig.alpha );
}

TEST( ColorTest, PackIsStable ) {
    // Roundtrip multiple values
    for ( uint8_t l_r :
          { ( uint8_t )0, ( uint8_t )1, ( uint8_t )127, ( uint8_t )255 } ) {
        for ( uint8_t l_g : { ( uint8_t )0, ( uint8_t )50, ( uint8_t )200 } ) {
            color_t l_a( l_r, l_g, 5u, 6u );
            color_t l_b;
            l_b.unpack( l_a.pack() );
            EXPECT_EQ( l_a.red, l_b.red );
            EXPECT_EQ( l_a.green, l_b.green );
            EXPECT_EQ( l_a.blue, l_b.blue );
            EXPECT_EQ( l_a.alpha, l_b.alpha );
        }
    }
}

static constexpr float g_epsF = 1.0f / 255.0f + 1e-6f;

TEST( ColorTest, FromFloatAndBack ) {
    SDL_FColor l_fIn{ .r = 0.2f, .g = 0.4f, .b = 0.6f, .a = 0.8f };
    color_t l_c( l_fIn );
    auto l_fOut = static_cast< SDL_FColor >( l_c );

    EXPECT_NEAR( l_fIn.r, l_fOut.r, g_epsF );
    EXPECT_NEAR( l_fIn.g, l_fOut.g, g_epsF );
    EXPECT_NEAR( l_fIn.b, l_fOut.b, g_epsF );
    EXPECT_NEAR( l_fIn.a, l_fOut.a, g_epsF );
}

TEST( ColorTest, ExtremesFloatConversion ) {
    // zero -> 0.0f
    SDL_FColor l_f0{ .r = 0.f, .g = 0.f, .b = 0.f, .a = 0.f };
    color_t l_c0( l_f0 );
    EXPECT_EQ( l_c0.red, 0u );
    EXPECT_EQ( l_c0.green, 0u );
    EXPECT_EQ( l_c0.blue, 0u );
    EXPECT_EQ( l_c0.alpha, 0u );

    // one -> max
    SDL_FColor l_f1{ .r = 1.f, .g = 1.f, .b = 1.f, .a = 1.f };
    color_t l_c1( l_f1 );
    EXPECT_EQ( l_c1.red, color_t::g_maxValue );
    EXPECT_EQ( l_c1.green, color_t::g_maxValue );
    EXPECT_EQ( l_c1.blue, color_t::g_maxValue );
    EXPECT_EQ( l_c1.alpha, color_t::g_maxValue );
}
