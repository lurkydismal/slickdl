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
