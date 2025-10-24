#pragma once

#include <SDL3/SDL_touch.h>

#include <cstddef>
#include <gsl/pointers>
#include <type_traits>

// SDL offers touch input, on platforms that support it. It can manage
// multiple touch devices and track multiple fingers on those devices.
//
// Touches are mostly dealt with through the event system, in the
// SDL_EVENT_FINGER_DOWN, SDL_EVENT_FINGER_MOTION, and SDL_EVENT_FINGER_UP
// events, but there are also functions to query for hardware details, etc.
//
// The touch system, by default, will also send virtual mouse events; this can
// be useful for making a some desktop apps work on a phone without
// significant changes. For apps that care about mouse and touch input
// separately, they should ignore mouse events that have a `which` field of
// SDL_TOUCH_MOUSEID.
namespace slickdl::touch {

// A unique ID for a touch device.
//
// This ID is valid for the time the device is connected to the system, and is
// never reused for the lifetime of the application.
//
// The value 0 is an invalid ID.
using id_t = uint64_t;

// An enum that describes the type of a touch device.
using deviceType_t = enum class deviceType : int8_t {
    invalid = -1,
    direct,           /**< touch screen with window-relative coordinates */
    indirectAbsolute, /**< trackpad with absolute device coordinates */
    indirectRelative, /**< trackpad with screen cursor-relative coordinates */
};

using deviceTypeUnderlying_t = std::underlying_type_t< deviceType_t >;

[[nodiscard]] constexpr auto toLegacy( deviceType_t _value )
    -> SDL_TouchDeviceType {
    return ( static_cast< SDL_TouchDeviceType >( _value ) );
}

[[nodiscard]] constexpr auto toLegacy( deviceType_t* _value )
    -> SDL_TouchDeviceType* {
    return ( std::bit_cast< SDL_TouchDeviceType* >( _value ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_TouchDeviceType _value )
    -> deviceType_t {
    return ( static_cast< deviceType_t >( _value ) );
}

// The SDL_MouseID for mouse events simulated with touch input.
constexpr size_t g_mouseId = SDL_TOUCH_MOUSEID;

// The SDL_TouchID for touch events simulated with mouse input.
constexpr size_t g_touchId = SDL_MOUSE_TOUCHID;

// Get a list of registered touch devices.
//
// On some platforms SDL first sees the touch device if it was actually used.
// Therefore the returned list might be empty, although devices are available.
// After using all devices at least once the number will be correct.
[[nodiscard]] auto all() -> std::vector< id_t >;

// Get the touch device name as reported from the driver.
[[nodiscard]] inline auto name( id_t _id ) -> std::string_view {
    return { gsl::make_not_null( SDL_GetTouchDeviceName( _id ) ) };
}

// Get the type of the given touch device.
[[nodiscard]] inline auto type( id_t _id ) -> deviceType_t {
    return ( fromLegacy( SDL_GetTouchDeviceType( _id ) ) );
}

namespace finger {

// A unique ID for a single finger on a touch device.
//
// This ID is valid for the time the finger (stylus, etc) is touching and will
// be unique for all fingers currently in contact, so this ID tracks the
// lifetime of a single continuous touch. This value may represent an index, a
// pointer, or some other unique ID, depending on the platform.
//
// The value 0 is an invalid ID.
using id_t = uint64_t;

// Data about a single finger in a multitouch event.
//
// Each touch event is a collection of fingers that are simultaneously in
// contact with the touch device (so a "touch" can be a "multitouch," in
// reality), and this struct reports details of the specific fingers.
using finger_t = gsl::not_null< SDL_Finger* >;

// Get a list of active fingers for a given touch device.
[[nodiscard]] auto all( id_t _id ) -> std::vector< finger_t >;

} // namespace finger

} // namespace slickdl::touch
