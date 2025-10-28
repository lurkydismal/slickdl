#pragma once

#include <SDL3/SDL_init.h>

#include <gsl/pointers>

namespace slickdl::thread::main {

// Whether this is the main thread
//
// On Apple platforms, the main thread is the thread that runs your program's
// main() entry point. On other platforms, the main thread is the one that
// calls SDL_Init(SDL_INIT_VIDEO), which should usually be the one that runs
// your program's main() entry point. If you are using the main callbacks,
// SDL_AppInit(), SDL_AppIterate(), and SDL_AppQuit() are all called on the
// main thread
inline auto is() -> bool {
    return ( SDL_IsMainThread() );
}

using callback_t = gsl::not_null< SDL_MainThreadCallback >;

// Call a function on the main thread during event processing
//
// If this is called on the main thread, the callback is executed immediately
// If this is called on another thread, this callback is queued for execution
// on the main thread during event processing
inline auto runOnMainThread( callback_t _callback,
                             bool _wait,
                             void* _userdata = nullptr ) -> bool {
    return ( SDL_RunOnMainThread( _callback, _userdata, _wait ) );
}

} // namespace slickdl::thread::main
