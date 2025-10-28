#pragma once

#include <SDL3/SDL_timer.h>

#include <chrono>
#include <cstdint>
#include <gsl/pointers>

#include "slickdl.hpp"
#include "stddebug.hpp"

// SDL provides time management functionality. It is useful for dealing with
// (usually) small durations of time.
//
// This is not to be confused with _calendar time_ management, which is
// provided by [CategoryTime](CategoryTime).
//
// This category covers measuring time elapsed (SDL_GetTicks(),
// SDL_GetPerformanceCounter()), putting a thread to sleep for a certain
// amount of time (SDL_Delay(), SDL_DelayNS(), SDL_DelayPrecise()), and firing
// a callback function after a certain amount of time has elasped
// (SDL_AddTimer(), etc).
//
// There are also useful macros to convert between time units, like
// SDL_SECONDS_TO_NS() and such.
namespace slickdl::timer {

namespace ticks {

// Get the number that have elapsed since the SDL library initialization.
template < typename T >
[[nodiscard]] inline auto get() -> T {
    if constexpr ( std::is_same_v< T, std::chrono::milliseconds > ) {
        return ( std::chrono::milliseconds{ SDL_GetTicks() } );

    } else if constexpr ( std::is_same_v< T, std::chrono::nanoseconds > ) {
        return ( std::chrono::nanoseconds{ SDL_GetTicksNS() } );

    } else {
        // TODO: Message
        stdfunc::assert( false );

        static_assert( false );
    }
}

} // namespace ticks

namespace performance {

// Get the current value of the high resolution counter.
//
// This function is typically used for profiling.
//
// The counter values are only meaningful relative to each other. Differences
// between values can be converted to times by using
// SDL_GetPerformanceFrequency().
[[nodiscard]] inline auto count() -> uint64_t {
    return ( SDL_GetPerformanceCounter() );
}

// Get the count per second of the high resolution counter.
[[nodiscard]] inline auto frequency() -> uint64_t {
    return ( SDL_GetPerformanceFrequency() );
}

} // namespace performance

namespace delay {

// Wait a specified number of milliseconds before returning.
//
// This function waits a specified number of milliseconds before returning. It
// waits at least the specified time, but possibly longer due to OS
// scheduling.
template < typename T >
inline void normal( T _amount ) {
    if constexpr ( std::is_same_v< T, std::chrono::milliseconds > ) {
        SDL_Delay( _amount.count() );

    } else if constexpr ( std::is_same_v< T, std::chrono::nanoseconds > ) {
        SDL_DelayNS( _amount.count() );

    } else {
        // TODO: Message
        stdfunc::assert( false );

        static_assert( false );
    }
}

// Wait a specified number of nanoseconds before returning.
//
// This function waits a specified number of nanoseconds before returning. It
// will attempt to wait as close to the requested time as possible, busy
// waiting if necessary, but could return later due to OS scheduling.
inline void precise( std::chrono::nanoseconds _amount ) {
    SDL_DelayPrecise( _amount.count() );
}

} // namespace delay

// Definition of the timer ID type.
using id_t = uint32_t;

// Function prototype for the millisecond timer callback function.
//
// The callback function is passed the current timer interval and returns the
// next timer interval, in milliseconds. If the returned value is the same as
// the one passed in, the periodic alarm continues, otherwise a new alarm is
// scheduled. If the callback returns 0, the periodic alarm is canceled and
// will be removed.
//
// The new callback time interval, or 0 to disable further runs of the callback.
//
// SDL may call this callback at any time from a background thread; the
// application is responsible for locking resources the callback touches that
// need to be protected.
using callback_t = gsl::not_null< SDL_TimerCallback >;

// Call a callback function at a future time.
//
// The callback function is passed the current timer interval and the user
// supplied parameter from the SDL_AddTimer() call and should return the next
// timer interval. If the value returned from the callback is 0, the timer is
// canceled and will be removed.
//
// The callback is run on a separate thread, and for short timeouts can
// potentially be called before this function returns.
//
// Timers take into account the amount of time it took to execute the
// callback. For example, if the callback took 250 ms to execute and returned
// 1000 (ms), the timer would only wait another 750 ms before its next
// iteration.
//
// Timing may be inexact due to OS scheduling. Be sure to note the current
// time with SDL_GetTicksNS() or SDL_GetPerformanceCounter() in case your
// callback needs to adjust for variances.
[[nodiscard]] inline auto add( std::chrono::milliseconds _interval,
                               callback_t _callback,
                               void* _userData = nullptr ) -> id_t {
    const id_t l_result =
        SDL_AddTimer( _interval.count(), _callback, _userData );

    assert( l_result );

    return ( l_result );
}

// Function prototype for the nanosecond timer callback function.
//
// The callback function is passed the current timer interval and returns the
// next timer interval, in nanoseconds. If the returned value is the same as
// the one passed in, the periodic alarm continues, otherwise a new alarm is
// scheduled. If the callback returns 0, the periodic alarm is canceled and
// will be removed.
//
// The new callback time interval, or 0 to disable further runs of the callback.
//
// SDL may call this callback at any time from a background thread; the
// application is responsible for locking resources the callback touches that
// need to be protected. application is responsible for locking resources the
// callback touches that need to be protected.
using callbackNanoseconds_t = gsl::not_null< SDL_NSTimerCallback >;

// Call a callback function at a future time.
//
// The callback function is passed the current timer interval and the user
// supplied parameter from the SDL_AddTimerNS() call and should return the
// next timer interval. If the value returned from the callback is 0, the
// timer is canceled and will be removed.
//
// The callback is run on a separate thread, and for short timeouts can
// potentially be called before this function returns.
//
// Timers take into account the amount of time it took to execute the
// callback. For example, if the callback took 250 ns to execute and returned
// 1000 (ns), the timer would only wait another 750 ns before its next
// iteration.
//
// Timing may be inexact due to OS scheduling. Be sure to note the current
// time with SDL_GetTicksNS() or SDL_GetPerformanceCounter() in case your
// callback needs to adjust for variances.
[[nodiscard]] inline auto add( std::chrono::nanoseconds _interval,
                               callbackNanoseconds_t _callback,
                               void* _userData = nullptr ) -> id_t {
    const id_t l_result =
        SDL_AddTimerNS( _interval.count(), _callback, _userData );

    assert( l_result );

    return ( l_result );
}

// Remove a timer created with SDL_AddTimer().
inline void remove( id_t _id ) {
    const bool l_result = SDL_RemoveTimer( _id );

    assert( l_result );
}

} // namespace slickdl::timer
