#pragma once

#include <SDL3/SDL_error.h>

#include "stdconcepts.hpp"

namespace slickdl::error {

// Set the SDL error message for the current thread.
//
// Calling this function will replace any previous error message that was set.
inline void set( std::string_view _message ) {
    SDL_SetError( "%s", std::string( _message ).c_str() );
}

// Set the SDL error message for the current thread.
//
// Calling this function will replace any previous error message that was set.
template < stdfunc::is_formattable... Arguments >
inline void set( std::format_string< Arguments... > _format,
                 Arguments&&... _arguments ) {
    const std::string l_message =
        std::format( _format, std::forward< Arguments >( _arguments )... );

    SDL_SetError( "%s", l_message.c_str() );
}

// Set an error indicating that memory allocation failed.
//
// This function does not do any memory allocation.
inline void outOfMemory() {
    SDL_OutOfMemory();
}

// Retrieve a message about the last error that occurred on the current
// thread.
//
// It is possible for multiple errors to occur before calling SDL_GetError().
// Only the last error is returned.
//
// The message is only applicable when an SDL function has signaled an error.
// You must check the return values of SDL function calls to determine when to
// appropriately call SDL_GetError(). You should *not* use the results of
// SDL_GetError() to decide if an error has occurred! Sometimes SDL will set
// an error string even when reporting success.
//
// SDL will *not* clear the error string for successful API calls. You *must*
// check return values for failure cases before you can assume the error
// string applies.
//
// Error strings are set per-thread, so an error set in a different thread
// will not interfere with the current thread's operation.
//
// The returned value is a thread-local string which will remain valid until
// the current thread's error string is changed. The caller should make a copy
// if the value is needed after the next SDL API call.
[[nodiscard]] inline auto get() -> std::optional< std::string_view > {
    const std::string_view l_result = SDL_GetError();

    if ( l_result.empty() ) {
        return ( std::nullopt );

    } else {
        return ( l_result );
    }
}

// Clear any previous error message for this thread.
inline void clear() {
    SDL_ClearError();
}

// This simply calls SDL_SetError() with a standardized error string, for
// convenience, consistency, and clarity.
inline void unsupported() {
    set( "That operation is not supported" );
}

} // namespace slickdl::error
