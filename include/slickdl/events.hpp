#pragma once

#include <SDL3/SDL_events.h>

#include <bit>
#include <chrono>
#include <gsl/pointers>
#include <type_traits>

#include "slickdl.hpp"
#include "slickdl/video.hpp"

// Event queue management.
//
// It's extremely common--often required--that an app deal with SDL's event
// queue. Almost all useful information about interactions with the real world
// flow through here: the user interacting with the computer and app, hardware
// coming and going, the system changing in some way, etc.
//
// An app generally takes a moment, perhaps at the start of a new frame, to
// examine any events that have occured since the last time and process or
// ignore them. This is generally done by calling SDL_PollEvent() in a loop
// until it returns false (or, if using the main callbacks, events are
// provided one at a time in calls to SDL_AppEvent() before the next call to
// SDL_AppIterate(); in this scenario, the app does not call SDL_PollEvent()
// at all).
//
// There is other forms of control, too: SDL_PeepEvents() has more
// functionality at the cost of more complexity, and SDL_WaitEvent() can block
// the process until something interesting happens, which might be beneficial
// for certain types of programs on low-power hardware. One may also call
// SDL_AddEventWatch() to set a callback when new events arrive.
//
// The app is free to generate their own events, too: SDL_PushEvent allows the
// app to put events onto the queue for later retrieval; SDL_RegisterEvents
// can guarantee that these events have a type that isn't in use by other
// parts of the system.
namespace slickdl::events {

// The types of events that can be delivered.
using type_t = enum class type : uint32_t {
    first = 0, /**< Unused (do not remove) */

    /* Application events */
    quit = 0x100, /**< User-requested quit */

    /* These application events have special meaning on iOS and Android, see
       README-ios.md and README-android.md for details */
    terminating,         /**< The application is being terminated by the OS.
                                      This event must be handled in a callback set with
                                      SDL_AddEventWatch(). Called on iOS in
                                      applicationWillTerminate() Called on Android in
                                      onDestroy()
                                   */
    lowMemory,           /**< The application is low on memory, free memory if
                                       possible. This event must be handled in a callback
                                       set with SDL_AddEventWatch().  Called on iOS in
                                       applicationDidReceiveMemoryWarning()  Called on
                                       Android in onTrimMemory()
                                    */
    willEnterBackground, /**< The application is about to enter the
                                     background. This event must be handled in a
                                     callback set with SDL_AddEventWatch().
                                     Called on iOS in
                                     applicationWillResignActive() Called on
                                     Android in onPause()
                                */
    didEnterBackground,  /**< The application did enter the
                                      background and may not get CPU for some
                                      time. This event must be handled in a
                                      callback set with SDL_AddEventWatch().
                                      Called on iOS in
                                      applicationDidEnterBackground()  Called on
                                      Android in onPause()
                                 */
    willEnterForeground, /**< The application is about to enter the
                                     foreground. This event must be handled in a
                                     callback set with SDL_AddEventWatch().
                                     Called on iOS in
                                     applicationWillEnterForeground() Called on
                                     Android in onResume()
                                */
    didEnterForeground,  /**< The application is now interactive.
                                      This event must be handled in a callback
                                      set with SDL_AddEventWatch().  Called on
                            iOS  in applicationDidBecomeActive()  Called on
                                      Android in onResume()
                                 */

    localeChanged, /**< The user's locale preferences have changed.
                    */

    systemThemeChanged, /**< The system theme changed */

    /* Display events */
    /* 0x150 was SDL_DISPLAYEVENT, reserve the number for sdl2-compat */
    displayOrientation = 0x151, /**< Display orientation has changed to data1 */
    displayAdded,               /**< Display has been added to the system */
    displayRemoved,             /**< Display has been removed from the system */
    displayMoved,               /**< Display has changed position */
    displayDesktopModeChanged,  /**< Display has changed desktop
                                                mode */
    displayCurrentModeChanged,  /**< Display has changed current
                                                mode */
    displayContentScaleChanged, /**< Display has changed content
                                                scale */
    displayFirst = displayOrientation,
    displayLast = displayContentScaleChanged,

    /* Window events */
    /* 0x200 was SDL_WINDOWEVENT, reserve the number for sdl2-compat */
    /* 0x201 was SDL_SYSWMEVENT, reserve the number for sdl2-compat */
    windowShown = 0x202,       /**< Window has been shown */
    windowHidden,              /**< Window has been hidden */
    windowExposed,             /**< Window has been exposed and should be
                                             redrawn, and can be redrawn directly from event
                                             watchers for this event */
    windowMoved,               /**< Window has been moved to data1, data2 */
    windowResized,             /**< Window has been resized to data1xdata2 */
    windowPixelSizeChanged,    /**< The pixel size of the window has
                                               changed to data1xdata2 */
    windowMetalViewResized,    /**< The pixel size of a Metal view
                                               associated with the window has
                                               changed */
    windowMinimized,           /**< Window has been minimized */
    windowMaximized,           /**< Window has been maximized */
    windowRestored,            /**< Window has been restored to normal size and
                                             position */
    windowMouseEnter,          /**< Window has gained mouse focus */
    windowMouseLeave,          /**< Window has lost mouse focus */
    windowFocusGained,         /**< Window has gained keyboard focus */
    windowFocusLost,           /**< Window has lost keyboard focus */
    windowCloseRequested,      /**< The window manager requests that the
                                              window be closed */
    windowHitTest,             /**< Wincdow had a hit test that wasn't
                                              SDL_HITTEST_NORMAL */
    windowIccprofChanged,      /**< The ICC profile of the window's
                                              display has changed */
    windowDisplayChanged,      /**< Window has been moved to display
                                              data1 */
    windowDisplayScaleChanged, /**< Window display scale has been
                                               changed */
    windowSafeAreaChanged,     /**< The window safe area has been
                                               changed */
    windowOccluded,            /**< The window has been occluded */
    windowEnterFullscreen,     /**< The window has entered fullscreen
                                              mode */
    windowLeaveFullscreen,     /**< The window has left fullscreen mode
                                */
    windowDestroyed,           /**< The window with the associated ID is being
                                             or has been destroyed. If this message is
                                             being handled in an event watcher, the window
                                             handle is still valid and can still be used
                                             to retrieve any properties associated with
                                             the window. Otherwise, the handle has already
                                             been destroyed and all resources associated
                                             with it are invalid */
    windowHdrStateChanged,     /**< Window HDR properties have changed
                                */
    windowFirst = windowShown,
    windowLast = windowHdrStateChanged,

    /* Keyboard events */
    keyDown = 0x300,       /**< Key pressed */
    keyUp,                 /**< Key released */
    textEditing,           /**< Keyboard text editing (composition) */
    textInput,             /**< Keyboard text input */
    keyMapChanged,         /**< Keymap changed due to a system event such as
                                         an input language or keyboard layout change. */
    keyboardAdded,         /**< A new keyboard has been inserted into the
                                         system */
    keyboardRemoved,       /**< A keyboard has been removed */
    textEditingCandidates, /**< Keyboard text editing candidates */

    /* Mouse events */
    mouseMotion = 0x400, /**< Mouse moved */
    mouseButtonDown,     /**< Mouse button pressed */
    mouseButtonUp,       /**< Mouse button released */
    mouseWheel,          /**< Mouse wheel motion */
    mouseAdded,          /**< A new mouse has been inserted into the system */
    mouseRemoved,        /**< A mouse has been removed */

    /* Joystick events */
    joystickAxisMotion = 0x600, /**< Joystick axis motion */
    joystickBallMotion,         /**< Joystick trackball motion */
    joystickHatMotion,          /**< Joystick hat position change */
    joystickButtonDown,         /**< Joystick button pressed */
    joystickButtonUp,           /**< Joystick button released */
    joystickAdded,              /**< A new joystick has been inserted into the
                                              system */
    joystickRemoved,            /**< An opened joystick has been removed */
    joystickBatteryUpdated,     /**< Joystick battery level change */
    joystickUpdateComplete,     /**< Joystick update is complete */

    /* Gamepad events */
    gamepadAxisMotion = 0x650, /**< Gamepad axis motion */
    gamepadButtonDown,         /**< Gamepad button pressed */
    gamepadButtonUp,           /**< Gamepad button released */
    gamepadAdded,              /**< A new gamepad has been inserted into the
                                             system */
    gamepadRemoved,            /**< A gamepad has been removed */
    gamepadRemapped,           /**< The gamepad mapping was updated */
    gamepadTouchpadDown,       /**< Gamepad touchpad was touched */
    gamepadTouchpadMotion,     /**< Gamepad touchpad finger was moved */
    gamepadTouchpadUp,         /**< Gamepad touchpad finger was lifted */
    gamepadSensorUpdate,       /**< Gamepad sensor was updated */
    gamepadUpdateComplete,     /**< Gamepad update is complete */
    gamepadSteamHandleUpdated, /**< Gamepad Steam handle has
                                               changed */

    /* Touch events */
    fingerDown = 0x700,
    fingerUp,
    fingerMotion,
    fingerCanceled,

    /* 0x800, 0x801, and 0x802 were the Gesture events from SDL2. Do not reuse
       these values! sdl2-compat needs them! */

    /* Clipboard events */
    clipboardUpdate = 0x900, /**< The clipboard or primary selection changed */

    /* Drag and drop events */
    dropFile = 0x1000, /**< The system requests a file open */
    dropText,          /**< text/plain drag-and-drop event */
    dropBegin,         /**< A new set of drops is beginning (NULL filename)
                        */
    dropComplete,      /**< Current set of drops is now complete (NULL
                                     filename) */
    dropPosition,      /**< Position while moving over the window */

    /* Audio hotplug events */
    audioDeviceAdded = 0x1100, /**< A new audio device is available */
    audioDeviceRemoved,        /**< An audio device has been removed. */
    audioDeviceFormatChanged,  /**< An audio device's format has
                                               been changed by the system. */

    /* Sensor events */
    sensorUpdate = 0x1200, /**< A sensor was updated */

    /* Pressure-sensitive pen events */
    penProximityIn = 0x1300, /**< Pressure-sensitive pen has become available */
    penProximityOut,         /**< Pressure-sensitive pen has become
                                            unavailable */
    penDown,       /**< Pressure-sensitive pen touched drawing surface */
    penUp,         /**< Pressure-sensitive pen stopped touching drawing
                                 surface */
    penButtonDown, /**< Pressure-sensitive pen button pressed */
    penButtonUp,   /**< Pressure-sensitive pen button released */
    penMotion,     /**< Pressure-sensitive pen is moving on the tablet */
    penAxis,       /**< Pressure-sensitive pen angle/pressure/etc changed
                    */

    /* Camera hotplug events */
    cameraDeviceAdded = 0x1400, /**< A new camera device is available */
    cameraDeviceRemoved,        /**< A camera device has been removed. */
    cameraDeviceApproved,       /**< A camera device has been approved for
                                               use by the user. */
    cameraDeviceDenied,         /**< A camera device has been denied for use
                                               by the user. */

    /* Render events */
    renderTargetsReset = 0x2000, /**< The render targets have been reset and
                                    their contents need to be updated */
    renderDeviceReset,           /**< The device has been reset and all
                                                textures need to be recreated */
    renderDeviceLost,            /**< The device has been lost and can't be
                                                recovered. */

    /* Reserved events for private platforms */
    private0 = 0x4000,
    private1,
    private2,
    private3,

    /* Internal events */
    pollSentinel = 0x7F00, /**< Signals the end of an event poll cycle */

    /** Events USER through SDL_EVENT_LAST are for your use,
     *  and should be allocated with SDL_RegisterEvents()
     */
    user = 0x8000,

    /**
     *  This last event is only for bounding internal arrays
     */
    last = 0xFFFF,

    /* This just makes sure the enum is the size of uint32_t */
    enumPadding = 0x7FFFFFFF,
};

using typeUnderlying_t = std::underlying_type_t< type_t >;

[[nodiscard]] constexpr auto toLegacy( type_t _value ) -> SDL_EventType {
    return ( static_cast< SDL_EventType >( _value ) );
}

[[nodiscard]] constexpr auto toLegacy( type_t* _value ) -> SDL_EventType* {
    return ( std::bit_cast< SDL_EventType* >( _value ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_EventType _value ) -> type_t {
    return ( static_cast< type_t >( _value ) );
}

// Fields shared by every event
using common_t = struct commont {
    using native_t = SDL_CommonEvent;

    // Variables
public:
    uint32_t
        type; /**< Event type, shared with all events, uint32_t to cover user
               events which are not in the SDL_EventType enumeration */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
};

// Display state change event data (event.display.*)
using display_t = struct display {
    using native_t = SDL_DisplayEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_DISPLAYEVENT_* */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    video::display::id_t displayID; /**< The associated display */
    int32_t data1;                  /**< event dependent data */
    int32_t data2;                  /**< event dependent data */
};

// Window state change event data (event.window.*)
using window_t = struct window {
    using native_t = SDL_DisplayEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_WINDOW_* */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    video::window::id_t windowID; /**< The associated window */
    int32_t data1;                /**< event dependent data */
    int32_t data2;                /**< event dependent data */
};

namespace keyboard {

// Keyboard device event structure (event.kdevice.*)
using device_t = struct keyboardDevice {
    using native_t = SDL_KeyboardDeviceEvent;

    // Variables
public:
    SDL_EventType
        type; /**< SDL_EVENT_KEYBOARD_ADDED or SDL_EVENT_KEYBOARD_REMOVED */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_KeyboardID which; /**< The keyboard instance id */
};

// Keyboard button event structure (event.key.*)
//
// The `key` is the base SDL_Keycode generated by pressing the `scancode`
// using the current keyboard layout, applying any options specified in
// SDL_HINT_KEYCODE_OPTIONS. You can get the SDL_Keycode corresponding to the
// event scancode and modifiers directly from the keyboard layout, bypassing
// SDL_HINT_KEYCODE_OPTIONS, by calling SDL_GetKeyFromScancode().
using keyboard_t = struct keyboard {
    using native_t = SDL_KeyboardEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_KEY_DOWN or SDL_EVENT_KEY_UP */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    video::window::id_t windowID; /**< The window with keyboard focus, if any */
    SDL_KeyboardID
        which; /**< The keyboard instance id, or 0 if unknown or virtual */
    SDL_Scancode scancode; /**< SDL physical key code */
    SDL_Keycode key;       /**< SDL virtual key code */
    SDL_Keymod mod;        /**< current key modifiers */
    uint16_t raw; /**< The platform dependent scancode for this event */
    bool down;    /**< true if the key is pressed */
    bool repeat;  /**< true if this is a key repeat */
};

// Keyboard text editing event structure (event.edit.*)
//
// The start cursor is the position, in UTF-8 characters, where new typing
// will be inserted into the editing text. The length is the number of UTF-8
// characters that will be replaced by new typing.
using textEditing_t = struct textEditing {
    using native_t = SDL_TextEditingEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_TEXT_EDITING */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    video::window::id_t windowID; /**< The window with keyboard focus, if any */
    const char* text;             /**< The editing text */
    int32_t start; /**< The start cursor of selected editing text, or -1 if not
                     set */
    int32_t
        length; /**< The length of selected editing text, or -1 if not set */
};

// Keyboard IME candidates event structure (event.edit_candidates.*)
using textEditingCandidates_t = struct textEditingCandidates {
    using native_t = SDL_TextEditingCandidatesEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_TEXT_EDITING_CANDIDATES */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    video::window::id_t windowID; /**< The window with keyboard focus, if any */
    const char* const* candidates; /**< The list of candidates, or NULL if there
                                      are no candidates available */
    int32_t num_candidates;        /**< The number of strings in `candidates` */
    int32_t selected_candidate; /**< The index of the selected candidate, or -1
                                  if no candidate is selected */
    bool horizontal; /**< true if the list is horizontal, false if it's vertical
                      */
    uint8_t padding1;
    uint8_t padding2;
    uint8_t padding3;
};

// Keyboard text input event structure (event.text.*)
//
// This event will never be delivered unless text input is enabled by calling
// SDL_StartTextInput(). Text input is disabled by default!
using textInput_t = struct textInput {
    using native_t = SDL_TextInputEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_TEXT_INPUT */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    video::window::id_t windowID; /**< The window with keyboard focus, if any */
    const char* text;             /**< The input text, UTF-8 encoded */
};

} // namespace keyboard

namespace mouse {

// Mouse device event structure (event.mdevice.*)
using device_t = struct device {
    using native_t = SDL_MouseDeviceEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_MOUSE_ADDED or SDL_EVENT_MOUSE_REMOVED */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_MouseID which;  /**< The mouse instance id */
};

// Mouse motion event structure (event.motion.*)
using motion_t = struct motion {
    using native_t = SDL_MouseMotionEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_MOUSE_MOTION */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    video::window::id_t windowID; /**< The window with mouse focus, if any */
    SDL_MouseID which;            /**< The mouse instance id in relative mode,
                                     SDL_TOUCH_MOUSEID for touch events, or 0 */
    SDL_MouseButtonFlags state;   /**< The current button state */
    float x;                      /**< X coordinate, relative to window */
    float y;                      /**< Y coordinate, relative to window */
    float xrel;                   /**< The relative motion in the X direction */
    float yrel;                   /**< The relative motion in the Y direction */
};

// Mouse button event structure (event.button.*)
using button_t = struct button {
    using native_t = SDL_MouseButtonEvent;

    // Variables
public:
    SDL_EventType
        type; /**< SDL_EVENT_MOUSE_BUTTON_DOWN or SDL_EVENT_MOUSE_BUTTON_UP */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    video::window::id_t windowID; /**< The window with mouse focus, if any */
    SDL_MouseID which;            /**< The mouse instance id in relative mode,
                                     SDL_TOUCH_MOUSEID for touch events, or 0 */
    uint8_t button;               /**< The mouse button index */
    bool down;                    /**< true if the button is pressed */
    uint8_t clicks; /**< 1 for single-click, 2 for double-click, etc. */
    uint8_t padding;
    float x; /**< X coordinate, relative to window */
    float y; /**< Y coordinate, relative to window */
};

// Mouse wheel event structure (event.wheel.*)
using wheel_t = struct wheel {
    using native_t = SDL_MouseWheelEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_MOUSE_WHEEL */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    video::window::id_t windowID; /**< The window with mouse focus, if any */
    SDL_MouseID which; /**< The mouse instance id in relative mode or 0 */
    float x; /**< The amount scrolled horizontally, positive to the right and
                negative to the left */
    float y; /**< The amount scrolled vertically, positive away from the user
                and negative toward the user */
    SDL_MouseWheelDirection
        direction; /**< Set to one of the SDL_MOUSEWHEEL_* defines. When FLIPPED
                      the values in X and Y will be opposite. Multiply by -1 to
                      change them back */
    float mouse_x; /**< X coordinate, relative to window */
    float mouse_y; /**< Y coordinate, relative to window */
    int32_t integer_x; /**< The amount scrolled horizontally, accumulated to
                         whole scroll "ticks" (added in 3.2.12) */
    int32_t integer_y; /**< The amount scrolled vertically, accumulated to whole
                         scroll "ticks" (added in 3.2.12) */
};

} // namespace mouse

namespace joystick {

// Joystick axis motion event structure (event.jaxis.*)
using axis_t = struct axis {
    using native_t = SDL_JoyAxisEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_JOYSTICK_AXIS_MOTION */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_JoystickID which; /**< The joystick instance id */
    uint8_t axis;         /**< The joystick axis index */
    uint8_t padding1;
    uint8_t padding2;
    uint8_t padding3;
    int16_t value; /**< The axis value (range: -32768 to 32767) */
    uint16_t padding4;
};

// Joystick trackball motion event structure (event.jball.*)
using ball_t = struct ball {
    using native_t = SDL_JoyBallEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_JOYSTICK_BALL_MOTION */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_JoystickID which; /**< The joystick instance id */
    uint8_t ball;         /**< The joystick trackball index */
    uint8_t padding1;
    uint8_t padding2;
    uint8_t padding3;
    int16_t xrel; /**< The relative motion in the X direction */
    int16_t yrel; /**< The relative motion in the Y direction */
};

// Joystick hat position change event structure (event.jhat.*)
using hat_t = struct hat {
    using native_t = SDL_JoyHatEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_JOYSTICK_HAT_MOTION */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_JoystickID which; /**< The joystick instance id */
    uint8_t hat;          /**< The joystick hat index */
    uint8_t value;        /**< The hat position value.
                           *   \sa SDL_HAT_LEFTUP SDL_HAT_UP SDL_HAT_RIGHTUP
                           *   \sa SDL_HAT_LEFT SDL_HAT_CENTERED SDL_HAT_RIGHT
                           *   \sa SDL_HAT_LEFTDOWN SDL_HAT_DOWN SDL_HAT_RIGHTDOWN
                           *
                           *   Note that zero means the POV is centered.
                           */
    uint8_t padding1;
    uint8_t padding2;
};

// Joystick button event structure (event.jbutton.*)
using button_t = struct button {
    using native_t = SDL_JoyButtonEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_JOYSTICK_BUTTON_DOWN or
                           SDL_EVENT_JOYSTICK_BUTTON_UP */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_JoystickID which; /**< The joystick instance id */
    uint8_t button;       /**< The joystick button index */
    bool down;            /**< true if the button is pressed */
    uint8_t padding1;
    uint8_t padding2;
};

// Joystick device event structure (event.jdevice.*)
//
// SDL will send JOYSTICK_ADDED events for devices that are already plugged in
// during SDL_Init.
using device_t = struct device {
    using native_t = SDL_JoyDeviceEvent;

    // Variables
public:
    SDL_EventType
        type; /**< SDL_EVENT_JOYSTICK_ADDED or SDL_EVENT_JOYSTICK_REMOVED or
                 SDL_EVENT_JOYSTICK_UPDATE_COMPLETE */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_JoystickID which; /**< The joystick instance id */
};

// Joystick battery level change event structure (event.jbattery.*)
using battery_t = struct battery {
    using native_t = SDL_JoyBatteryEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_JOYSTICK_BATTERY_UPDATED */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_JoystickID which; /**< The joystick instance id */
    SDL_PowerState state; /**< The joystick battery state */
    int percent;          /**< The joystick battery percent charge remaining */
};

} // namespace joystick

namespace gamepad {

// Gamepad axis motion event structure (event.gaxis.*)
using axis_t = struct axis {
    using native_t = SDL_GamepadAxisEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_GAMEPAD_AXIS_MOTION */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_JoystickID which; /**< The joystick instance id */
    uint8_t axis;         /**< The gamepad axis (SDL_GamepadAxis) */
    uint8_t padding1;
    uint8_t padding2;
    uint8_t padding3;
    int16_t value; /**< The axis value (range: -32768 to 32767) */
    uint16_t padding4;
};

// Gamepad button event structure (event.gbutton.*)
using button_t = struct button {
    using native_t = SDL_GamepadButtonEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_GAMEPAD_BUTTON_DOWN or
                           SDL_EVENT_GAMEPAD_BUTTON_UP */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_JoystickID which; /**< The joystick instance id */
    uint8_t button;       /**< The gamepad button (SDL_GamepadButton) */
    bool down;            /**< true if the button is pressed */
    uint8_t padding1;
    uint8_t padding2;
};

// Gamepad device event structure (event.gdevice.*)
//
// Joysticks that are supported gamepads receive both an SDL_JoyDeviceEvent
// and an SDL_GamepadDeviceEvent.
//
// SDL will send GAMEPAD_ADDED events for joysticks that are already plugged
// in during SDL_Init() and are recognized as gamepads. It will also send
// events for joysticks that get gamepad mappings at runtime.
using device_t = struct device {
    using native_t = SDL_GamepadDeviceEvent;

    // Variables
public:
    SDL_EventType
        type; /**< SDL_EVENT_GAMEPAD_ADDED, SDL_EVENT_GAMEPAD_REMOVED, or
                 SDL_EVENT_GAMEPAD_REMAPPED, SDL_EVENT_GAMEPAD_UPDATE_COMPLETE
                 or SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_JoystickID which; /**< The joystick instance id */
};

// Gamepad touchpad event structure (event.gtouchpad.*)
using touchpad_t = struct touchpad {
    using native_t = SDL_GamepadTouchpadEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN or
                           SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION or
                           SDL_EVENT_GAMEPAD_TOUCHPAD_UP */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_JoystickID which; /**< The joystick instance id */
    int32_t touchpad;     /**< The index of the touchpad */
    int32_t finger;       /**< The index of the finger on the touchpad */
    float x; /**< Normalized in the range 0...1 with 0 being on the left */
    float y; /**< Normalized in the range 0...1 with 0 being at the top */
    float pressure; /**< Normalized in the range 0...1 */
};

// Gamepad sensor event structure (event.gsensor.*)
using sensor_t = struct sensor {
    using native_t = SDL_GamepadSensorEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_GAMEPAD_SENSOR_UPDATE */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_JoystickID which; /**< The joystick instance id */
    int32_t sensor;       /**< The type of the sensor, one of the values of
                            SDL_SensorType */
    std::array< float, 3 > data; /**< Up to 3 values from the sensor, as defined
                           in SDL_sensor.h */
    uint64_t sensor_timestamp;   /**< The timestamp of the sensor reading in
                                  nanoseconds, not necessarily synchronized with
                                  the system clock */
};

} // namespace gamepad

namespace audio {

// Audio device event structure (event.adevice.*)
using device_t = struct device {
    using native_t = SDL_AudioDeviceEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_AUDIO_DEVICE_ADDED, or
                           SDL_EVENT_AUDIO_DEVICE_REMOVED, or
                           SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_AudioDeviceID which; /**< SDL_AudioDeviceID for the device being added
                                or removed or changing */
    bool recording; /**< false if a playback device, true if a recording device.
                     */
    uint8_t padding1;
    uint8_t padding2;
    uint8_t padding3;
};

} // namespace audio

namespace camera {

// Camera device event structure (event.cdevice.*)
using device_t = struct device {
    using native_t = SDL_CameraDeviceEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_CAMERA_DEVICE_ADDED,
                           SDL_EVENT_CAMERA_DEVICE_REMOVED,
                           SDL_EVENT_CAMERA_DEVICE_APPROVED,
                           SDL_EVENT_CAMERA_DEVICE_DENIED */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_CameraID which; /**< SDL_CameraID for the device being added or removed
                           or changing */
};

} // namespace camera

// Renderer event structure (event.render.*)
using render_t = struct render {
    using native_t = SDL_RenderEvent;

    // Variables
public:
    SDL_EventType
        type; /**< SDL_EVENT_RENDER_TARGETS_RESET,
                 SDL_EVENT_RENDER_DEVICE_RESET, SDL_EVENT_RENDER_DEVICE_LOST */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    video::window::id_t
        windowID; /**< The window containing the renderer in question. */
};

// Touch finger event structure (event.tfinger.*)
//
// Coordinates in this event are normalized. `x` and `y` are normalized to a
// range between 0.0f and 1.0f, relative to the window, so (0,0) is the top
// left and (1,1) is the bottom right. Delta coordinates `dx` and `dy` are
// normalized in the ranges of -1.0f (traversed all the way from the bottom or
// right to all the way up or left) to 1.0f (traversed all the way from the
// top or left to all the way down or right).
//
// Note that while the coordinates are _normalized_, they are not _clamped_,
// which means in some circumstances you can get a value outside of this
// range. For example, a renderer using logical presentation might give a
// negative value when the touch is in the letterboxing. Some platforms might
// report a touch outside of the window, which will also be outside of the
// range.
using touchFinger_t = struct touchFinger {
    using native_t = SDL_TouchFingerEvent;

    // Variables
public:
    SDL_EventType
        type; /**< SDL_EVENT_FINGER_DOWN, SDL_EVENT_FINGER_UP,
                 SDL_EVENT_FINGER_MOTION, or SDL_EVENT_FINGER_CANCELED */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_TouchID touchID; /**< The touch device id */
    SDL_FingerID fingerID;
    float x;        /**< Normalized in the range 0...1 */
    float y;        /**< Normalized in the range 0...1 */
    float dx;       /**< Normalized in the range -1...1 */
    float dy;       /**< Normalized in the range -1...1 */
    float pressure; /**< Normalized in the range 0...1 */
    video::window::id_t
        windowID; /**< The window underneath the finger, if any */
};

namespace pen {

// Pressure-sensitive pen proximity event structure (event.pproximity.*)
//
// When a pen becomes visible to the system (it is close enough to a tablet,
// etc), SDL will send an SDL_EVENT_PEN_PROXIMITY_IN event with the new pen's
// ID. This ID is valid until the pen leaves proximity again (has been removed
// from the tablet's area, the tablet has been unplugged, etc). If the same
// pen reenters proximity again, it will be given a new ID.
//
// Note that "proximity" means "close enough for the tablet to know the tool
// is there." The pen touching and lifting off from the tablet while not
// leaving the area are handled by SDL_EVENT_PEN_DOWN and SDL_EVENT_PEN_UP.
using proximity_t = struct proximity {
    using native_t = SDL_PenProximityEvent;

    // Variables
public:
    SDL_EventType
        type; /**< SDL_EVENT_PEN_PROXIMITY_IN or SDL_EVENT_PEN_PROXIMITY_OUT */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    video::window::id_t windowID; /**< The window with pen focus, if any */
    SDL_PenID which;              /**< The pen instance id */
};

// Pressure-sensitive pen motion event structure (event.pmotion.*)
//
// Depending on the hardware, you may get motion events when the pen is not
// touching a tablet, for tracking a pen even when it isn't drawing. You
// should listen for SDL_EVENT_PEN_DOWN and SDL_EVENT_PEN_UP events, or check
// `pen_state & SDL_PEN_INPUT_DOWN` to decide if a pen is "drawing" when
// dealing with pen motion.
using motion_t = struct motion {
    using native_t = SDL_PenMotionEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_PEN_MOTION */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    video::window::id_t windowID; /**< The window with pen focus, if any */
    SDL_PenID which;              /**< The pen instance id */
    SDL_PenInputFlags
        pen_state; /**< Complete pen input state at time of event */
    float x;       /**< X coordinate, relative to window */
    float y;       /**< Y coordinate, relative to window */
};

// Pressure-sensitive pen touched event structure (event.ptouch.*)
//
// These events come when a pen touches a surface (a tablet, etc), or lifts
// off from one.
using touch_t = struct touch {
    using native_t = SDL_PenTouchEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_PEN_DOWN or SDL_EVENT_PEN_UP */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    video::window::id_t windowID; /**< The window with pen focus, if any */
    SDL_PenID which;              /**< The pen instance id */
    SDL_PenInputFlags
        pen_state; /**< Complete pen input state at time of event */
    float x;       /**< X coordinate, relative to window */
    float y;       /**< Y coordinate, relative to window */
    bool eraser; /**< true if eraser end is used (not all pens support this). */
    bool down;   /**< true if the pen is touching or false if the pen is lifted
                    off */
};

// Pressure-sensitive pen button event structure (event.pbutton.*)
//
// This is for buttons on the pen itself that the user might click. The pen
// itself pressing down to draw triggers a SDL_EVENT_PEN_DOWN event instead.
using button_t = struct button {
    using native_t = SDL_PenButtonEvent;

    // Variables
public:
    SDL_EventType
        type; /**< SDL_EVENT_PEN_BUTTON_DOWN or SDL_EVENT_PEN_BUTTON_UP */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    video::window::id_t windowID; /**< The window with mouse focus, if any */
    SDL_PenID which;              /**< The pen instance id */
    SDL_PenInputFlags
        pen_state;  /**< Complete pen input state at time of event */
    float x;        /**< X coordinate, relative to window */
    float y;        /**< Y coordinate, relative to window */
    uint8_t button; /**< The pen button index (first button is 1). */
    bool down;      /**< true if the button is pressed */
};

// Pressure-sensitive pen pressure / angle event structure (event.paxis.*)
//
// You might get some of these events even if the pen isn't touching the
// tablet.
using axis_t = struct axis {
    using native_t = SDL_PenAxisEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_PEN_AXIS */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    video::window::id_t windowID; /**< The window with pen focus, if any */
    SDL_PenID which;              /**< The pen instance id */
    SDL_PenInputFlags
        pen_state;    /**< Complete pen input state at time of event */
    float x;          /**< X coordinate, relative to window */
    float y;          /**< Y coordinate, relative to window */
    SDL_PenAxis axis; /**< Axis that has changed */
    float value;      /**< New value of axis */
};

} // namespace pen

namespace file {

// An event used to drop text or request a file open by the system
// (event.drop.*)
using drop_t = struct drop {
    using native_t = SDL_DropEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_DROP_BEGIN or SDL_EVENT_DROP_FILE or
                           SDL_EVENT_DROP_TEXT or SDL_EVENT_DROP_COMPLETE or
                           SDL_EVENT_DROP_POSITION */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    video::window::id_t windowID; /**< The window that was dropped on, if any */
    float x;            /**< X coordinate, relative to window (not on begin) */
    float y;            /**< Y coordinate, relative to window (not on begin) */
    const char* source; /**< The source app that sent this drop event, or NULL
                           if that isn't available */
    const char* data;   /**< The text for SDL_EVENT_DROP_TEXT and the file name
                           for SDL_EVENT_DROP_FILE, NULL for other events */
};

} // namespace file

// An event triggered when the clipboard contents have changed
// (event.clipboard.*)
using clipboard_t = struct clipboard {
    using native_t = SDL_ClipboardEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_CLIPBOARD_UPDATE */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    bool owner;         /**< are we owning the clipboard (internal update) */
    int32_t num_mime_types;  /**< number of mime types */
    const char** mime_types; /**< current mime types */
};

// Sensor event structure (event.sensor.*)
using sensor_t = struct sensor {
    using native_t = SDL_SensorEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_SENSOR_UPDATE */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    SDL_SensorID which; /**< The instance ID of the sensor */
    std::array< float, 6 >
        data; /**< Up to 6 values from the sensor - additional values can
        be queried using SDL_GetSensorData() */
    uint64_t sensor_timestamp; /**< The timestamp of the sensor reading in
                                nanoseconds, not necessarily synchronized with
                                the system clock */
};

// The "quit requested" event
using quit_t = struct quit {
    using native_t = SDL_QuitEvent;

    // Variables
public:
    SDL_EventType type; /**< SDL_EVENT_QUIT */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
};

// A user-defined event type (event.user.*)
//
// This event is unique; it is never created by SDL, but only by the
// application. The event can be pushed onto the event queue using
// SDL_PushEvent(). The contents of the structure members are completely up to
// the programmer; the only requirement is that '''type''' is a value obtained
// from SDL_RegisterEvents().
using user_t = struct user {
    using native_t = SDL_UserEvent;

    // Variables
public:
    uint32_t type; /**< SDL_EVENT_USER through SDL_EVENT_LAST-1, uint32_t
                    because these are not in the SDL_EventType enumeration */
    uint32_t reserved;
    uint64_t timestamp; /**< In nanoseconds, populated using SDL_GetTicksNS() */
    video::window::id_t windowID; /**< The associated window if any */
    int32_t code;                 /**< User defined event code */
    void* data1;                  /**< User defined data pointer */
    void* data2;                  /**< User defined data pointer */
};

// The structure for all events in SDL.
//
// The SDL_Event structure is the core of all event handling in SDL. SDL_Event
// is a union of all event structures used in SDL.
using event_t = SDL_Event;
#if 0
using event_t = union event {
    using native_t = SDL_Event;

    constexpr operator native_t() const {
        return {
            type,
    common,          
    display,        
    window,          
    kdevice, 
    key,           
    edit,       
    edit_candidates,            
    text,        
    mdevice,   
    motion,    
    button,    
    wheel,      
    jdevice,     
    jaxis,         
    jball,         
    jhat,           
    jbutton,     
    jbattery,   
    gdevice, 
    gaxis,     
    gbutton, 
    gtouchpad, 
    gsensor,     
    adevice,       
    cdevice,      
    sensor,             
    quit,                 
    user,                 
    tfinger,       
    pproximity,   
    ptouch,           
    pmotion,         
    pbutton,         
    paxis,             
    render,             
    drop,                 
    clipboard,       
        };
    }

    // Variables
public:
    uint32_t
        type; /**< Event type, shared with all events, uint32_t to cover user
               events which are not in the SDL_EventType enumeration */
    SDL_CommonEvent common;          /**< Common event data */
    SDL_DisplayEvent display;        /**< Display event data */
    SDL_WindowEvent window;          /**< Window event data */
    SDL_KeyboardDeviceEvent kdevice; /**< Keyboard device change event data */
    SDL_KeyboardEvent key;           /**< Keyboard event data */
    SDL_TextEditingEvent edit;       /**< Text editing event data */
    SDL_TextEditingCandidatesEvent
        edit_candidates;            /**< Text editing candidates event data */
    SDL_TextInputEvent text;        /**< Text input event data */
    SDL_MouseDeviceEvent mdevice;   /**< Mouse device change event data */
    SDL_MouseMotionEvent motion;    /**< Mouse motion event data */
    SDL_MouseButtonEvent button;    /**< Mouse button event data */
    SDL_MouseWheelEvent wheel;      /**< Mouse wheel event data */
    SDL_JoyDeviceEvent jdevice;     /**< Joystick device change event data */
    SDL_JoyAxisEvent jaxis;         /**< Joystick axis event data */
    SDL_JoyBallEvent jball;         /**< Joystick ball event data */
    SDL_JoyHatEvent jhat;           /**< Joystick hat event data */
    SDL_JoyButtonEvent jbutton;     /**< Joystick button event data */
    SDL_JoyBatteryEvent jbattery;   /**< Joystick battery event data */
    SDL_GamepadDeviceEvent gdevice; /**< Gamepad device event data */
    SDL_GamepadAxisEvent gaxis;     /**< Gamepad axis event data */
    SDL_GamepadButtonEvent gbutton; /**< Gamepad button event data */
    SDL_GamepadTouchpadEvent gtouchpad; /**< Gamepad touchpad event data */
    SDL_GamepadSensorEvent gsensor;     /**< Gamepad sensor event data */
    SDL_AudioDeviceEvent adevice;       /**< Audio device event data */
    SDL_CameraDeviceEvent cdevice;      /**< Camera device event data */
    SDL_SensorEvent sensor;             /**< Sensor event data */
    SDL_QuitEvent quit;                 /**< Quit request event data */
    SDL_UserEvent user;                 /**< Custom event data */
    SDL_TouchFingerEvent tfinger;       /**< Touch finger event data */
    SDL_PenProximityEvent pproximity;   /**< Pen proximity event data */
    SDL_PenTouchEvent ptouch;           /**< Pen tip touching event data */
    SDL_PenMotionEvent pmotion;         /**< Pen motion event data */
    SDL_PenButtonEvent pbutton;         /**< Pen button event data */
    SDL_PenAxisEvent paxis;             /**< Pen axis event data */
    SDL_RenderEvent render;             /**< Render event data */
    SDL_DropEvent drop;                 /**< Drag and drop event data */
    SDL_ClipboardEvent clipboard;       /**< Clipboard event data */

    /* This is necessary for ABI compatibility between Visual C++ and GCC.
       Visual C++ will respect the push pack pragma and use 52 bytes (size of
       SDL_TextEditingEvent, the largest structure for 32-bit and 64-bit
       architectures) for this union, and GCC will use the alignment of the
       largest datatype within the union, which is 8 bytes on 64-bit
       architectures.

       So... we'll add padding to force the size to be the same for both.

       On architectures where pointers are 16 bytes, this needs rounding up to
       the next multiple of 16, 64, and on architectures where pointers are
       even larger the size of SDL_UserEvent will dominate as being 3 pointers.
    */
    uint8_t padding[ 128 ];
};
#endif

// Make sure we haven't broken binary compatibility.
static_assert( sizeof( event_t ) == sizeof( event_t{}.padding ) );

// Pump the event loop, gathering events from the input devices.
//
// This function updates the event queue and internal input device state.
//
// SDL_PumpEvents() gathers all the pending input information from devices and
// places it in the event queue. Without calls to SDL_PumpEvents() no events
// would ever be placed on the queue. Often the need for calls to
// SDL_PumpEvents() is hidden from the user since SDL_PollEvent() and
// SDL_WaitEvent() implicitly call SDL_PumpEvents(). However, if you are not
// polling or waiting for events (e.g. you are filtering them), then you must
// call SDL_PumpEvents() to force an event queue update.
//
// Should only be called on the main thread.
inline void pump() {
    SDL_PumpEvents();
}

// The type of action to request from SDL_PeepEvents().
using action_t = enum class action : uint8_t {
    addEvent,  /**< Add events to the back of the queue. */
    peekEvent, /**< Check but don't remove events from the queue front. */
    getEvent   /**< Retrieve/remove events from the front of the queue. */
};

using actionUnderlying_t = std::underlying_type_t< action_t >;

[[nodiscard]] constexpr auto toLegacy( action_t _value ) -> SDL_EventAction {
    return ( static_cast< SDL_EventAction >( _value ) );
}

[[nodiscard]] constexpr auto toLegacy( action_t* _value ) -> SDL_EventAction* {
    return ( std::bit_cast< SDL_EventAction* >( _value ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_EventAction _value ) -> action_t {
    return ( static_cast< action_t >( _value ) );
}

// Check the event queue for messages and optionally return them.
//
// `action` may be any of the following:
//
// - `SDL_ADDEVENT`: up to `numevents` events will be added to the back of the
//   event queue.
// - `SDL_PEEKEVENT`: `numevents` events at the front of the event queue,
//   within the specified minimum and maximum type, will be returned to the
//   caller and will _not_ be removed from the queue. If you pass NULL for
//   `events`, then `numevents` is ignored and the total number of matching
//   events will be returned.
// - `SDL_GETEVENT`: up to `numevents` events at the front of the event queue,
//   within the specified minimum and maximum type, will be returned to the
//   caller and will be removed from the queue.
//
// You may have to call SDL_PumpEvents() before calling this function.
// Otherwise, the events may not be ready to be filtered when you call
// SDL_PeepEvents().
//
// Destination buffer for the retrieved events, may be NULL to leave the events
// in the queue and return the number of events that would have been stored.
//
// Minimum value of the event type to be considered; SDL_EVENT_FIRST is a safe
// choice.
//
// Maximum value of the event type to be considered; SDL_EVENT_LAST is a safe
// choice.
[[nodiscard]] inline auto peep(
    action_t _action,
    std::optional< std::span< event_t > > _events = std::nullopt,
    uint32_t _minType = toLegacy( type_t::first ),
    uint32_t _maxType = toLegacy( type_t::last ) ) -> size_t {
    ssize_t l_result = -1;

    if ( _events ) {
        const std::span< event_t >& l_events = _events.value();

        l_result = SDL_PeepEvents( l_events.data(), l_events.size(),
                                   toLegacy( _action ), _minType, _maxType );

    } else {
        stdfunc::assert( _action != action_t::peekEvent );

        l_result = SDL_PeepEvents( nullptr, 0, toLegacy( _action ), _minType,
                                   _maxType );
    }

    assert( l_result != -1 );

    return ( l_result );
}

// Check for the existence of a certain event type in the event queue.
//
// If you need to check for a range of event types, use SDL_HasEvents()
// instead.
[[nodiscard]] inline auto hasSpecific( uint32_t _type ) -> bool {
    return ( SDL_HasEvent( _type ) );
}

// Check for the existence of certain event types in the event queue.
//
// If you need to check for a single event type, use SDL_HasEvent() instead.
//
// true if events with type >= `minType` and <= `maxType` are
//          present, or false if not.
[[nodiscard]] inline auto hasAny( uint32_t _minType = toLegacy( type_t::first ),
                                  uint32_t _maxType = toLegacy( type_t::last ) )
    -> bool {
    return ( SDL_HasEvents( _minType, _maxType ) );
}

// Clear events of a specific type from the event queue.
//
// This will unconditionally remove any events from the queue that match
// `type`. If you need to remove a range of event types, use SDL_FlushEvents()
// instead.
//
// It's also normal to just ignore events you don't care about in your event
// loop without calling this function.
//
// This function only affects currently queued events. If you want to make
// sure that all pending OS events are flushed, you can call SDL_PumpEvents()
// on the main thread immediately before the flush call.
//
// If you have user events with custom data that needs to be freed, you should
// use SDL_PeepEvents() to remove and clean up those events before calling
// this function.
//
// The type of event to be cleared; see SDL_EventType for details.
inline void flush( uint32_t _type ) {
    SDL_FlushEvent( _type );
}

// Clear events of a range of types from the event queue.
//
// This will unconditionally remove any events from the queue that are in the
// range of `minType` to `maxType`, inclusive. If you need to remove a single
// event type, use SDL_FlushEvent() instead.
//
// It's also normal to just ignore events you don't care about in your event
// loop without calling this function.
//
// This function only affects currently queued events. If you want to make
// sure that all pending OS events are flushed, you can call SDL_PumpEvents()
// on the main thread immediately before the flush call.
//
// Event types to be cleared, inclusive; see SDL_EventType for details.
inline void flush( uint32_t _minType = toLegacy( type_t::first ),
                   uint32_t _maxType = toLegacy( type_t::last ) ) {
    SDL_FlushEvents( _minType, _maxType );
}

// Poll for currently pending events.
//
// If `event` is not NULL, the next event is removed from the queue and stored
// in the SDL_Event structure pointed to by `event`. The 1 returned refers to
// this event, immediately stored in the SDL Event structure -- not an event
// to follow.
//
// If `event` is NULL, it simply returns 1 if there is an event in the queue,
// but will not remove it from the queue.
//
// As this function may implicitly call SDL_PumpEvents(), you can only call
// this function in the thread that set the video mode.
//
// SDL_PollEvent() is the favored way of receiving system events since it can
// be done from the main loop and does not suspend the main loop while waiting
// on an event to be posted.
//
// The common practice is to fully process the event queue once every frame,
// usually as a first step before updating the game's state:
//
// ```c
// while (game_is_still_running) {
//     SDL_Event event;
//     while (SDL_PollEvent(&event)) {  // poll until all events are handled!
//         // decide what to do with this event.
//     }
//
//     // update game state, draw the current frame
// }
// ```
//
// Should only be called on the main thread.
[[nodiscard]] inline auto poll() -> std::optional< event_t > {
    event_t l_event{};

    const bool l_result = SDL_PollEvent( &l_event );

    if ( l_result ) {
        return ( l_event );

    } else {
        return ( std::nullopt );
    }
}

// Wait indefinitely for the next available event.
//
// If `event` is not NULL, the next event is removed from the queue and stored
// in the SDL_Event structure pointed to by `event`.
//
// As this function may implicitly call SDL_PumpEvents(), you can only call
// this function in the thread that initialized the video subsystem.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto wait() -> std::optional< event_t > {
    event_t l_event{};

    const bool l_result = SDL_WaitEvent( &l_event );

    assert( l_result );

    return ( l_event );
}

// Wait until the specified timeout (in milliseconds) for the next available
// event.
//
// If `event` is not NULL, the next event is removed from the queue and stored
// in the SDL_Event structure pointed to by `event`.
//
// As this function may implicitly call SDL_PumpEvents(), you can only call
// this function in the thread that initialized the video subsystem.
//
// The timeout is not guaranteed, the actual wait time could be longer due to
// system scheduling.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto wait( std::chrono::milliseconds _timeout )
    -> std::optional< event_t > {
    event_t l_event{};

    const bool l_result = SDL_WaitEventTimeout( &l_event, _timeout.count() );

    if ( l_result ) {
        return ( l_event );

    } else {
        return std::nullopt;
    }
}

// Add an event to the event queue.
//
// The event queue can actually be used as a two way communication channel.
// Not only can events be read from the queue, but the user can also push
// their own events onto it. `event` is a pointer to the event structure you
// wish to push onto the queue. The event is copied into the queue, and the
// caller may dispose of the memory pointed to after SDL_PushEvent() returns.
//
// Note: Pushing device input events onto the queue doesn't modify the state
// of the device within SDL.
//
// Note: Events pushed onto the queue with SDL_PushEvent() get passed through
// the event filter but events added with SDL_PeepEvents() do not.
//
// For pushing application-specific events, please use SDL_RegisterEvents() to
// get an event type that does not conflict with other code that also wants
// its own custom event types.
inline void push( event_t& _event ) {
    const bool l_result = SDL_PushEvent( &_event );

    assert( l_result );
}

// A function pointer used for callbacks that watch the event queue.
//
// true to permit event to be added to the queue, and false to disallow it. When
// used with SDL_AddEventWatch, the return value is ignored.
//
// SDL may call this callback at any time from any thread; the application is
// responsible for locking resources the callback touches that need to be
// protected.
using filter_t = gsl::not_null< SDL_EventFilter >;

// Set up a filter to process all events before they are added to the internal
// event queue.
//
// If you just want to see events without modifying them or preventing them
// from being queued, you should use SDL_AddEventWatch() instead.
//
// If the filter function returns true when called, then the event will be
// added to the internal queue. If it returns false, then the event will be
// dropped from the queue, but the internal state will still be updated. This
// allows selective filtering of dynamically arriving events.
//
// **WARNING**: Be very careful of what you do in the event filter function,
// as it may run in a different thread!
//
// On platforms that support it, if the quit event is generated by an
// interrupt signal (e.g. pressing Ctrl-C), it will be delivered to the
// application at the next event poll.
//
// Note: Disabled events never make it to the event filter function; see
// SDL_SetEventEnabled().
//
// Note: Events pushed onto the queue with SDL_PushEvent() get passed through
// the event filter, but events pushed onto the queue with SDL_PeepEvents() do
// not.
//
// Add a callback to be triggered when an event is added to the event queue.
//
// `filter` will be called when an event happens, and its return value is
// ignored.
//
// **WARNING**: Be very careful of what you do in the event filter function,
// as it may run in a different thread!
//
// If the quit event is generated by a signal (e.g. SIGINT), it will bypass
// the internal queue and be delivered to the watch callback immediately, and
// arrive at the next event poll.
//
// Note: the callback is called for events posted by the user through
// SDL_PushEvent(), but not for disabled events, nor for events by a filter
// callback set with SDL_SetEventFilter(), nor for events posted by the user
// through SDL_PeepEvents().
//
// Run a specific filter function on the current event queue, removing any
// events for which the filter returns false.
//
// See SDL_SetEventFilter() for more information. Unlike SDL_SetEventFilter(),
// this function does not change the filter permanently, it only uses the
// supplied filter until this function returns.
inline void filter( filter_t _filter,
                    void* _userData,
                    bool _overwrite = true,
                    bool _runNow = false ) {
    if ( _runNow ) {
        SDL_FilterEvents( _filter, _userData );

    } else {
        if ( _overwrite ) {
            SDL_SetEventFilter( _filter, _userData );

        } else {
            SDL_AddEventWatch( _filter, _userData );
        }
    }
}

// Query the current event filter.
//
// This function can be used to "chain" filters, by saving the existing filter
// before replacing it with a function that will call that saved filter.
[[nodiscard]] inline auto filter()
    -> std::optional< std::pair< filter_t, void* > > {
    SDL_EventFilter l_filter = nullptr;
    void* l_userData = nullptr;

    const bool l_result = SDL_GetEventFilter( &l_filter, &l_userData );

    if ( l_result ) {
        return std::pair{ l_filter, l_userData };

    } else {
        return std::nullopt;
    }
}

// Remove an event watch callback added with SDL_AddEventWatch().
//
// This function takes the same input as SDL_AddEventWatch() to identify and
// delete the corresponding callback.
inline void removeFilter( filter_t _filter, void* _userData ) {
    SDL_RemoveEventWatch( _filter, _userData );
}

// Set the state of processing events by type.
//
// \param type the type of event; see SDL_EventType for details.
// \param enabled whether to process the event or not.
inline void toggle( uint32_t _type, bool _isEnabled ) {
    SDL_SetEventEnabled( _type, _isEnabled );
}

// Query the state of processing events by type.
[[nodiscard]] inline auto isEnabled( uint32_t _type ) -> bool {
    return ( SDL_EventEnabled( _type ) );
}

// Allocate a set of user-defined events, and return the beginning event
// number for that set of events.
//
// The beginning event number, or asserts if numevents is invalid or if there
// are not enough user-defined events left.
// TODO: Rename
[[nodiscard]] inline auto begister( size_t _amount ) -> uint32_t {
    const uint32_t l_result = SDL_RegisterEvents( _amount );

    assert( l_result );

    return ( l_result );
}

// Get window associated with an event.
[[nodiscard]] inline auto window( const event_t& _event ) -> slickdl::window_t {
    return ( SDL_GetWindowFromEvent( &_event ) );
}

// Generate a human-readable description of an event.
//
// This will fill `buf` with a null-terminated string that might look
// something like this:
//
// ```
// SDL_EVENT_MOUSE_MOTION (timestamp=1140256324 windowid=2 which=0 state=0
// x=492.99 y=139.09 xrel=52 yrel=6)
// ```
//
// The exact format of the string is not guaranteed; it is intended for
// logging purposes, to be read by a human, and not parsed by a computer.
//
// The returned value follows the same rules as SDL_snprintf(): `buf` will
// always be NULL-terminated (unless `buflen` is zero), and will be truncated
// if `buflen` is too small. The return code is the number of bytes needed for
// the complete string, not counting the NULL-terminator, whether the string
// was truncated or not. Unlike SDL_snprintf(), though, this function never
// returns -1.
//
// \param buf the buffer to fill with the description string. May be NULL.
// \returns number of bytes needed for the full string, not counting the
//          null-terminator byte.
// TODO: Improve reserve
[[nodiscard]] inline auto description( const std::optional< event_t >& _event )
    -> std::optional< std::string > {
    std::string l_result;

    l_result.reserve( MAX_INPUT );

    if ( _event ) {
        SDL_GetEventDescription( &_event.value(), l_result.data(),
                                 static_cast< int >( l_result.max_size() ) );

    } else {
        SDL_GetEventDescription( nullptr, l_result.data(),
                                 static_cast< int >( l_result.max_size() ) );
    }

    return ( l_result );
}

} // namespace slickdl::events
