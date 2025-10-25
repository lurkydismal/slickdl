#include "slickdl/point.hpp"

#include "test.hpp"

using namespace slickdl;

TEST( PointTest, FromSDLPointAndBack ) {
    const SDL_Point l_s{ 10, 20 };
    point_t< int > l_p( l_s );
    auto l_s2 = static_cast< SDL_Point >( l_p );
    EXPECT_EQ( l_s2.x, 10 );
    EXPECT_EQ( l_s2.y, 20 );
}

TEST( PointTest, FromSDLFPointAndBack ) {
    SDL_FPoint l_s{ 1.5f, -2.25f };
    point_t< float > l_p( l_s );
    auto l_s2 = static_cast< SDL_FPoint >( l_p );
    EXPECT_FLOAT_EQ( l_s2.x, 1.5f );
    EXPECT_FLOAT_EQ( l_s2.y, -2.25f );
}

TEST( PointTest, ComparisonAndThreeWay ) {
    point_t< int > l_a{ 1, 2 };
    point_t< int > l_b{ 1, 2 };
    point_t< int > l_c{ 2, 3 };

    EXPECT_TRUE( l_a == l_b );
    EXPECT_FALSE( l_a != l_b );
    EXPECT_TRUE( l_a < l_c );
    EXPECT_TRUE( l_c > l_a );
}

TEST( PointTest, ExplicitConversionBetweenTypes ) {
    point_t< int > l_pi{ 7, 8 };
    auto l_pf = static_cast< point_t< float > >( l_pi );
    EXPECT_FLOAT_EQ( static_cast< float >( l_pi.x ), l_pf.x );
    EXPECT_FLOAT_EQ( static_cast< float >( l_pi.y ), l_pf.y );
}

template < typename T >
struct poly {
    static constexpr size_t g_pointsAmount = 2;
    std::vector< point_t< T > > pts;
    [[nodiscard]] auto points() const& -> const std::vector< point_t< T > >& {
        return pts;
    }
};

TEST( PointTest, ToPointsFlattensContainer ) {
    poly< int > l_a;
    l_a.pts.emplace_back( 1, 2 );
    l_a.pts.emplace_back( 3, 4 );

    poly< int > l_b;
    l_b.pts.emplace_back( 5, 6 );

    std::vector< poly< int > > l_polys;
    l_polys.push_back( l_a );
    l_polys.push_back( l_b );

    auto l_flattened = toPoints< int, poly >( std::span{ l_polys } );
    ASSERT_EQ( l_flattened.size(), 3u );

    EXPECT_EQ( l_flattened[ 0 ].x, 1 );
    EXPECT_EQ( l_flattened[ 0 ].y, 2 );

    EXPECT_EQ( l_flattened[ 1 ].x, 3 );
    EXPECT_EQ( l_flattened[ 1 ].y, 4 );

    EXPECT_EQ( l_flattened[ 2 ].x, 5 );
    EXPECT_EQ( l_flattened[ 2 ].y, 6 );
}
