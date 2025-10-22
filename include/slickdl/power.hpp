#pragma once

#include <SDL3/SDL_power.h>

#include <bit>
#include <optional>
#include <type_traits>
#include <utility>

// SDL power management routines.
//
// There is a single function in this category: SDL_GetPowerInfo().
//
// This function is useful for games on the go. This allows an app to know if
// it's running on a draining battery, which can be useful if the app wants to
// reduce processing, or perhaps framerate, to extend the duration of the
// battery's charge. Perhaps the app just wants to show a battery meter when
// fullscreen, or alert the user when the power is getting extremely low, so
// they can save their game.
namespace slickdl::power {

// The basic state for the system's power supply.
using state_t = enum class state : int8_t {
    error = -1, /**< error determining power status */
    unknown,    /**< cannot determine power status */
    onBattery,  /**< Not plugged in, running on the battery */
    noBattery,  /**< Plugged in, no battery available */
    charging,   /**< Plugged in, charging battery */
    charged,    /**< Plugged in, battery charged */
};

using stateUnderlying_t = std::underlying_type_t< state_t >;

[[nodiscard]] constexpr auto toLegacy( state_t _state ) -> SDL_PowerState {
    return ( static_cast< SDL_PowerState >( _state ) );
}

[[nodiscard]] constexpr auto toLegacy( state_t* _state ) -> SDL_PowerState* {
    return ( std::bit_cast< SDL_PowerState* >( _state ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_PowerState _state ) -> state_t {
    return ( static_cast< state_t >( _state ) );
}

// Get the current power supply details.
//
// You should never take a battery status as absolute truth. Batteries
// (especially failing batteries) are delicate hardware, and the values
// reported here are best estimates based on what that hardware reports. It's
// not uncommon for older batteries to lose stored power much faster than it
// reports, or completely drain when reporting it has 20 percent left, etc.
//
// Battery status can change at any time; if you are concerned with power
// state, you should call this function frequently, and perhaps ignore changes
// until they seem to be stable for a few seconds.
//
// It's possible a platform can only report battery percentage or time left
// but not both.
//
// On some platforms, retrieving power supply details might be expensive. If
// you want to display continuous status you could call this function every
// minute or so.
//
// Seconds filled in with the seconds of battery life left, or NULL to ignore.
// This will be filled in with -1 if we can't determine a value or there is no
// battery.
//
// Percent filled in wth the percentage of battery life left, between 0 and
// 100, or NULL to ignore. This will be filled in with -1 we can't determine a
// value or there is no battery.
[[nodiscard]] auto info()
    -> std::pair< state_t, std::optional< std::pair< size_t, size_t > > >;

} // namespace slickdl::power
