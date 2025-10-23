#pragma once

#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_pen.h>
#include <SDL3/SDL_touch.h>

#include <bit>
#include <type_traits>

// SDL pen event handling.
//
// SDL provides an API for pressure-sensitive pen (stylus and/or eraser)
// handling, e.g., for input and drawing tablets or suitably equipped mobile /
// tablet devices.
//
// To get started with pens, simply handle SDL_EVENT_PEN_* events. When a pen
// starts providing input, SDL will assign it a unique SDL_PenID, which will
// remain for the life of the process, as long as the pen stays connected.
//
// Pens may provide more than simple touch input; they might have other axes,
// such as pressure, tilt, rotation, etc.
namespace slickdl::pen {

// SDL pen instance IDs.
//
// Zero is used to signify an invalid/null device.
//
// These show up in pen events when SDL sees input from them. They remain
// consistent as long as SDL can recognize a tool to be the same pen; but if a
// pen physically leaves the area and returns, it might get a new ID.
using id_t = uint32_t;

// The SDL_MouseID for mouse events simulated with pen input.
constexpr size_t g_mouseId = SDL_PEN_MOUSEID;

// The SDL_TouchID for touch events simulated with pen input.
constexpr size_t g_touchId = SDL_PEN_TOUCHID;

// Pen input flags, as reported by various pen events' `pen_state` field.
using inputFlags_t = enum class inputFlags : uint32_t {
    down = ( 1U << 0 ),       /**< pen is pressed down */
    button1 = ( 1U << 1 ),    /**< button 1 is pressed */
    button2 = ( 1U << 2 ),    /**< button 2 is pressed */
    button3 = ( 1U << 3 ),    /**< button 3 is pressed */
    button4 = ( 1U << 4 ),    /**< button 4 is pressed */
    button5 = ( 1U << 5 ),    /**< button 5 is pressed */
    eraserTip = ( 1U << 30 ), /**< eraser tip is used */
};

using inputFlagsUnderlying_t = std::underlying_type_t< inputFlags_t >;

[[nodiscard]] constexpr auto operator|( inputFlags_t _storage,
                                        inputFlags_t _value ) -> inputFlags_t {
    return ( static_cast< inputFlags_t >(
        static_cast< inputFlagsUnderlying_t >( _storage ) |
        static_cast< inputFlagsUnderlying_t >( _value ) ) );
}

[[nodiscard]] constexpr auto operator&( inputFlags_t _storage,
                                        inputFlags_t _value ) -> inputFlags_t {
    return ( static_cast< inputFlags_t >(
        static_cast< inputFlagsUnderlying_t >( _storage ) &
        static_cast< inputFlagsUnderlying_t >( _value ) ) );
}

[[nodiscard]] constexpr auto toLegacy( inputFlags_t _inputFlags )
    -> SDL_PenInputFlags {
    return ( static_cast< SDL_PenInputFlags >( _inputFlags ) );
}

[[nodiscard]] constexpr auto toLegacy( inputFlags_t* _inputFlags )
    -> SDL_PenInputFlags* {
    return ( std::bit_cast< SDL_PenInputFlags* >( _inputFlags ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_PenInputFlags _inputFlags )
    -> inputFlags_t {
    return ( static_cast< inputFlags_t >( _inputFlags ) );
}

// Pen axis indices.
//
// These are the valid values for the `axis` field in SDL_PenAxisEvent. All
// axes are either normalised to 0..1 or report a (positive or negative)
// angle in degrees, with 0.0 representing the centre. Not all pens/backends
// support all axes: unsupported axes are always zero.
//
// To convert angles for tilt and rotation into vector representation, use
// SDL_sinf on the XTILT, YTILT, or ROTATION component, for example:
//
// `SDL_sinf(xtilt * SDL_PI_F / 180.0)`.
using axis_t = enum class axis : uint8_t {
    pressure,           /**< Pen pressure.  Unidirectional: 0 to 1.0 */
    tiltX,              /**< Pen horizontal tilt angle.  Bidirectional:
                           -90.0    to 90.0 (left-to-right). */
    tiltY,              /**< Pen vertical tilt angle.  Bidirectional: -90.0
                           to 90.0 (top-to-down). */
    distance,           /**< Pen distance to drawing surface.
                           Unidirectional: 0.0 to 1.0 */
    rotation,           /**< Pen barrel rotation.  Bidirectional: -180 to
                           179.9 (clockwise, 0 is facing up, -180.0 is
                           facing down). */
    slider,             /**< Pen finger wheel or slider (e.g., Airbrush
                           Pen).   Unidirectional: 0 to 1.0 */
    tangentialPressure, /**< Pressure from squeezing the pen
                            ("barrel pressure"). */
    count               /**< Total known pen axis types in this version of
                                        SDL. This number may grow in future releases! */
};

using axisUnderlying_t = std::underlying_type_t< axis_t >;

[[nodiscard]] constexpr auto toLegacy( axis_t _axis ) -> SDL_PenAxis {
    return ( static_cast< SDL_PenAxis >( _axis ) );
}

[[nodiscard]] constexpr auto toLegacy( axis_t* _axis ) -> SDL_PenAxis* {
    return ( std::bit_cast< SDL_PenAxis* >( _axis ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_PenAxis _axis ) -> axis_t {
    return ( static_cast< axis_t >( _axis ) );
}

} // namespace slickdl::pen
