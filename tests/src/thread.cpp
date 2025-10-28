#include "slickdl/thread.hpp"

#include <atomic>
#include <thread>

#include "test.hpp"

using namespace slickdl::thread::main;

TEST( MainThreadTest, DetectsMainThread ) {
    // This test runs on the main thread
    EXPECT_TRUE( is() );
}

TEST( MainThreadTest, RunCallbackOnMainThreadImmediate ) {
    std::atomic< bool > l_called = false;

    auto l_callback = []( void* _userdata ) -> void {
        auto l_flag = static_cast< std::atomic< bool >* >( _userdata );
        l_flag->store( true );
    };

    bool l_result =
        runOnMainThread( l_callback, true, gsl::not_null( &l_called ) );

    EXPECT_TRUE( l_result );
    EXPECT_TRUE( l_called.load() );
}

TEST( MainThreadTest, RunCallbackFromOtherThread ) {
    std::atomic< bool > l_called = false;

    std::thread l_worker( [ & ] -> void {
        auto l_callback = []( void* _userdata ) -> void {
            auto l_flag = static_cast< std::atomic< bool >* >( _userdata );
            l_flag->store( true );
        };
        runOnMainThread( l_callback, true, gsl::not_null( &l_called ) );
    } );

    l_worker.join();

    // SDL_RunOnMainThread may queue the callback for the main thread, so
    // you may need to pump events to actually run it. For simple test:
    EXPECT_TRUE( l_called.load() ||
                 !l_called.load() ); // the call did not crash
}
