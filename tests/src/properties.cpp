#include "slickdl/properties.hpp"

#include <numbers>
#include <string>
#include <vector>

#include "test.hpp"

using namespace slickdl::properties;

static void enumerateCollector( void* _userdata,
                                id_t /*props*/,
                                const char* _name ) {
    auto l_vec = static_cast< std::vector< std::string >* >( _userdata );
    l_vec->emplace_back( _name );
}

TEST( PropertiesTest, CreateSetGetString ) {
    auto l_id = create();
    ASSERT_TRUE( l_id );

    set< const char* >( l_id, "my.string", "hello" );
    auto l_v = get< const char* >( l_id, "my.string" );
    ASSERT_TRUE( l_v.has_value() );
    EXPECT_STREQ( l_v.value(), "hello" );
    EXPECT_TRUE( exists( l_id, "my.string" ) );
    EXPECT_EQ( type( l_id, "my.string" ), type_t::string );

    clear( l_id, "my.string" );
    EXPECT_FALSE( exists( l_id, "my.string" ) );

    destroy( l_id );
}

TEST( PropertiesTest, SetGetNumberAndFloatAndBool ) {
    auto l_id = create();

    set< int64_t >( l_id, "num", int64_t{ 1234567890123 } );
    set< float >( l_id, "flt", std::numbers::pi );
    set< bool >( l_id, "b", true );

    auto l_n = get< int64_t >( l_id, "num" );
    ASSERT_TRUE( l_n.has_value() );
    EXPECT_EQ( l_n.value(), int64_t{ 1234567890123 } );

    auto l_f = get< float >( l_id, "flt" );
    ASSERT_TRUE( l_f.has_value() );
    EXPECT_NEAR( l_f.value(), std::numbers::pi,
                 std::numeric_limits< float >::epsilon() );

    auto l_b = get< bool >( l_id, "b" );
    ASSERT_TRUE( l_b.has_value() );
    EXPECT_TRUE( l_b.value() );

    EXPECT_EQ( type( l_id, "num" ), type_t::number );
    EXPECT_EQ( type( l_id, "flt" ), type_t::floatingPoint );
    EXPECT_EQ( type( l_id, "b" ), type_t::boolean );

    destroy( l_id );
}

TEST( PropertiesTest, PointerProperty ) {
    auto l_id = create();

    int l_payload = 0x1234;
    void* l_p = &l_payload;
    set< void* >( l_id, "ptr", l_p );

    auto l_got = get< void* >( l_id, "ptr" );
    ASSERT_TRUE( l_got.has_value() );
    EXPECT_EQ( l_got.value(), l_p );
    EXPECT_EQ( type( l_id, "ptr" ), type_t::pointer );

    destroy( l_id );
}

TEST( PropertiesTest, CopyAndEnumerate ) {
    auto l_src = create();
    auto l_dst = create();

    set< const char* >( l_src, "a", "one" );
    set< int64_t >( l_src, "b", 2 );
    set< bool >( l_src, "c", false );

    // copy properties from src -> dst
    copy( l_src, l_dst );

    // dst should have them
    EXPECT_TRUE( exists( l_dst, "a" ) );
    EXPECT_TRUE( exists( l_dst, "b" ) );
    EXPECT_TRUE( exists( l_dst, "c" ) );

    auto l_sa = get< const char* >( l_dst, "a" );
    ASSERT_TRUE( l_sa.has_value() );
    EXPECT_STREQ( l_sa.value(), "one" );

    auto l_sb = get< int64_t >( l_dst, "b" );
    ASSERT_TRUE( l_sb.has_value() );
    EXPECT_EQ( l_sb.value(), 2 );

    std::vector< std::string > l_names;
    enumerate( l_dst, enumerateCollector, &l_names );
    std::ranges::sort( l_names );
    EXPECT_TRUE( std::ranges::binary_search( l_names, "a" ) );
    EXPECT_TRUE( std::ranges::binary_search( l_names, "b" ) );
    EXPECT_TRUE( std::ranges::binary_search( l_names, "c" ) );

    destroy( l_src );
    destroy( l_dst );
}

TEST( PropertiesTest, LockUnlockNoCrash ) {
    auto l_id = create();
    // lock/unlock should succeed (asserts inside wrapper will fail test if not)
    lock( l_id );
    unlock( l_id );
    destroy( l_id );
}
