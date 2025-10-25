#include "slickdl/error.hpp"

#include <algorithm>
#include <cctype>
#include <string>

#include "test.hpp"

using namespace slickdl;

static auto toLower( std::string_view _s ) -> std::string {
    std::string l_out( _s );
    std::ranges::transform(
        l_out, l_out.begin(), []( unsigned char _c ) -> char {
            return static_cast< char >( std::tolower( _c ) );
        } );
    return l_out;
}

TEST( ErrorTest, ClearStartsEmpty ) {
    error::clear();
    auto l_e = error::get();
    EXPECT_FALSE( l_e.has_value() );
}

TEST( ErrorTest, SetAndGetString ) {
    error::clear();
    error::set( "hello world" );
    auto l_e = error::get();
    ASSERT_TRUE( l_e.has_value() );
    EXPECT_EQ( std::string( *l_e ), "hello world" );
}

TEST( ErrorTest, SetWithFormat ) {
    error::clear();
    error::set( "value = {}", 123 );
    auto l_e = error::get();
    ASSERT_TRUE( l_e.has_value() );
    EXPECT_EQ( std::string( *l_e ), "value = 123" );
}

TEST( ErrorTest, ClearRemovesMessage ) {
    error::set( "temporary" );
    ASSERT_TRUE( error::get().has_value() );
    error::clear();
    EXPECT_FALSE( error::get().has_value() );
}

TEST( ErrorTest, UnsupportedSetsStandardMessage ) {
    error::clear();
    error::unsupported();
    auto l_e = error::get();
    ASSERT_TRUE( l_e.has_value() );
    EXPECT_EQ( std::string( *l_e ), "That operation is not supported" );
}

TEST( ErrorTest, OutOfMemorySetsNonEmptyMessage ) {
    error::clear();
    error::outOfMemory();
    auto l_e = error::get();
    ASSERT_TRUE( l_e.has_value() );
    auto l_low = toLower( *l_e );
    // SDL_OutOfMemory should set a message indicating out-of-memory.
    // Check that message is non-empty and mentions "out" and "memory"
    // (case-insensitive).
    EXPECT_FALSE( l_low.empty() );
    EXPECT_TRUE( l_low.contains( "out" ) );
    EXPECT_TRUE( l_low.contains( "memory" ) );
}
