#include <SDL3/SDL_mouse.h>

#include <gsl/pointers>
#include <type_traits>

#include "slickdl.hpp"
#include "slickdl/point.hpp"
#include "slickdl/surface.hpp"

// Any GUI application has to deal with the mouse, and SDL provides functions
// to manage mouse input and the displayed cursor.
//
// Most interactions with the mouse will come through the event subsystem.
// Moving a mouse generates an SDL_EVENT_MOUSE_MOTION event, pushing a button
// generates SDL_EVENT_MOUSE_BUTTON_DOWN, etc, but one can also query the
// current state of the mouse at any time with SDL_GetMouseState().
//
// For certain games, it's useful to disassociate the mouse cursor from mouse
// input. An FPS, for example, would not want the player's motion to stop as
// the mouse hits the edge of the window. For these scenarios, use
// SDL_SetWindowRelativeMouseMode(), which hides the cursor, grabs mouse input
// to the window, and reads mouse input no matter how far it moves.
//
// Games that want the system to track the mouse but want to draw their own
// cursor can use SDL_HideCursor() and SDL_ShowCursor(). It might be more
// efficient to let the system manage the cursor, if possible, using
// SDL_SetCursor() with a custom image made through SDL_CreateColorCursor(),
// or perhaps just a specific system cursor from SDL_CreateSystemCursor().
//
// SDL can, on many platforms, differentiate between multiple connected mice,
// allowing for interesting input scenarios and multiplayer games. They can be
// enumerated with SDL_GetMice(), and SDL will send SDL_EVENT_MOUSE_ADDED and
// SDL_EVENT_MOUSE_REMOVED events as they are connected and unplugged.
//
// Since many apps only care about basic mouse input, SDL offers a virtual
// mouse device for touch and pen input, which often can make a desktop
// application work on a touchscreen phone without any code changes. Apps that
// care about touch/pen separately from mouse input should filter out events
// with a `which` field of SDL_TOUCH_MOUSEID/SDL_PEN_MOUSEID.
namespace slickdl::mouse {

// This is a unique ID for a mouse for the time it is connected to the system,
// and is never reused for the lifetime of the application.
//
// If the mouse is disconnected and reconnected, it will get a new ID.
//
// The value 0 is an invalid ID.
using id_t = uint32_t;

// The structure used to identify an SDL cursor.
//
// This is opaque data.
using cursor_t = gsl::not_null< SDL_Cursor* >;

// Cursor types for SDL_CreateSystemCursor().
using systemCursor_t = enum class systemCursor : uint8_t {
    // TODO: Rename
    inaction,  /**< Default cursor. Usually an arrow. */
    text,      /**< Text selection. Usually an I-beam. */
    wait,      /**< Wait. Usually an hourglass or watch or spinning
                  ball. */
    crosshair, /**< Crosshair. */
    progress,  /**< Program is busy but still interactive.
                  Usually it's WAIT with an arrow. */
    northWestSouthEastResize, /**< Double arrow pointing northwest and
                    southeast. */
    northEastSouthWestResize, /**< Double arrow pointing northeast and
                    southwest. */
    eastWestResize,           /**< Double arrow pointing west and east. */
    northSouthResize,         /**< Double arrow pointing north and south. */
    move,            /**< Four pointed arrow pointing north, south, east,
                        and west. */
    notAllowed,      /**< Not permitted. Usually a slashed circle
                         or crossbones. */
    pointer,         /**< Pointer that indicates a link. Usually a
                        pointing hand. */
    northWestResize, /**< Window resize top-left. This may be a
                  single arrow or a double arrow like
                  NWSE_RESIZE. */
    northResize,     /**< Window resize top. May be NS_RESIZE. */
    northEastResize, /**< Window resize top-right. May be
                  NESW_RESIZE. */
    eastResize,      /**< Window resize right. May be EW_RESIZE. */
    southEastResize, /**< Window resize bottom-right. May be
                  NWSE_RESIZE. */
    southResize,     /**< Window resize bottom. May be NS_RESIZE. */
    southWestResize, /**< Window resize bottom-left. May be
                  NESW_RESIZE. */
    westResize,      /**< Window resize left. May be EW_RESIZE. */
    count,
};

using systemCursorUnderlying_t = std::underlying_type_t< systemCursor_t >;

[[nodiscard]] constexpr auto toLegacy( systemCursor_t _state )
    -> SDL_SystemCursor {
    return ( static_cast< SDL_SystemCursor >( _state ) );
}

[[nodiscard]] constexpr auto toLegacy( systemCursor_t* _state )
    -> SDL_SystemCursor* {
    return ( std::bit_cast< SDL_SystemCursor* >( _state ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_SystemCursor _state )
    -> systemCursor_t {
    return ( static_cast< systemCursor_t >( _state ) );
}

// Scroll direction types for the Scroll event
using wheelDirection_t = enum class wheelDirection : uint8_t {
    normal,  /**< The scroll direction is normal */
    flipped, /**< The scroll direction is flipped / natural */
};

using wheelDirectionUnderlying_t = std::underlying_type_t< wheelDirection_t >;

[[nodiscard]] constexpr auto toLegacy( wheelDirection_t _state )
    -> SDL_MouseWheelDirection {
    return ( static_cast< SDL_MouseWheelDirection >( _state ) );
}

[[nodiscard]] constexpr auto toLegacy( wheelDirection_t* _state )
    -> SDL_MouseWheelDirection* {
    return ( std::bit_cast< SDL_MouseWheelDirection* >( _state ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_MouseWheelDirection _state )
    -> wheelDirection_t {
    return ( static_cast< wheelDirection_t >( _state ) );
}

// A bitmask of pressed mouse buttons, as reported by SDL_GetMouseState, etc.
//
// - Button 1: Left mouse button
// - Button 2: Middle mouse button
// - Button 3: Right mouse button
// - Button 4: Side mouse button 1
// - Button 5: Side mouse button 2
using buttonFlags_t = enum class buttonFlags : uint8_t {
    left = 1,
    middle = 2,
    right = 3,
    x1 = 4,
    x2 = 5,
};

using buttonFlagsUnderlying_t = std::underlying_type_t< buttonFlags_t >;

[[nodiscard]] constexpr auto toLegacy( buttonFlags_t _state )
    -> SDL_MouseButtonFlags {
    return ( static_cast< SDL_MouseButtonFlags >( _state ) );
}

[[nodiscard]] constexpr auto toLegacy( buttonFlags_t* _state )
    -> SDL_MouseButtonFlags* {
    return ( std::bit_cast< SDL_MouseButtonFlags* >( _state ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_MouseButtonFlags _state )
    -> buttonFlags_t {
    return ( static_cast< buttonFlags_t >( _state ) );
}

[[nodiscard]] constexpr auto mask( buttonFlags_t _x )
    -> buttonFlagsUnderlying_t {
    return ( 1u << ( static_cast< buttonFlagsUnderlying_t >( _x ) - 1 ) );
}

constexpr size_t g_leftMask = mask( buttonFlags_t::left );
constexpr size_t g_middleMask = mask( buttonFlags_t::middle );
constexpr size_t g_rightMask = mask( buttonFlags_t::right );
constexpr size_t g_x1Mask = mask( buttonFlags_t::x1 );
constexpr size_t g_x2Mask = mask( buttonFlags_t::x2 );

// A callback used to transform mouse motion delta from raw values.
//
// This is called during SDL's handling of platform mouse events to scale the
// values of the resulting motion delta.
//
// \param userdata what was passed as `userdata` to
//                 SDL_SetRelativeMouseTransform().
// \param timestamp the associated time at which this mouse motion event was
//                  received.
// \param window the associated window to which this mouse motion event was
//               addressed.
// \param mouseID the associated mouse from which this mouse motion event was
//                emitted.
// \param x pointer to a variable that will be treated as the resulting x-axis
//          motion.
// \param y pointer to a variable that will be treated as the resulting y-axis
//          motion.
//
// This callback is called by SDL's internal mouse input processing procedure,
// which may be a thread separate from the main event loop that is run at
// realtime priority. Stalling this thread with too much work in the callback
// can therefore potentially freeze the entire system. Care should be taken with
// proper synchronization practices when adding other side effects beyond
// mutation of the x and y values.
using motionTransformCallback_t = SDL_MouseMotionTransformCallback;

// Return whether a mouse is currently connected.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto hasAny() -> bool {
    return ( SDL_HasMouse() );
}

// Get a list of currently connected mice.
//
// Note that this will include any device or virtual driver that includes
// mouse functionality, including some game controllers, KVM switches, etc.
// You should wait for input from a device before you consider it actively in
// use.
//
// Should only be called on the main thread.
[[nodiscard]] auto all() -> std::vector< id_t >;

// Get the name of a mouse.
//
// This function returns "" if the mouse doesn't have a name.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto name( id_t _id ) -> std::string_view {
    return { gsl::make_not_null( SDL_GetMouseNameForID( _id ) ) };
}

// Get the window which currently has mouse focus.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto window() -> window_t {
    return ( SDL_GetMouseFocus() );
}

// Query SDL's cache for the synchronous mouse button state and the
// window-relative SDL-cursor position.
//
// This function returns the cached synchronous state as SDL understands it
// from the last pump of the event queue.
//
// To query the platform for immediate asynchronous state, use
// SDL_GetGlobalMouseState.
//
// Passing non-NULL pointers to `x` or `y` will write the destination with
// respective x or y coordinates relative to the focused window.
//
// In Relative Mode, the SDL-cursor's position usually contradicts the
// platform-cursor's position as manually calculated from
// SDL_GetGlobalMouseState() and SDL_GetWindowPosition.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto state()
    -> std::pair< buttonFlags_t, point_t< float > > {
    point_t< float > l_point{};

    const buttonFlags_t l_result =
        fromLegacy( SDL_GetMouseState( &l_point.x, &l_point.y ) );

    return { l_result, l_point };
}

// Query the platform for the asynchronous mouse button state and the
// desktop-relative platform-cursor position.
//
// This function immediately queries the platform for the most recent
// asynchronous state, more costly than retrieving SDL's cached state in
// SDL_GetMouseState().
//
// Passing non-NULL pointers to `x` or `y` will write the destination with
// respective x or y coordinates relative to the desktop.
//
// In Relative Mode, the platform-cursor's position usually contradicts the
// SDL-cursor's position as manually calculated from SDL_GetMouseState() and
// SDL_GetWindowPosition.
//
// This function can be useful if you need to track the mouse outside of a
// specific window and SDL_CaptureMouse() doesn't fit your needs. For example,
// it could be useful if you need to track the mouse while dragging a window,
// where coordinates relative to a window might not be in sync at all times.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto stateGlobal()
    -> std::pair< buttonFlags_t, point_t< float > > {
    point_t< float > l_point{};

    const buttonFlags_t l_result =
        fromLegacy( SDL_GetGlobalMouseState( &l_point.x, &l_point.y ) );

    return { l_result, l_point };
}

// Query SDL's cache for the synchronous mouse button state and accumulated
// mouse delta since last call.
//
// This function returns the cached synchronous state as SDL understands it
// from the last pump of the event queue.
//
// To query the platform for immediate asynchronous state, use
// SDL_GetGlobalMouseState.
//
// Passing non-NULL pointers to `x` or `y` will write the destination with
// respective x or y deltas accumulated since the last call to this function
// (or since event initialization).
//
// This function is useful for reducing overhead by processing relative mouse
// inputs in one go per-frame instead of individually per-event, at the
// expense of losing the order between events within the frame (e.g. quickly
// pressing and releasing a button within the same frame).
//
// Should only be called on the main thread.
[[nodiscard]] inline auto stateRelative()
    -> std::pair< buttonFlags_t, point_t< float > > {
    point_t< float > l_point{};

    const buttonFlags_t l_result =
        fromLegacy( SDL_GetRelativeMouseState( &l_point.x, &l_point.y ) );

    return { l_result, l_point };
}

// Move the mouse cursor to the given position within the window.
//
// This function generates a mouse motion event if relative mode is not
// enabled. If relative mode is enabled, you can force mouse events for the
// warp by setting the SDL_HINT_MOUSE_RELATIVE_WARP_MOTION hint.
//
// Note that this function will appear to succeed, but not actually move the
// mouse when used over Microsoft Remote Desktop.
//
// NULL for relative.
//
// Should only be called on the main thread.
inline void warpWindow( point_t< float > _point,
                        std::optional< window_t > _window = std::nullopt ) {
    if ( _window ) {
        SDL_WarpMouseInWindow( _window.value(), _point.x, _point.y );

    } else {
        SDL_WarpMouseInWindow( nullptr, _point.x, _point.y );
    }
}

// Move the mouse to the given position in global screen space.
//
// This function generates a mouse motion event.
//
// A failure of this function usually means that it is unsupported by a
// platform.
//
// Note that this function will appear to succeed, but not actually move the
// mouse when used over Microsoft Remote Desktop.
//
// Should only be called on the main thread.
inline void warpGlobal( point_t< float > _point ) {
    const bool l_result = SDL_WarpMouseGlobal( _point.x, _point.y );

    assert( l_result );
}

// Set a user-defined function by which to transform relative mouse inputs.
//
// This overrides the relative system scale and relative speed scale hints.
// Should be called prior to enabling relative mouse mode, fails otherwise.
//
// A callback used to transform relative mouse motion, or NULL for default
// behavior.
//
// Should only be called on the main thread.
// TODO: Implement nullopt for _userData
inline void relativeTransformHandler(
    std::optional< motionTransformCallback_t > _callback = std::nullopt,
    std::optional< void* > _userData = std::nullopt ) {
    bool l_result = false;

    if ( _callback ) {
        l_result = SDL_SetRelativeMouseTransform( _callback.value(),
                                                  _userData.value() );

    } else {
        l_result = SDL_SetRelativeMouseTransform( nullptr, nullptr );
    }

    assert( l_result );
}

// Set relative mouse mode for a window.
//
// While the window has focus and relative mouse mode is enabled, the cursor
// is hidden, the mouse position is constrained to the window, and SDL will
// report continuous relative mouse motion even if the mouse is at the edge
// of the window.
//
// If you'd like to keep the mouse position fixed while in relative mode you
// can use SDL_SetWindowMouseRect(). If you'd like the cursor to be at a
// specific location when relative mode ends, you should use
// SDL_WarpMouseInWindow() before disabling relative mode.
//
// This function will flush any pending mouse motion for this window.
//
// Should only be called on the main thread.
inline void relativeModeToggle( window_t _window, bool _isEnabled ) {
    const bool l_result = SDL_SetWindowRelativeMouseMode( _window, _isEnabled );

    assert( l_result );
}

// Query whether relative mouse mode is enabled for a window.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto relativeMode( window_t _window ) -> bool {
    return ( SDL_GetWindowRelativeMouseMode( _window ) );
}

// Capture the mouse and to track input outside an SDL window.
//
// Capturing enables your app to obtain mouse events globally, instead of
// just within your window. Not all video targets support this function.
// When capturing is enabled, the current window will get all mouse events,
// but unlike relative mode, no change is made to the cursor and it is not
// restrained to your window.
//
// This function may also deny mouse input to other windows--both those in
// your application and others on the system--so you should use this
// function sparingly, and in small bursts. For example, you might want to
// track the mouse while the user is dragging something, until the user
// releases a mouse button. It is not recommended that you capture the mouse
// for long periods of time, such as the entire time your app is running.
// For that, you should probably use SDL_SetWindowRelativeMouseMode() or
// SDL_SetWindowMouseGrab(), depending on your goals.
//
// While captured, mouse events still report coordinates relative to the
// current (foreground) window, but those coordinates may be outside the
// bounds of the window (including negative values). Capturing is only
// allowed for the foreground window. If the window loses focus while
// capturing, the capture will be disabled automatically.
//
// While capturing is enabled, the current window will have the
// `SDL_WINDOW_MOUSE_CAPTURE` flag set.
//
// Please note that SDL will attempt to "auto capture" the mouse while the
// user is pressing a button; this is to try and make mouse behavior more
// consistent between platforms, and deal with the common case of a user
// dragging the mouse outside of the window. This means that if you are
// calling SDL_CaptureMouse() only to deal with this situation, you do not
// have to (although it is safe to do so). If this causes problems for your
// app, you can disable auto capture by setting the
// `SDL_HINT_MOUSE_AUTO_CAPTURE` hint to zero.
//
// Should only be called on the main thread.
inline void capture( bool _isEnabled ) {
    const bool l_result = SDL_CaptureMouse( _isEnabled );

    assert( l_result );
}

// Create a cursor using the specified bitmap data and mask (in MSB format).
//
// `mask` has to be in MSB (Most Significant Bit) format.
//
// The cursor width (`w`) must be a multiple of 8 bits.
//
// The cursor is created in black and white according to the following:
//
// - data=0, mask=1: white
// - data=1, mask=1: black
// - data=0, mask=0: transparent
// - data=1, mask=0: inverted color if possible, black if not.
//
// Cursors created with this function must be freed with
// SDL_DestroyCursor().
//
// If you want to have a color cursor, or create your cursor from an
// SDL_Surface, you should use SDL_CreateColorCursor(). Alternately, you can
// hide the cursor and draw your own as part of your game's rendering, but
// it will be bound to the framerate.
//
// Also, SDL_CreateSystemCursor() is available, which provides several
// readily-available system cursors to pick from.
//
// The color value for each pixel of the cursor.
// The mask value for each pixel of the cursor.
//
// The x-axis offset from the left of the cursor image to the mouse x position,
// in the range of 0 to `w` - 1.
//
// The y-axis offset from the top of the cursor image to the mouse y position,
// in the range of 0 to `h` - 1.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto create( std::span< const uint8_t > _data,
                                  std::span< const uint8_t > _mask,
                                  volume_t< int > _volume,
                                  point_t< int > _hotPoint ) -> cursor_t {
    return ( SDL_CreateCursor( _data.data(), _mask.data(), _volume.width,
                               _volume.height, _hotPoint.x, _hotPoint.y ) );
}

// Create a color cursor.
//
// If this function is passed a surface with alternate representations, the
// surface will be interpreted as the content to be used for 100% display
// scale, and the alternate representations will be used for high DPI
// situations. For example, if the original surface is 32x32, then on a 2x
// macOS display or 200% display scale on Windows, a 64x64 version of the
// image will be used, if available. If a matching version of the image
// isn't available, the closest larger size image will be downscaled to the
// appropriate size and be used instead, if available. Otherwise, the
// closest smaller image will be upscaled and be used instead.
//
// Surface representing the cursor image.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto create( surface_t _surface, point_t< int > _hotPoint )
    -> cursor_t {
    return ( SDL_CreateColorCursor( _surface, _hotPoint.x, _hotPoint.y ) );
}

// Create a system cursor.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto create( systemCursor_t _id ) -> cursor_t {
    return ( SDL_CreateSystemCursor( toLegacy( _id ) ) );
}

// Set the active cursor.
//
// This function sets the currently active cursor to the specified one. If
// the cursor is currently visible, the change will be immediately
// represented on the display. SDL_SetCursor(NULL) can be used to force
// cursor redraw, if this is desired for any reason.
//
// Should only be called on the main thread.
inline void set( cursor_t _cursor ) {
    const bool l_result = SDL_SetCursor( _cursor );

    assert( l_result );
}

// Get the active cursor.
//
// This function returns a pointer to the current cursor which is owned by
// the library. It is not necessary to free the cursor with
// SDL_DestroyCursor().
//
// NULL if there is no mouse.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto active() -> std::optional< cursor_t > {
    SDL_Cursor* l_cursor = SDL_GetCursor();

    if ( l_cursor ) {
        return ( l_cursor );

    } else {
        return ( std::nullopt );
    }
}

// Get the default cursor.
//
// You do not have to call SDL_DestroyCursor() on the return value, but it
// is safe to do so.
//
// Should only be called on the main thread.
// TODO: Rename
[[nodiscard]] inline auto inaction() -> cursor_t {
    return ( SDL_GetDefaultCursor() );
}

// Free a previously-created cursor.
//
// Use this function to free cursor resources created with
// SDL_CreateCursor(), SDL_CreateColorCursor() or SDL_CreateSystemCursor().
//
// Should only be called on the main thread.
inline void destroy( cursor_t _cursor ) {
    SDL_DestroyCursor( _cursor );
}

// Show the cursor.
//
// Should only be called on the main thread.
inline void show() {
    const bool l_result = SDL_ShowCursor();

    assert( l_result );
}

// Hide the cursor.
//
// Should only be called on the main thread.
inline void hide() {
    const bool l_result = SDL_HideCursor();

    assert( l_result );
}

// Return whether the cursor is currently being shown.
//
// Should only be called on the main thread.
inline void isVisible() {
    const bool l_result = SDL_CursorVisible();

    assert( l_result );
}

} // namespace slickdl::mouse
