#include "slickdl/line_box.hpp"

#include <vector>

#include "test.hpp"

using namespace slickdl;

TEST( BoxBasic, EmptyAndContainsInt ) {
    box_t< int > l_b{ 0, 0, 10, 20 };

    EXPECT_FALSE( l_b.empty() );
    EXPECT_TRUE( l_b.contains( point_t< int >{ 0, 0 } ) );
    EXPECT_TRUE( l_b.contains( point_t< int >{ 9, 19 } ) );
    EXPECT_TRUE(
        l_b.contains( point_t< int >{ 10, 0 } ) ); // x == x+width is outside
    EXPECT_TRUE(
        l_b.contains( point_t< int >{ 0, 20 } ) ); // y == y+height is outside

    EXPECT_DEATH( box_t< int >( 0, 0, 0, 10 ), ".*" );
    EXPECT_DEATH( box_t< int >( 0, 0, 10, 0 ), ".*" );
}

TEST( BoxBasic, EmptyAndContainsFloat ) {
    box_t< float > l_b{ 0.0f, 0.0f, 1.5f, 2.5f };

    EXPECT_FALSE( l_b.empty() );
    EXPECT_TRUE( l_b.contains( point_t< float >{ 0.0f, 0.0f } ) );
    EXPECT_TRUE( l_b.contains( point_t< float >{ 1.4f, 2.4f } ) );
    EXPECT_TRUE( l_b.contains( point_t< float >{ 1.5f, 0.0f } ) );
    EXPECT_TRUE( l_b.contains( point_t< float >{ 0.0f, 2.5f } ) );

    EXPECT_DEATH( box_t< float >( 0, 0, 0, 10 ), ".*" );
    EXPECT_DEATH( box_t< float >( 0, 0, 10, 0 ), ".*" );
}

TEST( BoxBasic, IntersectsAndTouch ) {
    box_t< int > l_a{ 0, 0, 10, 10 };
    box_t< int > l_b{ 5, 5, 10, 10 }; // overlap
    box_t< int > l_c{ 10, 0, 5,
                      5 }; // touching on right edge -> no intersection
    box_t< int > l_d{ -10, -10, 5, 5 }; // disjoint

    EXPECT_TRUE( l_a.intersects( l_b ) );
    EXPECT_FALSE( l_a.intersects(
        l_c ) ); // touching edges should not count as intersection
    EXPECT_FALSE( l_a.intersects( l_d ) );
}

TEST( BoxCompositeIntersection, CompositeAndIntersection ) {
    box_t< int > l_a{ 0, 0, 10, 10 };
    box_t< int > l_b{ 5, 5, 10, 10 };

    auto l_comp = l_a.composite( l_b );
    ASSERT_TRUE( l_comp.has_value() );
    EXPECT_EQ( l_comp->x, 0 );
    EXPECT_EQ( l_comp->y, 0 );
    EXPECT_EQ( l_comp->width, 15 );
    EXPECT_EQ( l_comp->height, 15 );

    auto l_inter = l_a.intersection( l_b );
    ASSERT_TRUE( l_inter.has_value() );
    EXPECT_EQ( l_inter->x, 5 );
    EXPECT_EQ( l_inter->y, 5 );
    EXPECT_EQ( l_inter->width, 5 );
    EXPECT_EQ( l_inter->height, 5 );

    // intersection with disjoint -> nullopt
    line_t< int > l_d{
        { 20, 20 },
        { 2, 2 },
    };
    auto l_nointer = l_a.intersection( l_d );
    EXPECT_TRUE( l_nointer.has_value() );
}

//
// Enclosing / encloses (points -> minimal box)
//

TEST( Enclosing, PointsNoClip ) {
    std::vector< point_t< int > > l_pts = {
        { 2, 3 }, { 5, 1 }, { 4, 7 }, { 2, 1 } };
    box_t< int > l_drawer; // value not used for this overload
    auto l_encl = l_drawer.enclosing( std::span( l_pts ) );
    ASSERT_TRUE( l_encl.has_value() );
    EXPECT_EQ( l_encl->x, 2 );
    EXPECT_EQ( l_encl->y, 1 );
    EXPECT_EQ( l_encl->width, 5 - 2 );  // maxX - minX => 5-2
    EXPECT_EQ( l_encl->height, 7 - 1 ); // 7-1
}

TEST( Enclosing, PointsWithClip ) {
    std::vector< point_t< int > > l_pts = {
        { 2, 3 }, { 5, 1 }, { 4, 7 }, { 2, 1 }, { 100, 100 } };
    box_t< int > l_clipBox{ 0, 0, 10, 10 };
    auto l_encl = l_clipBox.enclosing( std::span( l_pts ), l_clipBox );
    ASSERT_TRUE( l_encl.has_value() );
    EXPECT_EQ( l_encl->x, 2 );
    EXPECT_EQ( l_encl->y, 1 );
    EXPECT_EQ( l_encl->width, 5 - 2 );
    EXPECT_EQ( l_encl->height, 7 - 1 );
}

TEST( Enclosing, EmptyPoints ) {
    std::vector< point_t< int > > l_pts;
    box_t< int > l_b;
    auto l_encl = l_b.enclosing( std::span( l_pts ) );
    EXPECT_FALSE( l_encl.has_value() );
}

//
// Line vs Box clipping (Cohen-Sutherland behaviors)
//

TEST( LineIntersection, HorizontalLineClipped ) {
    box_t< int > l_b{ 0, 0, 10, 10 };
    line_t< int > l_l{ { -5, 5 }, { 15, 5 } }; // horizontal across center

    auto l_r = l_b.intersection( l_l );
    ASSERT_TRUE( l_r.has_value() );
    // clipped to left/right edges
    EXPECT_EQ( l_r->start.x, 0 );
    EXPECT_EQ( l_r->start.y, 5 );
    EXPECT_EQ( l_r->end.x, 10 );
    EXPECT_EQ( l_r->end.y, 5 );
}

TEST( LineIntersection, VerticalLineClipped ) {
    box_t< int > l_b{ 0, 0, 10, 10 };
    line_t< int > l_l{ { 5, -10 }, { 5, 20 } }; // vertical across center

    auto l_r = l_b.intersection( l_l );
    ASSERT_TRUE( l_r.has_value() );
    EXPECT_EQ( l_r->start.x, 5 );
    EXPECT_EQ( l_r->start.y, 0 );
    EXPECT_EQ( l_r->end.x, 5 );
    EXPECT_EQ( l_r->end.y, 10 );
}

TEST( LineIntersection, LineCompletelyOutside ) {
    box_t< int > l_b{ 0, 0, 10, 10 };
    line_t< int > l_l{ { -20, -20 }, { -15, -10 } }; // entirely left/top

    auto l_r = l_b.intersection( l_l );
    EXPECT_FALSE( l_r.has_value() );
}

TEST( LineIntersection, LineCompletelyInside ) {
    box_t< int > l_b{ 0, 0, 10, 10 };
    line_t< int > l_l{ { 1, 1 }, { 2, 2 } }; // entirely inside
    auto l_r = l_b.intersection( l_l );
    ASSERT_TRUE( l_r.has_value() );
    // Expect same extremes (order may be preserved). Compare unordered
    // endpoints.
    EXPECT_EQ( l_r->start.x, l_l.start.x );
    EXPECT_EQ( l_r->start.y, l_l.start.y );
    EXPECT_EQ( l_r->end.x, l_l.end.x );
    EXPECT_EQ( l_r->end.y, l_l.end.y );
}

//
// small API sanity checks
//

TEST( PointsAndLines, BoxPointsArray ) {
    box_t< int > l_b{ 1, 2, 3, 4 };
    auto l_pts = l_b.points();
    EXPECT_EQ( l_pts.size(), box< int >::g_pointsAmount );
    EXPECT_EQ( l_pts[ 0 ].x, 1 );
    EXPECT_EQ( l_pts[ 0 ].y, 2 );
    EXPECT_EQ( l_pts[ 1 ].x, 4 );
    EXPECT_EQ( l_pts[ 1 ].y, 6 );
}

TEST( PointsAndLines, LinePointsArray ) {
    line_t< int > l_l{ { 1, 2 }, { 3, 4 } };
    auto l_pts = l_l.points();
    EXPECT_EQ( l_pts.size(), line< int >::g_pointsAmount );
    EXPECT_EQ( l_pts[ 0 ].x, 1 );
    EXPECT_EQ( l_pts[ 0 ].y, 2 );
    EXPECT_EQ( l_pts[ 1 ].x, 3 );
    EXPECT_EQ( l_pts[ 1 ].y, 4 );
}
