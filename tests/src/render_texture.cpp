#include <SDL3/SDL.h>

#include "slickdl/blend.hpp"
#include "slickdl/color.hpp"
#include "slickdl/point.hpp"
#include "slickdl/surface.hpp"
#include "test.hpp"

using namespace slickdl;

struct surfaceTest : testing::Test {
    void SetUp() override { ASSERT_TRUE( SDL_Init( SDL_INIT_VIDEO ) ); }
    void TearDown() override { SDL_Quit(); }
};

// Helper to create a raw SDL surface and wrap it.
static auto makeRgbaSurface( int _w = 16, int _h = 16 ) -> surface {
    surface_t l_raw = SDL_CreateSurface( _w, _h, SDL_PIXELFORMAT_RGBA8888 );
    return ( l_raw ); // wrapper will take ownership
}

TEST_F( surfaceTest, CreateAndMustLock ) {
    auto l_s = makeRgbaSurface();
    // default created surface should not require lock
    EXPECT_FALSE( l_s.mustLock() );
}

TEST_F( surfaceTest, WriteAndReadPixelRoundtrip ) {
    auto l_s = makeRgbaSurface( 8, 8 );
    point_t< int > l_p{ 2, 3 };
    color_t l_c( 10, 20, 30, 40 );

    l_s.write( l_p, l_c );

    auto l_read = l_s.read( l_p );
    EXPECT_EQ( l_read.red, l_c.red );
    EXPECT_EQ( l_read.green, l_c.green );
    EXPECT_EQ( l_read.blue, l_c.blue );
    EXPECT_EQ( l_read.alpha, l_c.alpha );
}

TEST_F( surfaceTest, MapAndFillThenRead ) {
    auto l_s = makeRgbaSurface( 4, 4 );
    color_t l_c( 0x11, 0x22, 0x33, 0x44 );

    // map to pixel value for this surface format
    auto l_px = l_s.map< uint32_t >( l_c );

    // fill entire surface and check a sample pixel
    l_s.fill( l_px );

    auto l_got = l_s.read( { 0, 0 } );
    EXPECT_EQ( l_got.red, l_c.red );
    EXPECT_EQ( l_got.green, l_c.green );
    EXPECT_EQ( l_got.blue, l_c.blue );
    EXPECT_EQ( l_got.alpha, l_c.alpha );
}

TEST_F( surfaceTest, ColorModAndAlphaMod ) {
    auto l_s = makeRgbaSurface();
    color_t l_mod( 5, 6, 7, 255 );

    l_s.colorMod( l_mod );
    auto l_got = l_s.colorMod();
    EXPECT_EQ( l_got.red, l_mod.red );
    EXPECT_EQ( l_got.green, l_mod.green );
    EXPECT_EQ( l_got.blue, l_mod.blue );

    l_s.alphaMod( 123 );
    EXPECT_EQ( l_s.alphaMod(), 123u );
}

TEST_F( surfaceTest, BlendAndClip ) {
    auto l_s = makeRgbaSurface();
    // blend mode set/get
    l_s.blend( blend_t::blend );
    EXPECT_EQ( l_s.blend(), blend_t::blend );

    // clip set/get
    clippingZone_t< int > l_cz( 1, 2, 5, 6 );
    l_s.clip( l_cz );
    auto l_cz2 = l_s.clip();
    EXPECT_EQ( l_cz2.minX, l_cz.minX );
    EXPECT_EQ( l_cz2.minY, l_cz.minY );
    EXPECT_EQ( l_cz2.maxX, l_cz.maxX );
    EXPECT_EQ( l_cz2.maxY, l_cz.maxY );
}

TEST_F( surfaceTest, ScaleReturnsValidSurface ) {
    auto l_s = makeRgbaSurface( 8, 8 );
    auto l_s2 = l_s.scale( 4, 4, scale_t::nearest );
    // should be non-null wrapper (owns SDL_Surface*)
    // read/write roundtrip on scaled surface
    color_t l_c( 7, 8, 9, 10 );
    l_s2.write( point_t< int >{ 0, 0 }, l_c );
    auto l_got = l_s2.read( { 0, 0 } );
    EXPECT_EQ( l_got.red, l_c.red );
    EXPECT_EQ( l_got.green, l_c.green );
    EXPECT_EQ( l_got.blue, l_c.blue );
    EXPECT_EQ( l_got.alpha, l_c.alpha );
}
