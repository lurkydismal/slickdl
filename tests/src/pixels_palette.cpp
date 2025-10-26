#include "slickdl/pixels_palette.hpp"

#include <SDL3/SDL.h>

#include <array>

#include "test.hpp"

using namespace slickdl;

struct pixelsTest : testing::Test {
    void SetUp() override { ASSERT_TRUE( SDL_Init( SDL_INIT_VIDEO ) ); }
    void TearDown() override { SDL_Quit(); }
};

static void expectColorEq( const color_t& _a, const color_t& _b ) {
    EXPECT_EQ( _a.red, _b.red );
    EXPECT_EQ( _a.green, _b.green );
    EXPECT_EQ( _a.blue, _b.blue );
    EXPECT_EQ( _a.alpha, _b.alpha );
}

TEST_F( pixelsTest, FormatRoundtripLegacy ) {
    constexpr std::array l_formats = {
        pixels::format_t::fRGBA8888,
        pixels::format_t::fRGB24,
        pixels::format_t::fYUY2,
        pixels::format_t::index8,
    };

    for ( auto l_f : l_formats ) {
        auto l_legacy = pixels::toLegacy( l_f );
        auto l_back = pixels::fromLegacy( l_legacy );
        EXPECT_EQ( l_back, l_f );
    }
}

TEST_F( pixelsTest, BitsBytesAndKinds ) {
    // RGBA8888
    EXPECT_EQ( pixels::bits( pixels::format_t::fRGBA8888 ), 32 );
    EXPECT_EQ( pixels::bytes( pixels::format_t::fRGBA8888 ), 4 );
    EXPECT_TRUE( pixels::isPacked( pixels::format_t::fRGBA8888 ) );
    EXPECT_TRUE( pixels::hasAlpha( pixels::format_t::fRGBA8888 ) );

    // RGB24
    EXPECT_EQ( pixels::bits( pixels::format_t::fRGB24 ), 24 );
    EXPECT_EQ( pixels::bytes( pixels::format_t::fRGB24 ), 3 );
    EXPECT_TRUE( pixels::isArray( pixels::format_t::fRGB24 ) );
    EXPECT_FALSE( pixels::hasAlpha( pixels::format_t::fRGB24 ) );

    // index8
    EXPECT_TRUE( pixels::isIndexed( pixels::format_t::index8 ) );
}

TEST_F( pixelsTest, MasksAndFormatForMasksRoundtrip ) {
    auto l_fmt = pixels::format_t::fRGBA8888;
    auto l_om = pixels::masks( l_fmt );
    ASSERT_TRUE( l_om.has_value() );
    auto l_m = l_om.value();

    // round-trip: formatForMasks(m) should recover same or equivalent format
    auto l_f2 = pixels::formatForMasks( l_m );
    EXPECT_EQ( l_f2, l_fmt );

    // Check a few expected mask fields for fRGBA8888 (pl8888 + poRGBA)
    EXPECT_EQ( l_m.bitsPerPixel, 32u );
    // For poRGBA we expect red in high byte on little-endian wrapper
    // implementation The implementation sets red = 0xFF000000, alpha =
    // 0x000000FF for poRGBA path.
    EXPECT_EQ( l_m.red, 0xFF000000u );
    EXPECT_EQ( l_m.green, 0x00FF0000u );
    EXPECT_EQ( l_m.blue, 0x0000FF00u );
    EXPECT_EQ( l_m.alpha, 0x000000FFu );
}

TEST_F( pixelsTest, PixelFormatDetailsAndMapColorRoundtrip ) {
    auto l_fmt = pixels::format_t::fRGBA8888;
    const auto l_details = pixels::pixelFormatDetails( l_fmt );
    ASSERT_NE( l_details.get(), nullptr );

    color_t l_c{ 10, 20, 30, 40 };
    // map -> value -> color using pixel helpers
    auto l_val = pixels::map< uint32_t >( l_details, l_c );
    auto l_round = pixels::color< uint32_t >( l_val, l_details );
    expectColorEq( l_c, l_round );
}

TEST_F( pixelsTest, PaletteMapAndColorRoundtrip ) {
    // Create an 8-bit indexed palette and set its two entries
    palette l_pal( 2 );

    // Fill palette entries via raw SDL API (wrapper exposes SDL_Palette* via
    // operator)
    std::array< SDL_Color, 2 > l_entries{};
    l_entries[ 0 ].r = 15;
    l_entries[ 0 ].g = 25;
    l_entries[ 0 ].b = 35;
    l_entries[ 0 ].a = 255;
    l_entries[ 1 ].r = 200;
    l_entries[ 1 ].g = 210;
    l_entries[ 1 ].b = 220;
    l_entries[ 1 ].a = 255;

    SDL_SetPaletteColors( l_pal, l_entries.data(), 0, 2 );

    // Use index8 format details
    auto l_details = pixels::pixelFormatDetails( pixels::format_t::index8 );

    // Map a color exactly equal to the first palette entry. Expect index 0.
    color_t l_want0{ l_entries[ 0 ].r, l_entries[ 0 ].g, l_entries[ 0 ].b,
                     l_entries[ 0 ].a };
    auto l_idx0 = l_pal.map< uint8_t >( l_details, l_want0 );
    EXPECT_EQ( static_cast< uint8_t >( l_idx0 ), 0u );

    // Retrieve color back from palette via palette.color
    auto l_got0 =
        l_pal.color< uint8_t >( static_cast< uint8_t >( l_idx0 ), l_details );
    expectColorEq( l_want0, l_got0 );

    // Map color close to second entry -> expect index 1
    color_t l_want1{ l_entries[ 1 ].r, l_entries[ 1 ].g, l_entries[ 1 ].b,
                     l_entries[ 1 ].a };
    auto l_idx1 = l_pal.map< uint8_t >( l_details, l_want1 );
    EXPECT_EQ( static_cast< uint8_t >( l_idx1 ), 1u );
    auto l_got1 =
        l_pal.color< uint8_t >( static_cast< uint8_t >( l_idx1 ), l_details );
    expectColorEq( l_want1, l_got1 );
}
