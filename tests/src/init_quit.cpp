#include "slickdl/init_quit.hpp"

#include "test.hpp"

using namespace slickdl;

TEST( SDLInitWrapperTest, InitAndQuitAll ) {
    // Initialize all video and events subsystems
    init::all( init::flag_t::video | init::flag_t::events );

    auto l_flags = init::currentFlags();
    EXPECT_TRUE( static_cast< uint32_t >( l_flags ) &
                 static_cast< uint32_t >( init::flag_t::video ) );
    EXPECT_TRUE( static_cast< uint32_t >( l_flags ) &
                 static_cast< uint32_t >( init::flag_t::events ) );

    quit::all();

    // After quit, no subsystems should remain initialized
    EXPECT_EQ( SDL_WasInit( 0 ), 0 );
}

TEST( SDLInitWrapperTest, InitSubsystemIndividually ) {
    init::subSystem( init::flag_t::audio );

    auto l_flags = init::currentFlags( init::flag_t::audio );
    EXPECT_TRUE( static_cast< uint32_t >( l_flags ) &
                 static_cast< uint32_t >( init::flag_t::audio ) );

    quit::subSystem( init::flag_t::audio );
    EXPECT_EQ( SDL_WasInit( SDL_INIT_AUDIO ), 0 );
}

TEST( SDLInitWrapperTest, FlagConversion ) {
    init::flag_t l_f = init::flag_t::video;
    SDL_InitFlags l_legacy = init::toLegacy( l_f );
    EXPECT_EQ( l_legacy, SDL_INIT_VIDEO );

    init::flag_t l_back = init::fromLegacy( l_legacy );
    EXPECT_EQ( l_back, l_f );
}

TEST( SDLInitWrapperTest, RefCountBehavior ) {
    // Initialize video subsystem twice
    init::subSystem( init::flag_t::video );
    init::subSystem( init::flag_t::video );

    // Quit once, should still remain initialized
    quit::subSystem( init::flag_t::video );
    EXPECT_TRUE( SDL_WasInit( SDL_INIT_VIDEO ) );

    // Quit again, should be fully uninitialized
    quit::subSystem( init::flag_t::video );
    EXPECT_EQ( SDL_WasInit( SDL_INIT_VIDEO ), 0 );
}
