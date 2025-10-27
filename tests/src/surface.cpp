#include "slickdl/surface.hpp"

#include <SDL3/SDL.h>

#include "slickdl/blend.hpp"
#include "slickdl/clipping_zone.hpp"
#include "slickdl/color.hpp"
#include "test.hpp"

using namespace slickdl;

struct surfaceTest : testing::Test {
    void SetUp() override { ASSERT_TRUE( SDL_Init( SDL_INIT_VIDEO ) ); }
    void TearDown() override { SDL_Quit(); }
};

TEST_F( surfaceTest, CreateAndBasicProperties ) {
    SDL_Surface* l_raw = SDL_CreateSurface( 32, 32, SDL_PIXELFORMAT_RGBA8888 );
    ASSERT_NE( l_raw, nullptr );

    surface l_src( l_raw ); // takes ownership through wrapper

    // mustLock should be false for a normal surface
    EXPECT_FALSE( l_src.mustLock() );

    // lock / unlock must succeed (wrapper asserts on failure)
    l_src.lock();
    l_src.unlock();

    // palette creation/get should work (palette was created in ctor used by
    // other constructors, but for this raw path SDL_CreateSurface might not
    // have one. We just call properties())
    auto l_props = l_src.properties();
    EXPECT_NE( l_props, 0u );
}

TEST_F( surfaceTest, ColorMapReadWriteAndMods ) {
    SDL_Surface* l_raw = SDL_CreateSurface( 16, 16, SDL_PIXELFORMAT_RGBA8888 );
    ASSERT_NE( l_raw, nullptr );
    surface l_s( l_raw );

    // map color to pixel value
    color_t l_inColor{ 255, 10, 20, 255 };
    auto l_pix = l_s.map< uint32_t >( l_inColor );
    EXPECT_NE( l_pix, 0u );

    // write then read
    l_s.write( point_t< int >{ 4, 5 }, l_inColor );
    color_t l_out = l_s.read( { 4, 5 } );
    EXPECT_EQ( l_out.red, l_inColor.red );
    EXPECT_EQ( l_out.green, l_inColor.green );
    EXPECT_EQ( l_out.blue, l_inColor.blue );
    EXPECT_EQ( l_out.alpha, l_inColor.alpha );

    // color mod and get
    color_t l_mod{ 10, 20, 30, 255 };
    l_s.colorMod( l_mod );
    color_t l_gotMod = l_s.colorMod();
    EXPECT_EQ( l_gotMod.red, l_mod.red );
    EXPECT_EQ( l_gotMod.green, l_mod.green );
    EXPECT_EQ( l_gotMod.blue, l_mod.blue );

    // alpha mod
    l_s.alphaMod( 123 );
    EXPECT_EQ( l_s.alphaMod(), 123 );
}

TEST_F( surfaceTest, FillAndBlendAndClip ) {
    SDL_Surface* l_raw = SDL_CreateSurface( 8, 8, SDL_PIXELFORMAT_RGBA8888 );
    ASSERT_NE( l_raw, nullptr );
    surface l_s( l_raw );

    // fill whole surface with opaque red (RGBA)
    uint32_t l_redPixel = 0xFF0000FFu;
    l_s.fill( l_redPixel );

    color_t l_c = l_s.read( { 0, 0 } );
    EXPECT_EQ( l_c.red, 255 );
    EXPECT_EQ( l_c.alpha, 255 );

    // set blend and read back
    l_s.blend( blend_t::add );
    EXPECT_EQ( l_s.blend(), blend_t::add );

    // clip zone set/get
    clippingZone_t< int > l_z{ 1, 1, 4, 4 };
    l_s.clip( l_z );
    auto l_got = l_s.clip();
    EXPECT_EQ( l_got.minX, l_z.minX );
    EXPECT_EQ( l_got.minY, l_z.minY );
    EXPECT_EQ( l_got.maxX, l_z.maxX );
    EXPECT_EQ( l_got.maxY, l_z.maxY );
}

TEST_F( surfaceTest, AlternatesAndScaleAndConvert ) {
    SDL_Surface* l_rawA = SDL_CreateSurface( 24, 24, SDL_PIXELFORMAT_RGBA8888 );
    SDL_Surface* l_rawB = SDL_CreateSurface( 12, 12, SDL_PIXELFORMAT_RGBA8888 );
    ASSERT_NE( l_rawA, nullptr );
    ASSERT_NE( l_rawB, nullptr );

    surface l_a( l_rawA );
    surface l_b( l_rawB );

    // add alternate image and check
    l_a.addAlternate( l_b );
    EXPECT_TRUE( l_a.hasAlternate() );

    // remove alternates
    l_a.removeAlternates();
    EXPECT_FALSE( l_a.hasAlternate() );

    // scaling returns a new surface (non-null)
    surface l_scaled = l_a.scale( 6, 6, scale_t::nearest );
    ASSERT_NE( static_cast< SDL_Surface* >( l_scaled ), nullptr );
    // basic expectation about dimensions (SDL_Surface struct exposes w/h)
    EXPECT_EQ( static_cast< SDL_Surface* >( l_scaled )->w, 6 );
    EXPECT_EQ( static_cast< SDL_Surface* >( l_scaled )->h, 6 );

    // convert to same format should succeed
    surface l_conv = l_a.convert( pixels::format_t::fRGBA8888 );
    ASSERT_NE( static_cast< SDL_Surface* >( l_conv ), nullptr );
}

TEST_F( surfaceTest, PremultiplyAndClearAndPixelFloatWrite ) {
    SDL_Surface* l_raw = SDL_CreateSurface( 10, 10, SDL_PIXELFORMAT_RGBA8888 );
    ASSERT_NE( l_raw, nullptr );
    surface l_s( l_raw );

    // premultiply alpha (linear false)
    l_s.premultiplyAlpha( false );

    // clear using float16 values; use 1.0 for components
    auto l_one = static_cast< float16_t >( 1.0f );
    l_s.clear( l_one, static_cast< float16_t >( 0.0f ),
               static_cast< float16_t >( 0.0f ), l_one );

    color_t l_c = l_s.read( { 0, 0 } );
    // after clear to full red expect red channel high and alpha high
    EXPECT_GE( l_c.red,
               240 ); // a tolerant check in case of color space conversions
    EXPECT_GE( l_c.alpha, 240 );

    // write float pixel (uses floating API)
    color_t l_blue{ 0, 0, 255, 255 };
    l_s.write( point_t< float >{ 2.0f, 2.0f }, l_blue );
    color_t l_out = l_s.read( { 2, 2 } );
    EXPECT_EQ( l_out.blue, 255 );
}

TEST_F( surfaceTest, BlitOperationsSmoke ) {
    SDL_Surface* l_srcRaw =
        SDL_CreateSurface( 16, 16, SDL_PIXELFORMAT_RGBA8888 );
    SDL_Surface* l_dstRaw =
        SDL_CreateSurface( 32, 32, SDL_PIXELFORMAT_RGBA8888 );
    ASSERT_NE( l_srcRaw, nullptr );
    ASSERT_NE( l_dstRaw, nullptr );

    surface l_src( l_srcRaw );
    surface l_dst( l_dstRaw );

    // fill source with a distinctive color
    l_src.fill( 0x00FF00FFu ); // green
    // normal blit
    l_src.blit( l_dst );
    // scaled blit
    l_src.blit( l_dst, scale_t::nearest );
    // tiled blit
    l_src.blitTiled( l_dst );
    // scaled tiled blit with scale amount
    l_src.blitTiled( l_dst, scale_t::nearest,
                     static_cast< float16_t >( 1.0f ) );

    // 9-grid blit with a zone (choose small zone)
    clippingZone_t< int > l_z{ 1, 2, 4, 5 };
    l_src.blit9Grid( l_dst, l_z, scale_t::nearest,
                     static_cast< float16_t >( 1.0f ) );

    // stretch
    l_src.stretch( l_dst, scale_t::linear );
}
