#include "slickdl/clipping_zone.hpp"

#include "test.hpp"

using namespace slickdl;

TEST( ClippingZoneTest, FromSDLRectAndBack ) {
    SDL_Rect l_r{ 10, 20, 30, 40 };    // x, y, w, h
    clippingZone_t< int > l_cz( l_r ); // ctor takes non-const ref
    EXPECT_EQ( l_cz.minX, 10 );
    EXPECT_EQ( l_cz.minY, 20 );
    EXPECT_EQ( l_cz.maxX, 40 ); // 10 + 30
    EXPECT_EQ( l_cz.maxY, 60 ); // 20 + 40

    auto l_r2 = static_cast< SDL_Rect >( l_cz );
    EXPECT_EQ( l_r2.x, 10 );
    EXPECT_EQ( l_r2.y, 20 );
    EXPECT_EQ( l_r2.w, 30 );
    EXPECT_EQ( l_r2.h, 40 );
}

TEST( ClippingZoneTest, FromSDLFRectAndBack ) {
    SDL_FRect l_r{ 1.5f, -2.25f, 3.25f, 4.5f };
    clippingZone_t< float > l_cz( l_r );
    EXPECT_FLOAT_EQ( l_cz.minX, 1.5f );
    EXPECT_FLOAT_EQ( l_cz.minY, -2.25f );
    EXPECT_FLOAT_EQ( l_cz.maxX, 1.5f + 3.25f );
    EXPECT_FLOAT_EQ( l_cz.maxY, -2.25f + 4.5f );

    auto l_r2 = static_cast< SDL_FRect >( l_cz );
    EXPECT_FLOAT_EQ( l_r2.x, 1.5f );
    EXPECT_FLOAT_EQ( l_r2.y, -2.25f );
    EXPECT_FLOAT_EQ( l_r2.w, 3.25f );
    EXPECT_FLOAT_EQ( l_r2.h, 4.5f );
}

TEST( ClippingZoneTest, EmptyDetection ) {
    clippingZone_t< int > l_cz;
    EXPECT_TRUE( l_cz.empty() );

    clippingZone_t< int > l_nonEmpty( 0, 0, 1, 1 );
    EXPECT_FALSE( l_nonEmpty.empty() );
}

TEST( ClippingZoneTest, AssignmentFromNative ) {
    SDL_Rect l_r{ 2, 4, 6, 8 };
    clippingZone_t< int > l_cz;
    l_cz = l_r;
    EXPECT_EQ( l_cz.minX, 2 );
    EXPECT_EQ( l_cz.minY, 4 );
    EXPECT_EQ( l_cz.maxX, 8 );  // 2 + 6
    EXPECT_EQ( l_cz.maxY, 12 ); // 4 + 8
}

TEST( ClippingZoneTest, ComparisonAndThreeWay ) {
    clippingZone_t< int > l_a{ 0, 0, 10, 10 };
    clippingZone_t< int > l_b{ 0, 0, 10, 10 };
    clippingZone_t< int > l_c{ 1, 1, 11, 11 };

    EXPECT_TRUE( l_a == l_b );
    EXPECT_FALSE( l_a != l_b );
    EXPECT_TRUE( l_a < l_c );
    EXPECT_TRUE( l_c > l_a );
}

TEST( ClippingZoneTest, ExplicitConversionBetweenTypes ) {
    clippingZone_t< int > l_ci{ 1, 2, 5, 6 };
    auto l_cf = static_cast< clippingZone_t< float > >( l_ci );
    EXPECT_FLOAT_EQ( static_cast< float >( l_ci.minX ), l_cf.minX );
    EXPECT_FLOAT_EQ( static_cast< float >( l_ci.minY ), l_cf.minY );
    EXPECT_FLOAT_EQ( static_cast< float >( l_ci.maxX ), l_cf.maxX );
    EXPECT_FLOAT_EQ( static_cast< float >( l_ci.maxY ), l_cf.maxY );
}

TEST( ClippingZoneTest, PointsReturnCorners ) {
    // Expect four corners in this order: top-left, top-right, bottom-right,
    // bottom-left
    clippingZone_t< int > l_cz{ 10, 20, 30, 40 }; // minX, minY, maxX, maxY
    auto l_pts = toPoints( l_cz );
    ASSERT_EQ( l_pts.size(), 2u );

    EXPECT_EQ( l_pts[ 0 ].x, 10 ); // Min
    EXPECT_EQ( l_pts[ 0 ].y, 20 );

    EXPECT_EQ( l_pts[ 1 ].x, 30 ); // Max
    EXPECT_EQ( l_pts[ 1 ].y, 40 );
}
