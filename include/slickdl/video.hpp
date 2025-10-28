#pragma once

#include <SDL3/SDL_video.h>
#include <math.h>

#include <algorithm>
#include <ranges>
#include <vector>

#include "slickdl.hpp"
#include "slickdl/line_box.hpp"
#include "slickdl/pixels_palette.hpp"
#include "slickdl/properties.hpp"
#include "slickdl/surface.hpp"

// SDL's video subsystem is largely interested in abstracting window
// management from the underlying operating system. You can create windows,
// manage them in various ways, set them fullscreen, and get events when
// interesting things happen with them, such as the mouse or keyboard
// interacting with a window.
//
// The video subsystem is also interested in abstracting away some
// platform-specific differences in OpenGL: context creation, swapping
// buffers, etc. This may be crucial to your app, but also you are not
// required to use OpenGL at all. In fact, SDL can provide rendering to those
// windows as well, either with an easy-to-use
// [2D API](https://wiki.libsdl.org/SDL3/CategoryRender)
// or with a more-powerful
// [GPU API](https://wiki.libsdl.org/SDL3/CategoryGPU)
// . Of course, it can simply get out of your way and give you the window
// handles you need to use Vulkan, Direct3D, Metal, or whatever else you like
// directly, too.
//
// The video subsystem covers a lot of functionality, out of necessity, so it
// is worth perusing the list of functions just to see what's available, but
// most apps can get by with simply creating a window and listening for
// events, so start with SDL_CreateWindow() and SDL_PollEvent().
namespace slickdl::video {

namespace display {

// This is a unique ID for a display for the time it is connected to the
// system, and is never reused for the lifetime of the application.
//
// If the display is disconnected and reconnected, it will get a new ID.
//
// The value 0 is an invalid ID.
using id_t = uint32_t;

// The pointer to the global `wl_display` object used by the Wayland video
// backend.
//
// Can be set before the video subsystem is initialized to import an external
// `wl_display` object from an application or toolkit for use in SDL, or read
// after initialization to export the `wl_display` used by the Wayland video
// backend. Setting this property after the video subsystem has been
// initialized has no effect, and reading it when the video subsystem is
// uninitialized will either return the user provided value, if one was set
// prior to initialization, or NULL. See docs/README-wayland.md for more
// information.
constexpr std::string_view g_videoWaylandWlDisplayPointer =
    "SDL.video.wayland.wl_display";

// Internal display mode data.
//
// This lives as a field in SDL_DisplayMode, as opaque data.
using modeData_t = gsl::not_null< SDL_DisplayModeData* >;

// The structure that defines a display mode.
using mode_t = SDL_DisplayMode;

// Display orientation values; the way a display is rotated.
using orientation_t = enum class orientation : uint8_t {
    unknown,          /**< The display orientation can't be determined */
    landscape,        /**< The display is in landscape mode, with the
                         right side up, relative to portrait mode */
    landscapeFlipped, /**< The display is in landscape mode,
                          with the left side up, relative to
                          portrait mode */
    portrait,         /**< The display is in portrait mode */
    portraitFlipped,  /**< The display is in portrait mode,
                                         upside down */
};

using orientationUnderlying_t = std::underlying_type_t< orientation_t >;

[[nodiscard]] constexpr auto toLegacy( orientation_t _value )
    -> SDL_DisplayOrientation {
    return ( static_cast< SDL_DisplayOrientation >( _value ) );
}

[[nodiscard]] constexpr auto toLegacy( orientation_t* _value )
    -> SDL_DisplayOrientation* {
    return ( std::bit_cast< SDL_DisplayOrientation* >( _value ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_DisplayOrientation _value )
    -> orientation_t {
    return ( static_cast< orientation_t >( _value ) );
}

// Get a list of currently connected displays.
//
// Should only be called on the main thread.
[[nodiscard]] auto all() -> std::vector< id_t >;

// Return the primary display.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto primary() -> id_t {
    const id_t l_result = SDL_GetPrimaryDisplay();

    assert( l_result );

    return ( l_result );
}

// Get the properties associated with a display.
//
// The following read-only properties are provided by SDL:
//
// - `SDL_PROP_DISPLAY_HDR_ENABLED_BOOLEAN`: true if the display has HDR
//   headroom above the SDR white point. This is for informational and
//   diagnostic purposes only, as not all platforms provide this information
//   at the display level.
//
// On KMS/DRM:
//
// - `SDL_PROP_DISPLAY_KMSDRM_PANEL_ORIENTATION_NUMBER`: the "panel
//   orientation" property for the display in degrees of clockwise rotation.
//   Note that this is provided only as a hint, and the application is
//   responsible for any coordinate transformations needed to conform to the
//   requested display orientation.
//
// On Wayland:
//
// - `SDL_PROP_DISPLAY_WAYLAND_WL_OUTPUT_POINTER`: the wl_output associated
//   with the display
//
// Should only be called on the main thread.
[[nodiscard]] inline auto properties( id_t _id ) -> properties::id_t {
    const properties::id_t l_result = SDL_GetDisplayProperties( _id );

    assert( l_result );

    return ( l_result );
}

constexpr std::string_view g_enabledHDRBoolean = "SDL.display.HDR_enabled";
constexpr std::string_view g_panelOrientationKMSDRMNumber =
    "SDL.display.KMSDRM.panel_orientation";
constexpr std::string_view g_waylandWlOutputPointer =
    "SDL.display.wayland.wl_output";

// Get the name of a display in UTF-8 encoding.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto name( id_t _id ) -> std::string_view {
    return { gsl::make_not_null( SDL_GetDisplayName( _id ) ) };
}

// Get the desktop area represented by a display.
//
// The primary display is often located at (0,0), but may be placed at a
// different location depending on monitor layout.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto bounds( id_t _id ) -> box_t< int > {
    SDL_Rect l_box;

    const bool l_result = SDL_GetDisplayBounds( _id, &l_box );

    assert( l_result );

    return { l_box };
}

// Get the usable desktop area represented by a display, in screen
// coordinates.
//
// This is the same area as SDL_GetDisplayBounds() reports, but with portions
// reserved by the system removed. For example, on Apple's macOS, this
// subtracts the area occupied by the menu bar and dock.
//
// Setting a window to be fullscreen generally bypasses these unusable areas,
// so these are good guidelines for the maximum space available to a
// non-fullscreen window.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto usableBounds( id_t _id ) -> box_t< int > {
    SDL_Rect l_box;

    const bool l_result = SDL_GetDisplayUsableBounds( _id, &l_box );

    assert( l_result );

    return { l_box };
}

// Get the orientation of a display when it is unrotated.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto naturalOrientation( id_t _id ) -> orientation_t {
    const orientation_t l_result =
        fromLegacy( SDL_GetNaturalDisplayOrientation( _id ) );

    assert( l_result != orientation_t::unknown );

    return ( l_result );
}

// Get the orientation of a display.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto currentOrientation( id_t _id ) -> orientation_t {
    const orientation_t l_result =
        fromLegacy( SDL_GetCurrentDisplayOrientation( _id ) );

    assert( l_result != orientation_t::unknown );

    return ( l_result );
}

// Get the content scale of a display.
//
// The content scale is the expected scale for content based on the DPI
// settings of the display. For example, a 4K display might have a 2.0 (200%)
// display scale, which means that the user expects UI elements to be twice as
// big on this display, to aid in readability.
//
// After window creation, SDL_GetWindowDisplayScale() should be used to query
// the content scale factor for individual windows instead of querying the
// display for a window and calling this function, as the per-window content
// scale factor may differ from the base value of the display it is on,
// particularly on high-DPI and/or multi-monitor desktop configurations.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto contentScale( id_t _id ) -> float {
    const float l_result = SDL_GetDisplayContentScale( _id );

    assert( l_result );

    return ( l_result );
}

// Get a list of fullscreen display modes available on a display.
//
// The display modes are sorted in this priority:
//
// - w -> largest to smallest
// - h -> largest to smallest
// - bits per pixel -> more colors to fewer colors
// - packed pixel layout -> largest to smallest
// - refresh rate -> highest to lowest
// - pixel density -> lowest to highest
//
// Should only be called on the main thread.
[[nodiscard]] auto fullscreenModes( id_t _id )
    -> std::vector< gsl::not_null< mode_t* > >;

// Get the closest match to the requested display mode.
//
// The available display modes are scanned and `closest` is filled in with the
// closest mode matching the requested mode and returned. The mode format and
// refresh rate default to the desktop mode if they are set to 0. The modes
// are scanned with size being first priority, format being second priority,
// and finally checking the refresh rate. If all the available modes are too
// small, then false is returned.
//
// Should only be called on the main thread.
inline void closestFullscreenMode( id_t _id,
                                   volume_t< int > _volume,
                                   float _refreshRate = 0,
                                   bool _includeHighDensityModes = false ) {
    mode_t l_closest;

    const bool l_result = SDL_GetClosestFullscreenDisplayMode(
        _id, _volume.width, _volume.height, _refreshRate,
        _includeHighDensityModes, &l_closest );

    assert( l_result );
}

// Get information about the desktop's display mode.
//
// There's a difference between this function and SDL_GetCurrentDisplayMode()
// when SDL runs fullscreen and has changed the resolution. In that case this
// function will return the previous native display mode, and not the current
// display mode.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto mode( id_t _id ) -> gsl::not_null< const mode_t* > {
    return ( SDL_GetDesktopDisplayMode( _id ) );
}

// Get information about the current display mode.
//
// There's a difference between this function and SDL_GetDesktopDisplayMode()
// when SDL runs fullscreen and has changed the resolution. In that case this
// function will return the current display mode, and not the previous native
// display mode.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto currentMode( id_t _id )
    -> gsl::not_null< const mode_t* > {
    return ( SDL_GetCurrentDisplayMode( _id ) );
}

// Get the display containing a point.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto get( const point_t< int >& _point ) -> id_t {
    const id_t l_result = SDL_GetDisplayForPoint( _point );

    assert( l_result );

    return ( l_result );
}

// Get the display primarily containing a rect.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto get( const box_t< int >& _box ) -> id_t {
    const id_t l_result = SDL_GetDisplayForRect( _box );

    assert( l_result );

    return ( l_result );
}

// Get the display associated with a window.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto get( window_t _window ) -> id_t {
    const id_t l_result = SDL_GetDisplayForWindow( _window );

    assert( l_result );

    return ( l_result );
}

} // namespace display

namespace window {

// This is a unique ID for a window.
//
// The value 0 is an invalid ID.
using id_t = uint32_t;

// The struct used as an opaque handle to a window.
#if 0
using window_t = SDL_Window;
#endif

// The flags on a window.
//
// These cover a lot of true/false, or on/off, window state. Some of it is
// immutable after being set through SDL_CreateWindow(), some of it can be
// changed on existing windows by the app, and some of it might be altered by
// the user or system outside of the app's control.
// TODO: Maybe uint64_t is a must be
using flags_t = enum class flags : uint32_t {
    fullscreen = 0x1,           /**< window is in fullscreen mode */
    openGL = 0x2,               /**< window usable with OpenGL context */
    occluded = 0x4,             /**< window is occluded */
    hidden = 0x8,               /**< window is neither mapped onto the desktop
                                                  nor shown in the taskbar/dock/window list;
                                                  SDL_ShowWindow( is required for it to become
                                                  visible */
    borderless = 0x10,          /**< no window decoration */
    resizable = 0x20,           /**< window can be resized */
    minimized = 0x40,           /**< window is minimized */
    maximized = 0x80,           /**< window is maximized */
    mouseGrabbed = 0x100,       /**< window has grabbed mouse input */
    inputFocus = 0x200,         /**< window has input focus */
    mouseFocus = 0x400,         /**< window has mouse focus */
    external = 0x800,           /**< window not created by SDL */
    modal = 0x1000,             /**< window is modal */
    highPixelDensity = 0x2000,  /**< window uses high pixel density
                                               back buffer if possible */
    mouseCapture = 0x4000,      /**< window has mouse captured (unrelated
                                               to MOUSE_GRABBED */
    mouseRelativeMode = 0x8000, /**< window has relative mode enabled */
    alwaysOnTop = 0x10000,      /**< window should always be above others */
    utility =
        0x20000,         /**< window should be treated as a utility window,
                                       not showing in the task bar and window list */
    tooltip = 0x40000,   /**< window should be treated as a tooltip and
                                       does not get mouse or keyboard focus,
                                       requires a parent window */
    popupMenu = 0x80000, /**< window should be treated as a popup
                                       menu, requires a parent window */
    keyboardGrabbed = 0x100000, /**< window has grabbed keyboard input */
    vulkan = 0x10000000,        /**< window usable for Vulkan surface */
    metal = 0x20000000,         /**< window usable for Metal view */
    transparent = 0x40000000,   /**< window with transparent buffer */
    notFocusable = 0x80000000,  /**< window should not be focusable */
};

using flagsUnderlying_t = std::underlying_type_t< flags_t >;

[[nodiscard]] constexpr auto toLegacy( flags_t _value ) -> SDL_WindowFlags {
    return ( static_cast< SDL_WindowFlags >( _value ) );
}

[[nodiscard]] constexpr auto toLegacy( flags_t* _value ) -> SDL_WindowFlags* {
    return ( std::bit_cast< SDL_WindowFlags* >( _value ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_WindowFlags _value ) -> flags_t {
    return ( static_cast< flags_t >( _value ) );
}

// A magic value used with SDL_WINDOWPOS_UNDEFINED.
//
// Generally this macro isn't used directly, but rather through
// SDL_WINDOWPOS_UNDEFINED or SDL_WINDOWPOS_UNDEFINED_DISPLAY.
constexpr size_t g_undefinedPositionMask = 0x1FFF0000u;

// Used to indicate that you don't care what the window position is.
//
// If you _really_ don't care, SDL_WINDOWPOS_UNDEFINED is the same, but always
// uses the primary display instead of specifying one.
// FIX: What is this?
[[nodiscard]] constexpr auto makePositionUndefined( auto _x ) {
    return ( g_undefinedPositionMask | _x );
}

// Used to indicate that you don't care what the window position/display is.
//
// This always uses the primary display.
constexpr auto g_undefinedPosition = makePositionUndefined( 0 );

// A macro to test if the window position is marked as "undefined."
//
// \param X the window position value.
[[nodiscard]] constexpr auto isPositionUndefined( auto _x ) -> bool {
    ( ( _x & 0xFFFF0000 ) == g_undefinedPositionMask );
}

// A magic value used with SDL_WINDOWPOS_CENTERED.
//
// Generally this macro isn't used directly, but rather through
// SDL_WINDOWPOS_CENTERED or SDL_WINDOWPOS_CENTERED_DISPLAY.
constexpr auto g_centeredPositionMask = 0x2FFF0000U;

// Used to indicate that the window position should be centered.
//
// SDL_WINDOWPOS_CENTERED is the same, but always uses the primary display
// instead of specifying one.
[[nodiscard]] constexpr auto makePositionCentered( auto _x ) {
    return ( g_centeredPositionMask | _x );
}

// Used to indicate that the window position should be centered.
//
// This always uses the primary display.
constexpr auto g_centeredPosition = makePositionCentered( 0 );

// A macro to test if the window position is marked as "centered."
[[nodiscard]] constexpr auto isPositionCentered( auto _x ) {
    return ( ( _x & 0xFFFF0000 ) == SDL_WINDOWPOS_CENTERED_MASK );
}

// Window flash operation.
using flashOperation_t = enum class flashOperation : uint8_t {
    cancel,       /**< Cancel any window flash state */
    briefly,      /**< Flash the window briefly to get attention */
    untilFocused, /**< Flash the window until it gets focus */
};

using flashOperationUnderlying_t = std::underlying_type_t< flashOperation_t >;

[[nodiscard]] constexpr auto toLegacy( flashOperation_t _value )
    -> SDL_FlashOperation {
    return ( static_cast< SDL_FlashOperation >( _value ) );
}

[[nodiscard]] constexpr auto toLegacy( flashOperation_t* _value )
    -> SDL_FlashOperation* {
    return ( std::bit_cast< SDL_FlashOperation* >( _value ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_FlashOperation _value )
    -> flashOperation_t {
    return ( static_cast< flashOperation_t >( _value ) );
}

// Window progress state
using progressState_t = enum class progressState : int8_t {
    invalid = -1,  /**< An invalid progress state indicating an error; check
                      SDL_GetError() */
    none,          /**< No progress bar is shown */
    indeterminate, /**< The progress bar is shown in a indeterminate state */
    normal,        /**< The progress bar is shown in a normal state */
    paused,        /**< The progress bar is shown in a paused state */
    error, /**< The progress bar is shown in a state indicating the application
              had an error */
};

using progressStateUnderlying_t = std::underlying_type_t< progressState_t >;

[[nodiscard]] constexpr auto toLegacy( progressState_t _value )
    -> SDL_ProgressState {
    return ( static_cast< SDL_ProgressState >( _value ) );
}

[[nodiscard]] constexpr auto toLegacy( progressState_t* _value )
    -> SDL_ProgressState* {
    return ( std::bit_cast< SDL_ProgressState* >( _value ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_ProgressState _value )
    -> progressState_t {
    return ( static_cast< progressState_t >( _value ) );
}

// Get the pixel density of a window.
//
// This is a ratio of pixel size to window size. For example, if the window is
// 1920x1080 and it has a high density back buffer of 3840x2160 pixels, it
// would have a pixel density of 2.0.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto pixelDensity( window_t _window ) -> float {
    const float l_result = SDL_GetWindowPixelDensity( _window );

    assert( l_result );

    return ( l_result );
}

// Get the content display scale relative to a window's pixel size.
//
// This is a combination of the window pixel density and the display content
// scale, and is the expected scale for displaying content in this window. For
// example, if a 3840x2160 window had a display scale of 2.0, the user expects
// the content to take twice as many pixels and be the same physical size as
// if it were being displayed in a 1920x1080 window with a display scale of
// 1.0.
//
// Conceptually this value corresponds to the scale display setting, and is
// updated when that setting is changed, or the window moves to a display with
// a different scale setting.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto displayScale( window_t _window ) -> float {
    const float l_result = SDL_GetWindowDisplayScale( _window );

    assert( l_result );

    return ( l_result );
}

// Set the display mode to use when a window is visible and fullscreen.
//
// This only affects the display mode used when the window is fullscreen. To
// change the window size when the window is not fullscreen, use
// SDL_SetWindowSize().
//
// If the window is currently in the fullscreen state, this request is
// asynchronous on some windowing systems and the new mode dimensions may not
// be applied immediately upon the return of this function. If an immediate
// change is required, call SDL_SyncWindow() to block until the changes have
// taken effect.
//
// When the new mode takes effect, an SDL_EVENT_WINDOW_RESIZED and/or an
// SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED event will be emitted with the new mode
// dimensions.
//
// Should only be called on the main thread.
inline void fullscreenMode(
    window_t _window,
    const std::optional< display::mode_t >& _mode = std::nullopt ) {
    bool l_result = false;

    if ( _mode ) {
        l_result = SDL_SetWindowFullscreenMode( _window, &_mode.value() );

    } else {
        l_result = SDL_SetWindowFullscreenMode( _window, nullptr );
    }

    assert( l_result );
}

// Query the display mode to use when a window is visible at fullscreen.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto fullscreenMode( window_t _window )
    -> std::optional< gsl::not_null< const display::mode_t* > > {
    const display::mode_t* l_mode = SDL_GetWindowFullscreenMode( _window );

    if ( l_mode ) {
        return ( l_mode );

    } else {
        return ( std::nullopt );
    }
}

// Get the raw ICC profile data for the screen the window is currently on.
//
// Should only be called on the main thread.
[[nodiscard]] auto getWindowICCProfile( window_t _window )
    -> std::vector< std::byte >;

// Get the pixel format associated with the window.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto pixelFormat( window_t _window ) -> pixels::format_t {
    const pixels::format_t l_result =
        pixels::fromLegacy( SDL_GetWindowPixelFormat( _window ) );

    assert( l_result != pixels::format_t::unknown );

    return ( l_result );
}

// Get a list of valid windows.
//
// Should only be called on the main thread.
[[nodiscard]] auto all() -> std::vector< window_t >;

// Create a window with the specified dimensions and flags.
//
// The window size is a request and may be different than expected based on
// the desktop layout and window manager policies. Your application should be
// prepared to handle a window of any size.
//
// `flags` may be any of the following OR'd together:
//
// - `SDL_WINDOW_FULLSCREEN`: fullscreen window at desktop resolution
// - `SDL_WINDOW_OPENGL`: window usable with an OpenGL context
// - `SDL_WINDOW_OCCLUDED`: window partially or completely obscured by another
//   window
// - `SDL_WINDOW_HIDDEN`: window is not visible
// - `SDL_WINDOW_BORDERLESS`: no window decoration
// - `SDL_WINDOW_RESIZABLE`: window can be resized
// - `SDL_WINDOW_MINIMIZED`: window is minimized
// - `SDL_WINDOW_MAXIMIZED`: window is maximized
// - `SDL_WINDOW_MOUSE_GRABBED`: window has grabbed mouse focus
// - `SDL_WINDOW_INPUT_FOCUS`: window has input focus
// - `SDL_WINDOW_MOUSE_FOCUS`: window has mouse focus
// - `SDL_WINDOW_EXTERNAL`: window not created by SDL
// - `SDL_WINDOW_MODAL`: window is modal
// - `SDL_WINDOW_HIGH_PIXEL_DENSITY`: window uses high pixel density back
//   buffer if possible
// - `SDL_WINDOW_MOUSE_CAPTURE`: window has mouse captured (unrelated to
//   MOUSE_GRABBED)
// - `SDL_WINDOW_ALWAYS_ON_TOP`: window should always be above others
// - `SDL_WINDOW_UTILITY`: window should be treated as a utility window, not
//   showing in the task bar and window list
// - `SDL_WINDOW_TOOLTIP`: window should be treated as a tooltip and does not
//   get mouse or keyboard focus, requires a parent window
// - `SDL_WINDOW_POPUP_MENU`: window should be treated as a popup menu,
//   requires a parent window
// - `SDL_WINDOW_KEYBOARD_GRABBED`: window has grabbed keyboard input
// - `SDL_WINDOW_VULKAN`: window usable with a Vulkan instance
// - `SDL_WINDOW_METAL`: window usable with a Metal instance
// - `SDL_WINDOW_TRANSPARENT`: window with transparent buffer
// - `SDL_WINDOW_NOT_FOCUSABLE`: window should not be focusable
//
// The SDL_Window is implicitly shown if SDL_WINDOW_HIDDEN is not set.
//
// On Apple's macOS, you **must** set the NSHighResolutionCapable Info.plist
// property to YES, otherwise you will not receive a High-DPI OpenGL canvas.
//
// The window pixel size may differ from its window coordinate size if the
// window is on a high pixel density display. Use SDL_GetWindowSize() to query
// the client area's size in window coordinates, and
// SDL_GetWindowSizeInPixels() or SDL_GetRenderOutputSize() to query the
// drawable size in pixels. Note that the drawable size can vary after the
// window is created and should be queried again if you get an
// SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED event.
//
// If the window is created with any of the SDL_WINDOW_OPENGL or
// SDL_WINDOW_VULKAN flags, then the corresponding LoadLibrary function
// (SDL_GL_LoadLibrary or SDL_Vulkan_LoadLibrary) is called and the
// corresponding UnloadLibrary function is called by SDL_DestroyWindow().
//
// If SDL_WINDOW_VULKAN is specified and there isn't a working Vulkan driver,
// SDL_CreateWindow() will fail, because SDL_Vulkan_LoadLibrary() will fail.
//
// If SDL_WINDOW_METAL is specified on an OS that does not support Metal,
// SDL_CreateWindow() will fail.
//
// If you intend to use this window with an SDL_Renderer, you should use
// SDL_CreateWindowAndRenderer() instead of this function, to avoid window
// flicker.
//
// On non-Apple devices, SDL requires you to either not link to the Vulkan
// loader or link to a dynamic library version. This limitation may be removed
// in a future version of SDL.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto create( std::string_view _title,
                                  volume_t< int > _volume,
                                  flags_t _flags ) -> window_t {
    return ( SDL_CreateWindow( std::string( _title ).c_str(), _volume.width,
                               _volume.height, toLegacy( _flags ) ) );
}

// Create a child popup window of the specified parent window.
//
// The window size is a request and may be different than expected based on
// the desktop layout and window manager policies. Your application should be
// prepared to handle a window of any size.
//
// The flags parameter **must** contain at least one of the following:
//
// - `SDL_WINDOW_TOOLTIP`: The popup window is a tooltip and will not pass any
//   input events.
// - `SDL_WINDOW_POPUP_MENU`: The popup window is a popup menu. The topmost
//   popup menu will implicitly gain the keyboard focus.
//
// The following flags are not relevant to popup window creation and will be
// ignored:
//
// - `SDL_WINDOW_MINIMIZED`
// - `SDL_WINDOW_MAXIMIZED`
// - `SDL_WINDOW_FULLSCREEN`
// - `SDL_WINDOW_BORDERLESS`
//
// The following flags are incompatible with popup window creation and will
// cause it to fail:
//
// - `SDL_WINDOW_UTILITY`
// - `SDL_WINDOW_MODAL`
//
// The parent parameter **must** be non-null and a valid window. The parent of
// a popup window can be either a regular, toplevel window, or another popup
// window.
//
// Popup windows cannot be minimized, maximized, made fullscreen, raised,
// flash, be made a modal window, be the parent of a toplevel window, or grab
// the mouse and/or keyboard. Attempts to do so will fail.
//
// Popup windows implicitly do not have a border/decorations and do not appear
// on the taskbar/dock or in lists of windows such as alt-tab menus.
//
// By default, popup window positions will automatically be constrained to
// keep the entire window within display bounds. This can be overridden with
// the `SDL_PROP_WINDOW_CREATE_CONSTRAIN_POPUP_BOOLEAN` property.
//
// By default, popup menus will automatically grab keyboard focus from the
// parent when shown. This behavior can be overridden by setting the
// `SDL_WINDOW_NOT_FOCUSABLE` flag, setting the
// `SDL_PROP_WINDOW_CREATE_FOCUSABLE_BOOLEAN` property to false, or toggling
// it after creation via the `SDL_SetWindowFocusable()` function.
//
// If a parent window is hidden or destroyed, any child popup windows will be
// recursively hidden or destroyed as well. Child popup windows not explicitly
// hidden will be restored when the parent is shown.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto createPopup( window_t _parent,
                                       point_t< int > _offset,
                                       volume_t< int > _volume,
                                       flags_t _flags ) -> window_t {
    return ( SDL_CreatePopupWindow( _parent, _offset.x, _offset.y,
                                    _volume.width, _volume.height,
                                    toLegacy( _flags ) ) );
}

// Create a window with the specified properties.
//
// The window size is a request and may be different than expected based on
// the desktop layout and window manager policies. Your application should be
// prepared to handle a window of any size.
//
// These are the supported properties:
//
// - `SDL_PROP_WINDOW_CREATE_ALWAYS_ON_TOP_BOOLEAN`: true if the window should
//   be always on top
// - `SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN`: true if the window has no
//   window decoration
// - `SDL_PROP_WINDOW_CREATE_CONSTRAIN_POPUP_BOOLEAN`: true if the "tooltip"
//   and "menu" window types should be automatically constrained to be
//   entirely within display bounds (default), false if no constraints on the
//   position are desired.
// - `SDL_PROP_WINDOW_CREATE_EXTERNAL_GRAPHICS_CONTEXT_BOOLEAN`: true if the
//   window will be used with an externally managed graphics context.
// - `SDL_PROP_WINDOW_CREATE_FOCUSABLE_BOOLEAN`: true if the window should
//   accept keyboard input (defaults true)
// - `SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN`: true if the window should
//   start in fullscreen mode at desktop resolution
// - `SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER`: the height of the window
// - `SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN`: true if the window should start
//   hidden
// - `SDL_PROP_WINDOW_CREATE_HIGH_PIXEL_DENSITY_BOOLEAN`: true if the window
//   uses a high pixel density buffer if possible
// - `SDL_PROP_WINDOW_CREATE_MAXIMIZED_BOOLEAN`: true if the window should
//   start maximized
// - `SDL_PROP_WINDOW_CREATE_MENU_BOOLEAN`: true if the window is a popup menu
// - `SDL_PROP_WINDOW_CREATE_METAL_BOOLEAN`: true if the window will be used
//   with Metal rendering
// - `SDL_PROP_WINDOW_CREATE_MINIMIZED_BOOLEAN`: true if the window should
//   start minimized
// - `SDL_PROP_WINDOW_CREATE_MODAL_BOOLEAN`: true if the window is modal to
//   its parent
// - `SDL_PROP_WINDOW_CREATE_MOUSE_GRABBED_BOOLEAN`: true if the window starts
//   with grabbed mouse focus
// - `SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN`: true if the window will be used
//   with OpenGL rendering
// - `SDL_PROP_WINDOW_CREATE_PARENT_POINTER`: an SDL_Window that will be the
//   parent of this window, required for windows with the "tooltip", "menu",
//   and "modal" properties
// - `SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN`: true if the window should be
//   resizable
// - `SDL_PROP_WINDOW_CREATE_TITLE_STRING`: the title of the window, in UTF-8
//   encoding
// - `SDL_PROP_WINDOW_CREATE_TRANSPARENT_BOOLEAN`: true if the window show
//   transparent in the areas with alpha of 0
// - `SDL_PROP_WINDOW_CREATE_TOOLTIP_BOOLEAN`: true if the window is a tooltip
// - `SDL_PROP_WINDOW_CREATE_UTILITY_BOOLEAN`: true if the window is a utility
//   window, not showing in the task bar and window list
// - `SDL_PROP_WINDOW_CREATE_VULKAN_BOOLEAN`: true if the window will be used
//   with Vulkan rendering
// - `SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER`: the width of the window
// - `SDL_PROP_WINDOW_CREATE_X_NUMBER`: the x position of the window, or
//   `SDL_WINDOWPOS_CENTERED`, defaults to `SDL_WINDOWPOS_UNDEFINED`. This is
//   relative to the parent for windows with the "tooltip" or "menu" property
//   set.
// - `SDL_PROP_WINDOW_CREATE_Y_NUMBER`: the y position of the window, or
//   `SDL_WINDOWPOS_CENTERED`, defaults to `SDL_WINDOWPOS_UNDEFINED`. This is
//   relative to the parent for windows with the "tooltip" or "menu" property
//   set.
//
// These are additional supported properties on macOS:
//
// - `SDL_PROP_WINDOW_CREATE_COCOA_WINDOW_POINTER`: the
//   `(__unsafe_unretained)` NSWindow associated with the window, if you want
//   to wrap an existing window.
// - `SDL_PROP_WINDOW_CREATE_COCOA_VIEW_POINTER`: the `(__unsafe_unretained)`
//   NSView associated with the window, defaults to `[window contentView]`
//
// These are additional supported properties on Wayland:
//
// - `SDL_PROP_WINDOW_CREATE_WAYLAND_SURFACE_ROLE_CUSTOM_BOOLEAN` - true if
//   the application wants to use the Wayland surface for a custom role and
//   does not want it attached to an XDG toplevel window. See
//   [README-wayland](README-wayland) for more information on using custom
//   surfaces.
// - `SDL_PROP_WINDOW_CREATE_WAYLAND_CREATE_EGL_WINDOW_BOOLEAN` - true if the
//   application wants an associated `wl_egl_window` object to be created and
//   attached to the window, even if the window does not have the OpenGL
//   property or `SDL_WINDOW_OPENGL` flag set.
// - `SDL_PROP_WINDOW_CREATE_WAYLAND_WL_SURFACE_POINTER` - the wl_surface
//   associated with the window, if you want to wrap an existing window. See
//   [README-wayland](README-wayland) for more information.
//
// These are additional supported properties on Windows:
//
// - `SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER`: the HWND associated with the
//   window, if you want to wrap an existing window.
// - `SDL_PROP_WINDOW_CREATE_WIN32_PIXEL_FORMAT_HWND_POINTER`: optional,
//   another window to share pixel format with, useful for OpenGL windows
//
// These are additional supported properties with X11:
//
// - `SDL_PROP_WINDOW_CREATE_X11_WINDOW_NUMBER`: the X11 Window associated
//   with the window, if you want to wrap an existing window.
//
// The window is implicitly shown if the "hidden" property is not set.
//
// These are additional supported properties with Emscripten:
//
// - `SDL_PROP_WINDOW_CREATE_EMSCRIPTEN_CANVAS_ID_STRING`: the id given to the
//   canvas element. This should start with a '#' sign
// - `SDL_PROP_WINDOW_CREATE_EMSCRIPTEN_KEYBOARD_ELEMENT_STRING`: override the
//   binding element for keyboard inputs for this canvas. The variable can be
//   one of:
// - "#window": the javascript window object (default)
// - "#document": the javascript document object
// - "#screen": the javascript window.screen object
// - "#canvas": the WebGL canvas element
// - "#none": Don't bind anything at all
// - any other string without a leading # sign applies to the element on the
//   page with that ID. Windows with the "tooltip" and "menu" properties are
//   popup windows and have the behaviors and guidelines outlined in
//   SDL_CreatePopupWindow().
//
// If this window is being created to be used with an SDL_Renderer, you should
// not add a graphics API specific property
// (`SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN`, etc), as SDL will handle that
// internally when it chooses a renderer. However, SDL might need to recreate
// your window at that point, which may cause the window to appear briefly,
// and then flicker as it is recreated. The correct approach to this is to
// create the window with the `SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN` property
// set to true, then create the renderer, then show the window with
// SDL_ShowWindow().
//
// Should only be called on the main thread.
[[nodiscard]] inline auto create( properties::id_t _properties ) -> window_t {
    return ( SDL_CreateWindowWithProperties( _properties ) );
}

namespace wproperties {

constexpr std::string_view g_createAlwaysOnTopBoolean =
    "SDL.window.create.always_on_top";
constexpr std::string_view g_createBorderlessBoolean =
    "SDL.window.create.borderless";
constexpr std::string_view g_createConstrainPopupBoolean =
    "SDL.window.create.constrain_popup";
constexpr std::string_view g_createFocusableBoolean =
    "SDL.window.create.focusable";
constexpr std::string_view g_createExternalGraphicsContextBoolean =
    "SDL.window.create.external_graphics_context";
constexpr std::string_view g_createFlagsNumber = "SDL.window.create.flags";
constexpr std::string_view g_createFullscreenBoolean =
    "SDL.window.create.fullscreen";
constexpr std::string_view g_createHeightNumber = "SDL.window.create.height";
constexpr std::string_view g_createHiddenBoolean = "SDL.window.create.hidden";
constexpr std::string_view g_createHighPixelDensityBoolean =
    "SDL.window.create.high_pixel_density";
constexpr std::string_view g_createMaximizedBoolean =
    "SDL.window.create.maximized";
constexpr std::string_view g_createMenuBoolean = "SDL.window.create.menu";
constexpr std::string_view g_createMetalBoolean = "SDL.window.create.metal";
constexpr std::string_view g_createMinimizedBoolean =
    "SDL.window.create.minimized";
constexpr std::string_view g_createModalBoolean = "SDL.window.create.modal";
constexpr std::string_view g_createMouseGrabbedBoolean =
    "SDL.window.create.mouse_grabbed";
constexpr std::string_view g_createOpenglBoolean = "SDL.window.create.opengl";
constexpr std::string_view g_createParentPointer = "SDL.window.create.parent";
constexpr std::string_view g_createResizableBoolean =
    "SDL.window.create.resizable";
constexpr std::string_view g_createTitleString = "SDL.window.create.title";
constexpr std::string_view g_createTransparentBoolean =
    "SDL.window.create.transparent";
constexpr std::string_view g_createTooltipBoolean = "SDL.window.create.tooltip";
constexpr std::string_view g_createUtilityBoolean = "SDL.window.create.utility";
constexpr std::string_view g_createVulkanBoolean = "SDL.window.create.vulkan";
constexpr std::string_view g_createWidthNumber = "SDL.window.create.width";
constexpr std::string_view g_createXNumber = "SDL.window.create.x";
constexpr std::string_view g_createYNumber = "SDL.window.create.y";
constexpr std::string_view g_createCocoaWindowPointer =
    "SDL.window.create.cocoa.window";
constexpr std::string_view g_createCocoaViewPointer =
    "SDL.window.create.cocoa.view";
constexpr std::string_view g_createWaylandSurfaceRoleCustomBoolean =
    "SDL.window.create.wayland.surface_role_custom";
constexpr std::string_view g_createWaylandCreateEglWindowBoolean =
    "SDL.window.create.wayland.create_egl_window";
constexpr std::string_view g_createWaylandWlSurfacePointer =
    "SDL.window.create.wayland.wl_surface";
constexpr std::string_view g_createWiN32HwndPointer =
    "SDL.window.create.win32.hwnd";
constexpr std::string_view g_createWiN32PixelFormatHwndPointer =
    "SDL.window.create.win32.pixel_format_hwnd";
constexpr std::string_view g_createX11WindowNumber =
    "SDL.window.create.x11.window";
constexpr std::string_view g_createEmscriptenCanvasIdString =
    "SDL.window.create.emscripten.canvas_id";
constexpr std::string_view g_createEmscriptenKeyboardElementString =
    "SDL.window.create.emscripten.keyboard_element";

} // namespace wproperties

// Get the numeric ID of a window.
//
// The numeric ID is what SDL_WindowEvent references, and is necessary to map
// these events to specific SDL_Window objects.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto id( window_t _window ) -> id_t {
    return ( SDL_GetWindowID( _window ) );
}

// Get a window from a stored ID.
//
// The numeric ID is what SDL_WindowEvent references, and is necessary to map
// these events to specific SDL_Window objects.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto window( id_t _id ) -> window_t {
    return ( SDL_GetWindowFromID( _id ) );
}

// Get parent of a window.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto parent( window_t _window ) -> window_t {
    return ( SDL_GetWindowParent( _window ) );
}

// Get the properties associated with a window.
//
// The following read-only properties are provided by SDL:
//
// - `SDL_PROP_WINDOW_SHAPE_POINTER`: the surface associated with a shaped
//   window
// - `SDL_PROP_WINDOW_HDR_ENABLED_BOOLEAN`: true if the window has HDR
//   headroom above the SDR white point. This property can change dynamically
//   when SDL_EVENT_WINDOW_HDR_STATE_CHANGED is sent.
// - `SDL_PROP_WINDOW_SDR_WHITE_LEVEL_FLOAT`: the value of SDR white in the
//   SDL_COLORSPACE_SRGB_LINEAR colorspace. On Windows this corresponds to the
//   SDR white level in scRGB colorspace, and on Apple platforms this is
//   always 1.0 for EDR content. This property can change dynamically when
//   SDL_EVENT_WINDOW_HDR_STATE_CHANGED is sent.
// - `SDL_PROP_WINDOW_HDR_HEADROOM_FLOAT`: the additional high dynamic range
//   that can be displayed, in terms of the SDR white point. When HDR is not
//   enabled, this will be 1.0. This property can change dynamically when
//   SDL_EVENT_WINDOW_HDR_STATE_CHANGED is sent.
//
// On Android:
//
// - `SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER`: the ANativeWindow associated
//   with the window
// - `SDL_PROP_WINDOW_ANDROID_SURFACE_POINTER`: the EGLSurface associated with
//   the window
//
// On iOS:
//
// - `SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER`: the `(__unsafe_unretained)`
//   UIWindow associated with the window
// - `SDL_PROP_WINDOW_UIKIT_METAL_VIEW_TAG_NUMBER`: the NSInteger tag
//   associated with metal views on the window
// - `SDL_PROP_WINDOW_UIKIT_OPENGL_FRAMEBUFFER_NUMBER`: the OpenGL view's
//   framebuffer object. It must be bound when rendering to the screen using
//   OpenGL.
// - `SDL_PROP_WINDOW_UIKIT_OPENGL_RENDERBUFFER_NUMBER`: the OpenGL view's
//   renderbuffer object. It must be bound when SDL_GL_SwapWindow is called.
// - `SDL_PROP_WINDOW_UIKIT_OPENGL_RESOLVE_FRAMEBUFFER_NUMBER`: the OpenGL
//   view's resolve framebuffer, when MSAA is used.
//
// On KMS/DRM:
//
// - `SDL_PROP_WINDOW_KMSDRM_DEVICE_INDEX_NUMBER`: the device index associated
//   with the window (e.g. the X in /dev/dri/cardX)
// - `SDL_PROP_WINDOW_KMSDRM_DRM_FD_NUMBER`: the DRM FD associated with the
//   window
// - `SDL_PROP_WINDOW_KMSDRM_GBM_DEVICE_POINTER`: the GBM device associated
//   with the window
//
// On macOS:
//
// - `SDL_PROP_WINDOW_COCOA_WINDOW_POINTER`: the `(__unsafe_unretained)`
//   NSWindow associated with the window
// - `SDL_PROP_WINDOW_COCOA_METAL_VIEW_TAG_NUMBER`: the NSInteger tag
//   assocated with metal views on the window
//
// On OpenVR:
//
// - `SDL_PROP_WINDOW_OPENVR_OVERLAY_ID_NUMBER`: the OpenVR Overlay Handle ID
//   for the associated overlay window.
//
// On Vivante:
//
// - `SDL_PROP_WINDOW_VIVANTE_DISPLAY_POINTER`: the EGLNativeDisplayType
//   associated with the window
// - `SDL_PROP_WINDOW_VIVANTE_WINDOW_POINTER`: the EGLNativeWindowType
//   associated with the window
// - `SDL_PROP_WINDOW_VIVANTE_SURFACE_POINTER`: the EGLSurface associated with
//   the window
//
// On Windows:
//
// - `SDL_PROP_WINDOW_WIN32_HWND_POINTER`: the HWND associated with the window
// - `SDL_PROP_WINDOW_WIN32_HDC_POINTER`: the HDC associated with the window
// - `SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER`: the HINSTANCE associated with
//   the window
//
// On Wayland:
//
// Note: The `xdg_*` window objects do not internally persist across window
// show/hide calls. They will be null if the window is hidden and must be
// queried each time it is shown.
//
// - `SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER`: the wl_display associated with
//   the window
// - `SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER`: the wl_surface associated with
//   the window
// - `SDL_PROP_WINDOW_WAYLAND_VIEWPORT_POINTER`: the wp_viewport associated
//   with the window
// - `SDL_PROP_WINDOW_WAYLAND_EGL_WINDOW_POINTER`: the wl_egl_window
//   associated with the window
// - `SDL_PROP_WINDOW_WAYLAND_XDG_SURFACE_POINTER`: the xdg_surface associated
//   with the window
// - `SDL_PROP_WINDOW_WAYLAND_XDG_TOPLEVEL_POINTER`: the xdg_toplevel role
//   associated with the window
// - 'SDL_PROP_WINDOW_WAYLAND_XDG_TOPLEVEL_EXPORT_HANDLE_STRING': the export
//   handle associated with the window
// - `SDL_PROP_WINDOW_WAYLAND_XDG_POPUP_POINTER`: the xdg_popup role
//   associated with the window
// - `SDL_PROP_WINDOW_WAYLAND_XDG_POSITIONER_POINTER`: the xdg_positioner
//   associated with the window, in popup mode
//
// On X11:
//
// - `SDL_PROP_WINDOW_X11_DISPLAY_POINTER`: the X11 Display associated with
//   the window
// - `SDL_PROP_WINDOW_X11_SCREEN_NUMBER`: the screen number associated with
//   the window
// - `SDL_PROP_WINDOW_X11_WINDOW_NUMBER`: the X11 Window associated with the
//   window
//
// On Emscripten:
//
// - `SDL_PROP_WINDOW_EMSCRIPTEN_CANVAS_ID_STRING`: the id the canvas element
//   will have
// - `SDL_PROP_WINDOW_EMSCRIPTEN_KEYBOARD_ELEMENT_STRING`: the keyboard
//   element that associates keyboard events to this window
//
// Should only be called on the main thread.
[[nodiscard]] inline auto properties( window_t _window ) -> properties::id_t {
    return ( SDL_GetWindowProperties( _window ) );
}

namespace wproperties {

constexpr std::string_view g_shapePointer = "SDL.window.shape";
constexpr std::string_view g_hdrEnabledBoolean = "SDL.window.HDR_enabled";
constexpr std::string_view g_sdrWhiteLevelFloat = "SDL.window.SDR_white_level";
constexpr std::string_view g_hdrHeadroomFloat = "SDL.window.HDR_headroom";
constexpr std::string_view g_androidWindowPointer = "SDL.window.android.window";
constexpr std::string_view g_androidSurfacePointer =
    "SDL.window.android.surface";
constexpr std::string_view g_uikitWindowPointer = "SDL.window.uikit.window";
constexpr std::string_view g_uikitMetalViewTagNumber =
    "SDL.window.uikit.metal_view_tag";
constexpr std::string_view g_uikitOpenglFramebufferNumber =
    "SDL.window.uikit.opengl.framebuffer";
constexpr std::string_view g_uikitOpenglRenderbufferNumber =
    "SDL.window.uikit.opengl.renderbuffer";
constexpr std::string_view g_uikitOpenglResolveFramebufferNumber =
    "SDL.window.uikit.opengl.resolve_framebuffer";
constexpr std::string_view g_kmsdrmDeviceIndexNumber =
    "SDL.window.kmsdrm.dev_index";
constexpr std::string_view g_kmsdrmDrmFdNumber = "SDL.window.kmsdrm.drm_fd";
constexpr std::string_view g_kmsdrmGbmDevicePointer =
    "SDL.window.kmsdrm.gbm_dev";
constexpr std::string_view g_cocoaWindowPointer = "SDL.window.cocoa.window";
constexpr std::string_view g_cocoaMetalViewTagNumber =
    "SDL.window.cocoa.metal_view_tag";
constexpr std::string_view g_openvrOverlayIdNumber =
    "SDL.window.openvr.overlay_id";
constexpr std::string_view g_vivanteDisplayPointer =
    "SDL.window.vivante.display";
constexpr std::string_view g_vivanteWindowPointer = "SDL.window.vivante.window";
constexpr std::string_view g_vivanteSurfacePointer =
    "SDL.window.vivante.surface";
constexpr std::string_view g_wiN32HwndPointer = "SDL.window.win32.hwnd";
constexpr std::string_view g_wiN32HdcPointer = "SDL.window.win32.hdc";
constexpr std::string_view g_wiN32InstancePointer = "SDL.window.win32.instance";
constexpr std::string_view g_waylandDisplayPointer =
    "SDL.window.wayland.display";
constexpr std::string_view g_waylandSurfacePointer =
    "SDL.window.wayland.surface";
constexpr std::string_view g_waylandViewportPointer =
    "SDL.window.wayland.viewport";
constexpr std::string_view g_waylandEglWindowPointer =
    "SDL.window.wayland.egl_window";
constexpr std::string_view g_waylandXdgSurfacePointer =
    "SDL.window.wayland.xdg_surface";
constexpr std::string_view g_waylandXdgToplevelPointer =
    "SDL.window.wayland.xdg_toplevel";
constexpr std::string_view g_waylandXdgToplevelExportHandleString =
    "SDL.window.wayland.xdg_toplevel_export_handle";
constexpr std::string_view g_waylandXdgPopupPointer =
    "SDL.window.wayland.xdg_popup";
constexpr std::string_view g_waylandXdgPositionerPointer =
    "SDL.window.wayland.xdg_positioner";
constexpr std::string_view g_x11DisplayPointer = "SDL.window.x11.display";
constexpr std::string_view g_x11ScreenNumber = "SDL.window.x11.screen";
constexpr std::string_view g_x11WindowNumber = "SDL.window.x11.window";
constexpr std::string_view g_emscriptenCanvasIdString =
    "SDL.window.emscripten.canvas_id";
constexpr std::string_view g_emscriptenKeyboardElementString =
    "SDL.window.emscripten.keyboard_element";

} // namespace wproperties

// Get the window flags.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto flags( window_t _window ) -> flags_t {
    return ( fromLegacy( SDL_GetWindowFlags( _window ) ) );
}

// Set the title of a window.
//
// This string is expected to be in UTF-8 encoding.
//
// Should only be called on the main thread.
inline void title( window_t _window, std::string_view _title ) {
    const bool l_result =
        SDL_SetWindowTitle( _window, std::string( _title ).c_str() );

    assert( l_result );
}

// Get the title of a window.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto title( window_t _window ) -> std::string_view {
    return { SDL_GetWindowTitle( _window ) };
}

// Set the icon for a window.
//
// If this function is passed a surface with alternate representations, the
// surface will be interpreted as the content to be used for 100% display
// scale, and the alternate representations will be used for high DPI
// situations. For example, if the original surface is 32x32, then on a 2x
// macOS display or 200% display scale on Windows, a 64x64 version of the
// image will be used, if available. If a matching version of the image isn't
// available, the closest larger size image will be downscaled to the
// appropriate size and be used instead, if available. Otherwise, the closest
// smaller image will be upscaled and be used instead.
//
// Should only be called on the main thread.
inline void icon( window_t _window, surface_t& _icon ) {
    const bool l_result = SDL_SetWindowIcon( _window, _icon );

    assert( l_result );
}

// Request that the window's position be set.
//
// If the window is in an exclusive fullscreen or maximized state, this
// request has no effect.
//
// This can be used to reposition fullscreen-desktop windows onto a different
// display, however, as exclusive fullscreen windows are locked to a specific
// display, they can only be repositioned programmatically via
// SDL_SetWindowFullscreenMode().
//
// On some windowing systems this request is asynchronous and the new
// coordinates may not have have been applied immediately upon the return of
// this function. If an immediate change is required, call SDL_SyncWindow() to
// block until the changes have taken effect.
//
// When the window position changes, an SDL_EVENT_WINDOW_MOVED event will be
// emitted with the window's new coordinates. Note that the new coordinates
// may not match the exact coordinates requested, as some windowing systems
// can restrict the position of the window in certain scenarios (e.g.
// constraining the position so the window is always within desktop bounds).
// Additionally, as this is just a request, it can be denied by the windowing
// system.
//
// Should only be called on the main thread.
inline void position( window_t _window, point_t< int > _position ) {
    const bool l_result =
        SDL_SetWindowPosition( _window, _position.x, _position.y );

    assert( l_result );
}

// Get the position of a window.
//
// This is the current position of the window as last reported by the
// windowing system.
//
// If you do not need the value for one of the positions a NULL may be passed
// in the `x` or `y` parameter.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto position( window_t _window ) -> point_t< int > {
    point_t< int > l_position;

    const bool l_result =
        SDL_GetWindowPosition( _window, &l_position.x, &l_position.y );

    assert( l_result );

    return ( l_position );
}

// Request that the size of a window's client area be set.
//
// If the window is in a fullscreen or maximized state, this request has no
// effect.
//
// To change the exclusive fullscreen mode of a window, use
// SDL_SetWindowFullscreenMode().
//
// On some windowing systems, this request is asynchronous and the new window
// size may not have have been applied immediately upon the return of this
// function. If an immediate change is required, call SDL_SyncWindow() to
// block until the changes have taken effect.
//
// When the window size changes, an SDL_EVENT_WINDOW_RESIZED event will be
// emitted with the new window dimensions. Note that the new dimensions may
// not match the exact size requested, as some windowing systems can restrict
// the window size in certain scenarios (e.g. constraining the size of the
// content area to remain within the usable desktop bounds). Additionally, as
// this is just a request, it can be denied by the windowing system.
//
// Should only be called on the main thread.
inline void volume( window_t _window, volume_t< int > _volume ) {
    const bool l_result =
        SDL_SetWindowSize( _window, _volume.width, _volume.height );

    assert( l_result );
}

// Get the size of a window's client area.
//
// The window pixel size may differ from its window coordinate size if the
// window is on a high pixel density display. Use SDL_GetWindowSizeInPixels()
// or SDL_GetRenderOutputSize() to get the real client area size in pixels.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto volume( window_t _window ) -> volume_t< int > {
    volume_t< int > l_volume;

    const bool l_result =
        SDL_GetWindowSize( _window, &l_volume.width, &l_volume.height );

    assert( l_result );

    return ( l_volume );
}

// Get the safe area for this window.
//
// Some devices have portions of the screen which are partially obscured or
// not interactive, possibly due to on-screen controls, curved edges, camera
// notches, TV overscan, etc. This function provides the area of the window
// which is safe to have interactable content. You should continue rendering
// into the rest of the window, but it should not contain visually important
// or interactible content.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto safeArea( window_t _window ) -> box_t< int > {
    box_t< int > l_box;

    const bool l_result = SDL_GetWindowSafeArea( _window, l_box );

    assert( l_result );

    return { l_box };
}

// Request that the aspect ratio of a window's client area be set.
//
// The aspect ratio is the ratio of width divided by height, e.g. 2560x1600
// would be 1.6. Larger aspect ratios are wider and smaller aspect ratios are
// narrower.
//
// If, at the time of this request, the window in a fixed-size state, such as
// maximized or fullscreen, the request will be deferred until the window
// exits this state and becomes resizable again.
//
// On some windowing systems, this request is asynchronous and the new window
// aspect ratio may not have have been applied immediately upon the return of
// this function. If an immediate change is required, call SDL_SyncWindow() to
// block until the changes have taken effect.
//
// When the window size changes, an SDL_EVENT_WINDOW_RESIZED event will be
// emitted with the new window dimensions. Note that the new dimensions may
// not match the exact aspect ratio requested, as some windowing systems can
// restrict the window size in certain scenarios (e.g. constraining the size
// of the content area to remain within the usable desktop bounds).
// Additionally, as this is just a request, it can be denied by the windowing
// system.
//
// Should only be called on the main thread.
inline void aspectRatio( window_t _window,
                         float _minAspect,
                         float _maxAspect ) {
    const bool l_result =
        SDL_SetWindowAspectRatio( _window, _minAspect, _maxAspect );

    assert( l_result );
}

// Get the size of a window's client area.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto aspectRatio( window_t _window )
    -> std::pair< float, float > {
    float l_minAspect = NAN;
    float l_maxAspect = NAN;

    const bool l_result =
        SDL_GetWindowAspectRatio( _window, &l_minAspect, &l_maxAspect );

    assert( l_result );

    return { l_minAspect, l_maxAspect };
}

// Get the size of a window's borders (decorations) around the client area.
//
// Note: If this function fails (returns false), the size values will be
// initialized to 0, 0, 0, 0 (if a non-NULL pointer is provided), as if the
// window in question was borderless.
//
// Note: This function may fail on systems where the window has not yet been
// decorated by the display server (for example, immediately after calling
// SDL_CreateWindow). It is recommended that you wait at least until the
// window has been presented and composited, so that the window system has a
// chance to decorate the window and provide the border dimensions to SDL.
//
// This function also returns false if getting the information is not
// supported.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto borders( window_t _window )
    -> std::tuple< int, int, int, int > {
    int l_top = 0;
    int l_left = 0;
    int l_bottom = 0;
    int l_right = 0;

    const bool l_result = SDL_GetWindowBordersSize( _window, &l_top, &l_left,
                                                    &l_bottom, &l_right );

    assert( l_result );

    return { l_top, l_left, l_bottom, l_right };
}

// Get the size of a window's client area, in pixels.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto volumeInPixels( window_t _window )
    -> volume_t< int > {
    volume_t< int > l_volume;

    const bool l_result =
        SDL_GetWindowSizeInPixels( _window, &l_volume.width, &l_volume.height );

    assert( l_result );

    return ( l_volume );
}

// Set the minimum size of a window's client area.
//
// Should only be called on the main thread.
inline void minimumVolume( window_t _window, volume_t< int > _volume ) {
    const bool l_result =
        SDL_SetWindowMinimumSize( _window, _volume.width, _volume.height );

    assert( l_result );
}

// Get the minimum size of a window's client area.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto minimumVolume( window_t _window ) -> volume_t< int > {
    volume_t< int > l_volume;

    const bool l_result =
        SDL_GetWindowMinimumSize( _window, &l_volume.width, &l_volume.height );

    assert( l_result );

    return ( l_volume );
}

// Set the maximum size of a window's client area.
//
// Should only be called on the main thread.
inline void maximumVolume( window_t _window, volume_t< int > _volume ) {
    const bool l_result =
        SDL_SetWindowMaximumSize( _window, _volume.width, _volume.height );

    assert( l_result );
}

// Get the maximum size of a window's client area.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto maximumVolume( window_t _window ) -> volume_t< int > {
    volume_t< int > l_volume;

    const bool l_result =
        SDL_GetWindowMaximumSize( _window, &l_volume.width, &l_volume.height );

    assert( l_result );

    return ( l_volume );
}

// Set the border state of a window.
//
// This will add or remove the window's `SDL_WINDOW_BORDERLESS` flag and add
// or remove the border from the actual window. This is a no-op if the
// window's border already matches the requested state.
//
// You can't change the border state of a fullscreen window.
//
// Should only be called on the main thread.
inline void bordered( window_t _window, bool _isBordered ) {
    const bool l_result = SDL_SetWindowBordered( _window, _isBordered );

    assert( l_result );
}

// Set the user-resizable state of a window.
//
// This will add or remove the window's `SDL_WINDOW_RESIZABLE` flag and
// allow/disallow user resizing of the window. This is a no-op if the window's
// resizable state already matches the requested state.
//
// You can't change the resizable state of a fullscreen window.
//
// Should only be called on the main thread.
inline void resizable( window_t _window, bool _isResizable ) {
    const bool l_result = SDL_SetWindowResizable( _window, _isResizable );

    assert( l_result );
}

// Set the window to always be above the others.
//
// This will add or remove the window's `SDL_WINDOW_ALWAYS_ON_TOP` flag. This
// will bring the window to the front and keep the window above the rest.
//
// Should only be called on the main thread.
inline void alwaysOnTop( window_t _window, bool _isOnTop ) {
    const bool l_result = SDL_SetWindowAlwaysOnTop( _window, _isOnTop );

    assert( l_result );
}

// Show a window.
//
// Should only be called on the main thread.
inline void show( window_t _window ) {
    const bool l_result = SDL_ShowWindow( _window );

    assert( l_result );
}

// Hide a window.
//
// Should only be called on the main thread.
inline void hide( window_t _window ) {
    const bool l_result = SDL_HideWindow( _window );

    assert( l_result );
}

// Request that a window be raised above other windows and gain the input
// focus.
//
// The result of this request is subject to desktop window manager policy,
// particularly if raising the requested window would result in stealing focus
// from another application. If the window is successfully raised and gains
// input focus, an SDL_EVENT_WINDOW_FOCUS_GAINED event will be emitted, and
// the window will have the SDL_WINDOW_INPUT_FOCUS flag set.
//
// Should only be called on the main thread.
inline void raise( window_t _window ) {
    const bool l_result = SDL_RaiseWindow( _window );

    assert( l_result );
}

// Request that the window be made as large as possible.
//
// Non-resizable windows can't be maximized. The window must have the
// SDL_WINDOW_RESIZABLE flag set, or this will have no effect.
//
// On some windowing systems this request is asynchronous and the new window
// state may not have have been applied immediately upon the return of this
// function. If an immediate change is required, call SDL_SyncWindow() to
// block until the changes have taken effect.
//
// When the window state changes, an SDL_EVENT_WINDOW_MAXIMIZED event will be
// emitted. Note that, as this is just a request, the windowing system can
// deny the state change.
//
// When maximizing a window, whether the constraints set via
// SDL_SetWindowMaximumSize() are honored depends on the policy of the window
// manager. Win32 and macOS enforce the constraints when maximizing, while X11
// and Wayland window managers may vary.
//
// Should only be called on the main thread.
inline void maximize( window_t _window ) {
    SDL_MaximizeWindow( _window );
}

// Request that the window be minimized to an iconic representation.
//
// If the window is in a fullscreen state, this request has no direct effect.
// It may alter the state the window is returned to when leaving fullscreen.
//
// On some windowing systems this request is asynchronous and the new window
// state may not have been applied immediately upon the return of this
// function. If an immediate change is required, call SDL_SyncWindow() to
// block until the changes have taken effect.
//
// When the window state changes, an SDL_EVENT_WINDOW_MINIMIZED event will be
// emitted. Note that, as this is just a request, the windowing system can
// deny the state change.
//
// Should only be called on the main thread.
inline void minimize( window_t _window ) {
    const bool l_result = SDL_MinimizeWindow( _window );

    assert( l_result );
}

// Request that the size and position of a minimized or maximized window be
// restored.
//
// If the window is in a fullscreen state, this request has no direct effect.
// It may alter the state the window is returned to when leaving fullscreen.
//
// On some windowing systems this request is asynchronous and the new window
// state may not have have been applied immediately upon the return of this
// function. If an immediate change is required, call SDL_SyncWindow() to
// block until the changes have taken effect.
//
// When the window state changes, an SDL_EVENT_WINDOW_RESTORED event will be
// emitted. Note that, as this is just a request, the windowing system can
// deny the state change.
//
// Should only be called on the main thread.
inline void restore( window_t _window ) {
    const bool l_result = SDL_RestoreWindow( _window );

    assert( l_result );
}

// Request that the window's fullscreen state be changed.
//
// By default a window in fullscreen state uses borderless fullscreen desktop
// mode, but a specific exclusive display mode can be set using
// SDL_SetWindowFullscreenMode().
//
// On some windowing systems this request is asynchronous and the new
// fullscreen state may not have have been applied immediately upon the return
// of this function. If an immediate change is required, call SDL_SyncWindow()
// to block until the changes have taken effect.
//
// When the window state changes, an SDL_EVENT_WINDOW_ENTER_FULLSCREEN or
// SDL_EVENT_WINDOW_LEAVE_FULLSCREEN event will be emitted. Note that, as this
// is just a request, it can be denied by the windowing system.
//
// Should only be called on the main thread.
inline void fullscreen( window_t _window, bool _isFullscreen ) {
    const bool l_result = SDL_SetWindowFullscreen( _window, _isFullscreen );

    assert( l_result );
}

// Block until any pending window state is finalized.
//
// On asynchronous windowing systems, this acts as a synchronization barrier
// for pending window state. It will attempt to wait until any pending window
// state has been applied and is guaranteed to return within finite time. Note
// that for how long it can potentially block depends on the underlying window
// system, as window state changes may involve somewhat lengthy animations
// that must complete before the window is in its final requested state.
//
// On windowing systems where changes are immediate, this does nothing.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto sync( window_t _window ) -> bool {
    return SDL_SyncWindow( _window );
}

// Return whether the window has a surface associated with it.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto hasSurface( window_t _window ) -> bool {
    return ( SDL_WindowHasSurface( _window ) );
}

// Get the SDL surface associated with the window.
//
// A new surface will be created with the optimal format for the window, if
// necessary. This surface will be freed when the window is destroyed. Do not
// free this surface.
//
// This surface will be invalidated if the window is resized. After resizing a
// window this function must be called again to return a valid surface.
//
// You may not combine this with 3D or the rendering API on this window.
//
// This function is affected by `SDL_HINT_FRAMEBUFFER_ACCELERATION`.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto surface( window_t _window ) -> surface_t {
    return ( SDL_GetWindowSurface( _window ) );
}

// Toggle VSync for the window surface.
//
// When a window surface is created, vsync defaults to
// SDL_WINDOW_SURFACE_VSYNC_DISABLED.
//
// The `vsync` parameter can be 1 to synchronize present with every vertical
// refresh, 2 to synchronize present with every second vertical refresh, etc.,
// SDL_WINDOW_SURFACE_VSYNC_ADAPTIVE for late swap tearing (adaptive vsync),
// or SDL_WINDOW_SURFACE_VSYNC_DISABLED to disable. Not every value is
// supported by every driver, so you should check the return value to see
// whether the requested setting is supported.
//
// Should only be called on the main thread.
inline void surfaceVSync( window_t _window, int _vsync ) {
    const bool l_result = SDL_SetWindowSurfaceVSync( _window, _vsync );

    assert( l_result );
}

constexpr int g_surfaceVsyncDisabled = 0;
constexpr int g_surfaceVsyncAdaptive = ( -1 );

// Get VSync for the window surface.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto surfaceVSync( window_t _window ) -> int {
    int l_vsync = 0;

    const bool l_result = SDL_GetWindowSurfaceVSync( _window, &l_vsync );

    assert( l_result );

    return ( l_vsync );
}

// Copy the window surface to the screen.
//
// This is the function you use to reflect any changes to the surface on the
// screen.
//
// This function is equivalent to the SDL 1.2 API SDL_Flip().
//
// Should only be called on the main thread.
inline void updateSurface( window_t _window ) {
    const bool l_result = SDL_UpdateWindowSurface( _window );

    assert( l_result );
}

// Copy areas of the window surface to the screen.
//
// This is the function you use to reflect changes to portions of the surface
// on the screen.
//
// This function is equivalent to the SDL 1.2 API SDL_UpdateRects().
//
// Note that this function will update _at least_ the rectangles specified,
// but this is only intended as an optimization; in practice, this might
// update more of the screen (or all of the screen!), depending on what method
// SDL uses to send pixels to the system.
//
// Should only be called on the main thread.
void updateSurface( window_t _window, std::span< const box_t< int > > _boxes );

// Destroy the surface associated with the window.
//
// Should only be called on the main thread.
inline void destroySurface( window_t _window ) {
    const bool l_result = SDL_DestroyWindowSurface( _window );

    assert( l_result );
}

// Set a window's keyboard grab mode.
//
// Keyboard grab enables capture of system keyboard shortcuts like Alt+Tab or
// the Meta/Super key. Note that not all system keyboard shortcuts can be
// captured by applications (one example is Ctrl+Alt+Del on Windows).
//
// This is primarily intended for specialized applications such as VNC clients
// or VM frontends. Normal games should not use keyboard grab.
//
// When keyboard grab is enabled, SDL will continue to handle Alt+Tab when the
// window is full-screen to ensure the user is not trapped in your
// application. If you have a custom keyboard shortcut to exit fullscreen
// mode, you may suppress this behavior with
// `SDL_HINT_ALLOW_ALT_TAB_WHILE_GRABBED`.
//
// If the caller enables a grab while another window is currently grabbed, the
// other window loses its grab in favor of the caller's window.
//
// Should only be called on the main thread.
inline void keyboardGrab( window_t _window, bool _isGrabbed ) {
    const bool l_result = SDL_SetWindowKeyboardGrab( _window, _isGrabbed );

    assert( l_result );
}

// Set a window's mouse grab mode.
//
// Mouse grab confines the mouse cursor to the window.
//
// Should only be called on the main thread.
inline void mouseGrab( window_t _window, bool _isGrabbed ) {
    const bool l_result = SDL_SetWindowMouseGrab( _window, _isGrabbed );

    assert( l_result );
}

// Get a window's keyboard grab mode.
[[nodiscard]] inline auto keyboardGrab( window_t _window ) -> bool {
    return ( SDL_GetWindowKeyboardGrab( _window ) );
}

// Get a window's mouse grab mode.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto mouseGrab( window_t _window ) -> bool {
    return ( SDL_GetWindowMouseGrab( _window ) );
}

// Get the window that currently has an input grab enabled.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto grabbed() -> window_t {
    return ( SDL_GetGrabbedWindow() );
}

// Confines the cursor to the specified area of a window.
//
// Note that this does NOT grab the cursor, it only defines the area a cursor
// is restricted to when the window has mouse focus.
//
// Should only be called on the main thread.
// TODO: Change box to clipping zone
inline void mouseClippingZone( window_t _window, const box_t< int >& _box ) {
    const bool l_result = SDL_SetWindowMouseRect( _window, _box );

    assert( l_result );
}

// Get the mouse confinement rectangle of a window.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto mouseClippingZone( window_t _window )
    -> std::optional< box_t< int > > {
    const SDL_Rect* l_result = SDL_GetWindowMouseRect( _window );

    if ( l_result ) {
        return ( box_t< int >{ *l_result } );

    } else {
        return ( std::nullopt );
    }
}

/**
 * Set the opacity for a window.
 *
 * The parameter `opacity` will be clamped internally between 0.0f
 * (transparent) and 1.0f (opaque).
 *
 * This function also returns false if setting the opacity isn't supported.
 *
 * \param window the window which will be made transparent or opaque.
 * \param opacity the opacity value (0.0f - transparent, 1.0f - opaque).
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetWindowOpacity
 */
bool SDL_SetWindowOpacity( window_t window, float opacity );

/**
 * Get the opacity of a window.
 *
 * If transparency isn't supported on this platform, opacity will be returned
 * as 1.0f without error.
 *
 * \param window the window to get the current opacity value from.
 * \returns the opacity, (0.0f - transparent, 1.0f - opaque), or -1.0f on
 *          failure; call SDL_GetError() for more information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_SetWindowOpacity
 */
float SDL_GetWindowOpacity( window_t window );

/**
 * Set the window as a child of a parent window.
 *
 * If the window is already the child of an existing window, it will be
 * reparented to the new owner. Setting the parent window to NULL unparents
 * the window and removes child window status.
 *
 * If a parent window is hidden or destroyed, the operation will be
 * recursively applied to child windows. Child windows hidden with the parent
 * that did not have their hidden status explicitly set will be restored when
 * the parent is shown.
 *
 * Attempting to set the parent of a window that is currently in the modal
 * state will fail. Use SDL_SetWindowModal() to cancel the modal status before
 * attempting to change the parent.
 *
 * Popup windows cannot change parents and attempts to do so will fail.
 *
 * Setting a parent window that is currently the sibling or descendent of the
 * child window results in undefined behavior.
 *
 * \param window the window that should become the child of a parent.
 * \param parent the new parent window for the child window.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_SetWindowModal
 */
bool SDL_SetWindowParent( window_t window, window_t parent );

/**
 * Toggle the state of the window as modal.
 *
 * To enable modal status on a window, the window must currently be the child
 * window of a parent, or toggling modal status on will fail.
 *
 * \param window the window on which to set the modal state.
 * \param modal true to toggle modal status on, false to toggle it off.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_SetWindowParent
 * \sa SDL_WINDOW_MODAL
 */
bool SDL_SetWindowModal( window_t window, bool modal );

/**
 * Set whether the window may have input focus.
 *
 * \param window the window to set focusable state.
 * \param focusable true to allow input focus, false to not allow input focus.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 */
bool SDL_SetWindowFocusable( window_t window, bool focusable );

/**
 * Display the system-level window menu.
 *
 * This default window menu is provided by the system and on some platforms
 * provides functionality for setting or changing privileged state on the
 * window, such as moving it between workspaces or displays, or toggling the
 * always-on-top property.
 *
 * On platforms or desktops where this is unsupported, this function does
 * nothing.
 *
 * \param window the window for which the menu will be displayed.
 * \param x the x coordinate of the menu, relative to the origin (top-left) of
 *          the client area.
 * \param y the y coordinate of the menu, relative to the origin (top-left) of
 *          the client area.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 */
bool SDL_ShowWindowSystemMenu( window_t window, int x, int y );

} // namespace window

// An opaque handle to an OpenGL context.
using contextGL_t = gsl::not_null< gsl::not_null< SDL_GLContextState* >* >;

// Opaque type for an EGL display.
using displayEGL_t = gsl::not_null< void* >;

// Opaque type for an EGL config.
using configEGL_t = gsl::not_null< void* >;

// Opaque type for an EGL surface.
using surfaceEGL_t = gsl::not_null< void* >;

// An EGL attribute, used when creating an EGL context.
using attributeEGL_t = intptr_t;

// An EGL integer attribute, used when creating an EGL surface.
using intEGL_t = int;

// EGL platform attribute initialization callback.
//
// This is called when SDL is attempting to create an EGL context, to let the
// app add extra attributes to its eglGetPlatformDisplay() call.
//
// The callback should return a pointer to an EGL attribute array terminated
// with `EGL_NONE`. If this function returns NULL, the SDL_CreateWindow
// process will fail gracefully.
//
// The returned pointer should be allocated with SDL_malloc() and will be
// passed to SDL_free().
//
// The arrays returned by each callback will be appended to the existing
// attribute arrays defined by SDL.
using attributeEGLArrayCallback_t = gsl::not_null< SDL_EGLAttribArrayCallback >;

// EGL surface/context attribute initialization callback types.
//
// This is called when SDL is attempting to create an EGL surface, to let the
// app add extra attributes to its eglCreateWindowSurface() or
// eglCreateContext calls.
//
// For convenience, the EGLDisplay and EGLConfig to use are provided to the
// callback.
//
// The callback should return a pointer to an EGL attribute array terminated
// with `EGL_NONE`. If this function returns NULL, the SDL_CreateWindow
// process will fail gracefully.
//
// The returned pointer should be allocated with SDL_malloc() and will be
// passed to SDL_free().
//
// The arrays returned by each callback will be appended to the existing
// attribute arrays defined by SDL.
using intEGLArrayCallback_t = gsl::not_null< SDL_EGLIntArrayCallback >;

// An enumeration of OpenGL configuration attributes.
//
// While you can set most OpenGL attributes normally, the attributes listed
// above must be known before SDL creates the window that will be used with
// the OpenGL context. These attributes are set and read with
// SDL_GL_SetAttribute() and SDL_GL_GetAttribute().
//
// In some cases, these attributes are minimum requests; the GL does not
// promise to give you exactly what you asked for. It's possible to ask for a
// 16-bit depth buffer and get a 24-bit one instead, for example, or to ask
// for no stencil buffer and still have one available. Context creation should
// fail if the GL can't provide your requested attributes at a minimum, but
// you should check to see exactly what you got.
using attributeGL_t = enum class attributeGL : uint8_t {
    redSize,        /**< the minimum number of bits for the red channel of the
                               color buffer; defaults to 8. */
    greenSize,      /**< the minimum number of bits for the green channel of
                               the color buffer; defaults to 8. */
    blueSize,       /**< the minimum number of bits for the blue channel of
                               the color buffer; defaults to 8. */
    alphaSize,      /**< the minimum number of bits for the alpha channel of
                               the color buffer; defaults to 8. */
    bufferSize,     /**< the minimum number of bits for frame buffer size;
                               defaults to 0. */
    doublebuffer,   /**< whether the output is single or double buffered;
                              defaults to double buffering on. */
    depthSize,      /**< the minimum number of bits in the depth buffer;
                               defaults to 16. */
    stencilSize,    /**< the minimum number of bits in the stencil buffer;
                               defaults to 0. */
    accumRedSize,   /**< the minimum number of bits for the red channel
                                of the accumulation buffer; defaults to 0. */
    accumGreenSize, /**< the minimum number of bits for the green
                                channel of the accumulation buffer; defaults to
                                0. */
    accumBlueSize,  /**< the minimum number of bits for the blue channel
                                of the accumulation buffer; defaults to 0. */
    accumAlphaSize, /**< the minimum number of bits for the alpha
                                channel of the accumulation buffer; defaults to
                                0. */
    stereo,         /**< whether the output is stereo 3D; defaults to off. */
    multiSampleBuffers,  /**< the number of buffers used for multisample
                                   anti-aliasing; defaults to 0. */
    multiSampleSamples,  /**< the number of samples used around the
                                   current pixel used for multisample
                                   anti-aliasing. */
    acceleratedVisual,   /**< set to 1 to require hardware acceleration,
                                    set to 0 to force software rendering; defaults
                                    to allow either. */
    retainedBacking,     /**< not used (deprecated). */
    contextMajorVersion, /**< OpenGL context major version. */
    contextMinorVersion, /**< OpenGL context minor version. */
    contextFlags,        /**< some combination of 0 or more of elements of the
                                    SDL_GLContextFlag enumeration; defaults to 0. */
    contextProfileMask,  /**< type of GL context (Core, Compatibility,
                                     ES). See SDL_GLProfile; default value
                                     depends on platform. */
    shareWithCurrentContext, /**< OpenGL context sharing; defaults to 0. */
    framebufferSrgbCapable, /**< requests sRGB capable visual; defaults to 0. */
    contextReleaseBehavior, /**< sets context the release behavior. See
                                        SDL_GLContextReleaseFlag; defaults to
                                        FLUSH. */
    contextResetNotification, /**< set context reset notification. See
                                          SDL_GLContextResetNotification;
                                          defaults to NO_NOTIFICATION. */
    contextNoError,
    floatBuffers,
    platformEGL,
};

using attributeGLUnderlying_t = std::underlying_type_t< attributeGL_t >;

[[nodiscard]] constexpr auto toLegacy( attributeGL_t _value ) -> SDL_GLAttr {
    return ( static_cast< SDL_GLAttr >( _value ) );
}

[[nodiscard]] constexpr auto toLegacy( attributeGL_t* _value ) -> SDL_GLAttr* {
    return ( std::bit_cast< SDL_GLAttr* >( _value ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_GLAttr _value ) -> attributeGL_t {
    return ( static_cast< attributeGL_t >( _value ) );
}

// Possible values to be set for the SDL_GL_CONTEXT_PROFILE_MASK attribute.
using profileGL_t = enum class profileGL : uint8_t {
    core = 0x1,        /**< OpenGL Core Profile context */
    compatibility = 2, /**< OpenGL Compatibility Profile context */
    es = 0x4,          /**< GLX_CONTEXT_ES2_PROFILE_BIT_EXT \ */
};

using profileGLUnderlying_t = std::underlying_type_t< profileGL_t >;

[[nodiscard]] constexpr auto toLegacy( profileGL_t _value ) -> SDL_GLProfile {
    return ( static_cast< SDL_GLProfile >( _value ) );
}

[[nodiscard]] constexpr auto toLegacy( profileGL_t* _value ) -> SDL_GLProfile* {
    return ( std::bit_cast< SDL_GLProfile* >( _value ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_GLProfile _value ) -> profileGL_t {
    return ( static_cast< profileGL_t >( _value ) );
}

// Possible flags to be set for the SDL_GL_CONTEXT_FLAGS attribute.
using contextGLFlag_t = enum class contextGLFlag : uint8_t {
    debug = 0x1,
    forwardCompatible = 0x2,
    robustAccess = 0x4,
    resetIsolation = 0x8,
};

using contextGLFlagUnderlying_t = std::underlying_type_t< contextGLFlag_t >;

[[nodiscard]] constexpr auto toLegacy( contextGLFlag_t _value )
    -> SDL_GLContextFlag {
    return ( static_cast< SDL_GLContextFlag >( _value ) );
}

[[nodiscard]] constexpr auto toLegacy( contextGLFlag_t* _value )
    -> SDL_GLContextFlag* {
    return ( std::bit_cast< SDL_GLContextFlag* >( _value ) );
}

#if 0
[[nodiscard]] constexpr auto fromLegacy( SDL_GLContextFlag _value )
    -> contextGLFlag_t {
    return ( static_cast< contextGLFlag_t >( _value ) );
}
#endif

// Possible values to be set for the SDL_GL_CONTEXT_RELEASE_BEHAVIOR
// attribute.
using contextGLReleaseFlag_t = enum class contextGLReleaseFlag : uint8_t {
    none = 0,
    flush = 0x1,
};

using contextGLReleaseFlagUnderlying_t =
    std::underlying_type_t< contextGLReleaseFlag_t >;

[[nodiscard]] constexpr auto toLegacy( contextGLReleaseFlag_t _value )
    -> SDL_GLContextReleaseFlag {
    return ( static_cast< SDL_GLContextReleaseFlag >( _value ) );
}

[[nodiscard]] constexpr auto toLegacy( contextGLReleaseFlag_t* _value )
    -> SDL_GLContextReleaseFlag* {
    return ( std::bit_cast< SDL_GLContextReleaseFlag* >( _value ) );
}

#if 0
[[nodiscard]] constexpr auto fromLegacy( SDL_GLContextReleaseFlag _value )
    -> contextGLReleaseFlag_t {
    return ( static_cast< contextGLReleaseFlag_t >( _value ) );
}
#endif

// Possible values to be set SDL_GL_CONTEXT_RESET_NOTIFICATION attribute.
using contextGLResetNotification_t =
    enum class contextGLResetNotification : uint8_t {
        noNotification = 0,
        loseContext = 0x1,
    };

using contextGLResetNotificationUnderlying_t =
    std::underlying_type_t< contextGLResetNotification_t >;

[[nodiscard]] constexpr auto toLegacy( contextGLResetNotification_t _value )
    -> SDL_GLContextResetNotification {
    return ( static_cast< SDL_GLContextResetNotification >( _value ) );
}

[[nodiscard]] constexpr auto toLegacy( contextGLResetNotification_t* _value )
    -> SDL_GLContextResetNotification* {
    return ( std::bit_cast< SDL_GLContextResetNotification* >( _value ) );
}

#if 0
[[nodiscard]] constexpr auto fromLegacy( SDL_GLContextResetNotification _value )
    -> contextGLResetNotification_t {
    return ( static_cast< contextGLResetNotification_t >( _value ) );
}
#endif

namespace driver {

// Get the number of video drivers compiled into SDL.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto all() -> size_t {
    return ( SDL_GetNumVideoDrivers() );
}

// Get the name of a built in video driver.
//
// The video drivers are presented in the order in which they are normally
// checked during initialization.
//
// The names of drivers are all simple, low-ASCII identifiers, like "cocoa",
// "x11" or "windows". These never have Unicode characters, and are not meant
// to be proper names.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto name( size_t _index ) -> std::string_view {
    return ( SDL_GetVideoDriver( _index ) );
}

// Get the name of the currently initialized video driver.
//
// The names of drivers are all simple, low-ASCII identifiers, like "cocoa",
// "x11" or "windows". These never have Unicode characters, and are not meant
// to be proper names.
//
// The name of the current video driver or assert if no driver has been
// initialized.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto current() -> std::string_view {
    return { gsl::make_not_null( SDL_GetCurrentVideoDriver() ) };
}

} // namespace driver

namespace system_theme {

// System theme.
using type_t = enum class type : uint8_t {
    unknown, /**< Unknown system theme */
    light,   /**< Light colored system theme */
    dark,    /**< Dark colored system theme */
};

using typeUnderlying_t = std::underlying_type_t< type_t >;

[[nodiscard]] constexpr auto toLegacy( type_t _value ) -> SDL_SystemTheme {
    return ( static_cast< SDL_SystemTheme >( _value ) );
}

[[nodiscard]] constexpr auto toLegacy( type_t* _value ) -> SDL_SystemTheme* {
    return ( std::bit_cast< SDL_SystemTheme* >( _value ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_SystemTheme _value ) -> type_t {
    return ( static_cast< type_t >( _value ) );
}

// Get the current system theme.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto current() -> type_t {
    return ( fromLegacy( SDL_GetSystemTheme() ) );
}

} // namespace system_theme

/**
 * Possible return values from the SDL_HitTest callback.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This enum is available since SDL 3.2.0.
 *
 * \sa SDL_HitTest
 */
typedef enum SDL_HitTestResult {
    SDL_HITTEST_NORMAL,         /**< Region is normal. No special properties. */
    SDL_HITTEST_DRAGGABLE,      /**< Region can drag entire window. */
    SDL_HITTEST_RESIZE_TOPLEFT, /**< Region is the resizable top-left corner
                                   border. */
    SDL_HITTEST_RESIZE_TOP,     /**< Region is the resizable top border. */
    SDL_HITTEST_RESIZE_TOPRIGHT, /**< Region is the resizable top-right corner
                                    border. */
    SDL_HITTEST_RESIZE_RIGHT,    /**< Region is the resizable right border. */
    SDL_HITTEST_RESIZE_BOTTOMRIGHT, /**< Region is the resizable bottom-right
                                       corner border. */
    SDL_HITTEST_RESIZE_BOTTOM, /**< Region is the resizable bottom border. */
    SDL_HITTEST_RESIZE_BOTTOMLEFT, /**< Region is the resizable bottom-left
                                      corner border. */
    SDL_HITTEST_RESIZE_LEFT        /**< Region is the resizable left border. */
} SDL_HitTestResult;

/**
 * Callback used for hit-testing.
 *
 * \param win the SDL_Window where hit-testing was set on.
 * \param area an SDL_Point which should be hit-tested.
 * \param data what was passed as `callback_data` to SDL_SetWindowHitTest().
 * \returns an SDL_HitTestResult value.
 *
 * \sa SDL_SetWindowHitTest
 */
typedef SDL_HitTestResult ( *SDL_HitTest )( window_t win,
                                            const SDL_Point* area,
                                            void* data );

/**
 * Provide a callback that decides if a window region has special properties.
 *
 * Normally windows are dragged and resized by decorations provided by the
 * system window manager (a title bar, borders, etc), but for some apps, it
 * makes sense to drag them from somewhere else inside the window itself; for
 * example, one might have a borderless window that wants to be draggable from
 * any part, or simulate its own title bar, etc.
 *
 * This function lets the app provide a callback that designates pieces of a
 * given window as special. This callback is run during event processing if we
 * need to tell the OS to treat a region of the window specially; the use of
 * this callback is known as "hit testing."
 *
 * Mouse input may not be delivered to your application if it is within a
 * special area; the OS will often apply that input to moving the window or
 * resizing the window and not deliver it to the application.
 *
 * Specifying NULL for a callback disables hit-testing. Hit-testing is
 * disabled by default.
 *
 * Platforms that don't support this functionality will return false
 * unconditionally, even if you're attempting to disable hit-testing.
 *
 * Your callback may fire at any time, and its firing does not indicate any
 * specific behavior (for example, on Windows, this certainly might fire when
 * the OS is deciding whether to drag your window, but it fires for lots of
 * other reasons, too, some unrelated to anything you probably care about _and
 * when the mouse isn't actually at the location it is testing_). Since this
 * can fire at any time, you should try to keep your callback efficient,
 * devoid of allocations, etc.
 *
 * \param window the window to set hit-testing on.
 * \param callback the function to call when doing a hit-test.
 * \param callback_data an app-defined void pointer passed to **callback**.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 */
bool SDL_SetWindowHitTest( window_t window,
                           SDL_HitTest callback,
                           void* callback_data );

/**
 * Set the shape of a transparent window.
 *
 * This sets the alpha channel of a transparent window and any fully
 * transparent areas are also transparent to mouse clicks. If you are using
 * something besides the SDL render API, then you are responsible for drawing
 * the alpha channel of the window to match the shape alpha channel to get
 * consistent cross-platform results.
 *
 * The shape is copied inside this function, so you can free it afterwards. If
 * your shape surface changes, you should call SDL_SetWindowShape() again to
 * update the window. This is an expensive operation, so should be done
 * sparingly.
 *
 * The window must have been created with the SDL_WINDOW_TRANSPARENT flag.
 *
 * \param window the window.
 * \param shape the surface representing the shape of the window, or NULL to
 *              remove any current shape.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 */
bool SDL_SetWindowShape( window_t window, SDL_Surface* shape );

/**
 * Request a window to demand attention from the user.
 *
 * \param window the window to be flashed.
 * \param operation the operation to perform.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 */
bool SDL_FlashWindow( window_t window, SDL_FlashOperation operation );

/**
 * Sets the state of the progress bar for the given window’s taskbar icon.
 *
 * \param window the window whose progress state is to be modified.
 * \param state the progress state. `SDL_PROGRESS_STATE_NONE` stops displaying
 *              the progress bar.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.4.0.
 */
bool SDL_SetWindowProgressState( window_t window, SDL_ProgressState state );

/**
 * Get the state of the progress bar for the given window’s taskbar icon.
 *
 * \param window the window to get the current progress state from.
 * \returns the progress state, or `SDL_PROGRESS_STATE_INVALID` on failure;
 *          call SDL_GetError() for more information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.4.0.
 */
SDL_ProgressState SDL_GetWindowProgressState( window_t window );

/**
 * Sets the value of the progress bar for the given window’s taskbar icon.
 *
 * \param window the window whose progress value is to be modified.
 * \param value the progress value in the range of [0.0f - 1.0f]. If the value
 *              is outside the valid range, it gets clamped.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.4.0.
 */
bool SDL_SetWindowProgressValue( window_t window, float value );

/**
 * Get the value of the progress bar for the given window’s taskbar icon.
 *
 * \param window the window to get the current progress value from.
 * \returns the progress value in the range of [0.0f - 1.0f], or -1.0f on
 *          failure; call SDL_GetError() for more information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.4.0.
 */
float SDL_GetWindowProgressValue( window_t window );

/**
 * Destroy a window.
 *
 * Any child windows owned by the window will be recursively destroyed as
 * well.
 *
 * Note that on some platforms, the visible window may not actually be removed
 * from the screen until the SDL event loop is pumped again, even though the
 * SDL_Window is no longer valid after this call.
 *
 * \param window the window to destroy.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_CreatePopupWindow
 * \sa SDL_CreateWindow
 * \sa SDL_CreateWindowWithProperties
 */
void SDL_DestroyWindow( window_t window );

/**
 * Check whether the screensaver is currently enabled.
 *
 * The screensaver is disabled by default.
 *
 * The default can also be changed using `SDL_HINT_VIDEO_ALLOW_SCREENSAVER`.
 *
 * \returns true if the screensaver is enabled, false if it is disabled.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_DisableScreenSaver
 * \sa SDL_EnableScreenSaver
 */
bool SDL_ScreenSaverEnabled( void );

/**
 * Allow the screen to be blanked by a screen saver.
 *
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_DisableScreenSaver
 * \sa SDL_ScreenSaverEnabled
 */
bool SDL_EnableScreenSaver( void );

/**
 * Prevent the screen from being blanked by a screen saver.
 *
 * If you disable the screensaver, it is automatically re-enabled when SDL
 * quits.
 *
 * The screensaver is disabled by default, but this may by changed by
 * SDL_HINT_VIDEO_ALLOW_SCREENSAVER.
 *
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_EnableScreenSaver
 * \sa SDL_ScreenSaverEnabled
 */
bool SDL_DisableScreenSaver( void );

/**
 *  \name OpenGL support functions
 */
/* @{ */

/**
 * Dynamically load an OpenGL library.
 *
 * This should be done after initializing the video driver, but before
 * creating any OpenGL windows. If no OpenGL library is loaded, the default
 * library will be loaded upon creation of the first OpenGL window.
 *
 * If you do this, you need to retrieve all of the GL functions used in your
 * program from the dynamic library using SDL_GL_GetProcAddress().
 *
 * \param path the platform dependent OpenGL library name, or NULL to open the
 *             default OpenGL library.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GL_GetProcAddress
 * \sa SDL_GL_UnloadLibrary
 */
bool SDL_GL_LoadLibrary( const char* path );

/**
 * Get an OpenGL function by name.
 *
 * If the GL library is loaded at runtime with SDL_GL_LoadLibrary(), then all
 * GL functions must be retrieved this way. Usually this is used to retrieve
 * function pointers to OpenGL extensions.
 *
 * There are some quirks to looking up OpenGL functions that require some
 * extra care from the application. If you code carefully, you can handle
 * these quirks without any platform-specific code, though:
 *
 * - On Windows, function pointers are specific to the current GL context;
 *   this means you need to have created a GL context and made it current
 *   before calling SDL_GL_GetProcAddress(). If you recreate your context or
 *   create a second context, you should assume that any existing function
 *   pointers aren't valid to use with it. This is (currently) a
 *   Windows-specific limitation, and in practice lots of drivers don't suffer
 *   this limitation, but it is still the way the wgl API is documented to
 *   work and you should expect crashes if you don't respect it. Store a copy
 *   of the function pointers that comes and goes with context lifespan.
 * - On X11, function pointers returned by this function are valid for any
 *   context, and can even be looked up before a context is created at all.
 *   This means that, for at least some common OpenGL implementations, if you
 *   look up a function that doesn't exist, you'll get a non-NULL result that
 *   is _NOT_ safe to call. You must always make sure the function is actually
 *   available for a given GL context before calling it, by checking for the
 *   existence of the appropriate extension with SDL_GL_ExtensionSupported(),
 *   or verifying that the version of OpenGL you're using offers the function
 *   as core functionality.
 * - Some OpenGL drivers, on all platforms, *will* return NULL if a function
 *   isn't supported, but you can't count on this behavior. Check for
 *   extensions you use, and if you get a NULL anyway, act as if that
 *   extension wasn't available. This is probably a bug in the driver, but you
 *   can code defensively for this scenario anyhow.
 * - Just because you're on Linux/Unix, don't assume you'll be using X11.
 *   Next-gen display servers are waiting to replace it, and may or may not
 *   make the same promises about function pointers.
 * - OpenGL function pointers must be declared `APIENTRY` as in the example
 *   code. This will ensure the proper calling convention is followed on
 *   platforms where this matters (Win32) thereby avoiding stack corruption.
 *
 * \param proc the name of an OpenGL function.
 * \returns a pointer to the named OpenGL function. The returned pointer
 *          should be cast to the appropriate function signature.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GL_ExtensionSupported
 * \sa SDL_GL_LoadLibrary
 * \sa SDL_GL_UnloadLibrary
 */
SDL_FunctionPointer SDL_GL_GetProcAddress( const char* proc );

/**
 * Get an EGL library function by name.
 *
 * If an EGL library is loaded, this function allows applications to get entry
 * points for EGL functions. This is useful to provide to an EGL API and
 * extension loader.
 *
 * \param proc the name of an EGL function.
 * \returns a pointer to the named EGL function. The returned pointer should
 *          be cast to the appropriate function signature.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_EGL_GetCurrentDisplay
 */
SDL_FunctionPointer SDL_EGL_GetProcAddress( const char* proc );

/**
 * Unload the OpenGL library previously loaded by SDL_GL_LoadLibrary().
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GL_LoadLibrary
 */
void SDL_GL_UnloadLibrary( void );

/**
 * Check if an OpenGL extension is supported for the current context.
 *
 * This function operates on the current GL context; you must have created a
 * context and it must be current before calling this function. Do not assume
 * that all contexts you create will have the same set of extensions
 * available, or that recreating an existing context will offer the same
 * extensions again.
 *
 * While it's probably not a massive overhead, this function is not an O(1)
 * operation. Check the extensions you care about after creating the GL
 * context and save that information somewhere instead of calling the function
 * every time you need to know.
 *
 * \param extension the name of the extension to check.
 * \returns true if the extension is supported, false otherwise.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 */
bool SDL_GL_ExtensionSupported( const char* extension );

/**
 * Reset all previously set OpenGL context attributes to their default values.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GL_GetAttribute
 * \sa SDL_GL_SetAttribute
 */
void SDL_GL_ResetAttributes( void );

/**
 * Set an OpenGL window attribute before window creation.
 *
 * This function sets the OpenGL attribute `attr` to `value`. The requested
 * attributes should be set before creating an OpenGL window. You should use
 * SDL_GL_GetAttribute() to check the values after creating the OpenGL
 * context, since the values obtained can differ from the requested ones.
 *
 * \param attr an SDL_GLAttr enum value specifying the OpenGL attribute to
 *             set.
 * \param value the desired value for the attribute.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GL_GetAttribute
 * \sa SDL_GL_ResetAttributes
 */
bool SDL_GL_SetAttribute( SDL_GLAttr attr, int value );

/**
 * Get the actual value for an attribute from the current context.
 *
 * \param attr an SDL_GLAttr enum value specifying the OpenGL attribute to
 *             get.
 * \param value a pointer filled in with the current value of `attr`.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GL_ResetAttributes
 * \sa SDL_GL_SetAttribute
 */
bool SDL_GL_GetAttribute( SDL_GLAttr attr, int* value );

/**
 * Create an OpenGL context for an OpenGL window, and make it current.
 *
 * Windows users new to OpenGL should note that, for historical reasons, GL
 * functions added after OpenGL version 1.1 are not available by default.
 * Those functions must be loaded at run-time, either with an OpenGL
 * extension-handling library or with SDL_GL_GetProcAddress() and its related
 * functions.
 *
 * SDL_GLContext is opaque to the application.
 *
 * \param window the window to associate with the context.
 * \returns the OpenGL context associated with `window` or NULL on failure;
 *          call SDL_GetError() for more information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GL_DestroyContext
 * \sa SDL_GL_MakeCurrent
 */
SDL_GLContext SDL_GL_CreateContext( window_t window );

/**
 * Set up an OpenGL context for rendering into an OpenGL window.
 *
 * The context must have been created with a compatible window.
 *
 * \param window the window to associate with the context.
 * \param context the OpenGL context to associate with the window.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GL_CreateContext
 */
bool SDL_GL_MakeCurrent( window_t window, SDL_GLContext context );

/**
 * Get the currently active OpenGL window.
 *
 * \returns the currently active OpenGL window on success or NULL on failure;
 *          call SDL_GetError() for more information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 */
window_t SDL_GL_GetCurrentWindow( void );

/**
 * Get the currently active OpenGL context.
 *
 * \returns the currently active OpenGL context or NULL on failure; call
 *          SDL_GetError() for more information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GL_MakeCurrent
 */
SDL_GLContext SDL_GL_GetCurrentContext( void );

/**
 * Get the currently active EGL display.
 *
 * \returns the currently active EGL display or NULL on failure; call
 *          SDL_GetError() for more information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 */
SDL_EGLDisplay SDL_EGL_GetCurrentDisplay( void );

/**
 * Get the currently active EGL config.
 *
 * \returns the currently active EGL config or NULL on failure; call
 *          SDL_GetError() for more information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 */
SDL_EGLConfig SDL_EGL_GetCurrentConfig( void );

/**
 * Get the EGL surface associated with the window.
 *
 * \param window the window to query.
 * \returns the EGLSurface pointer associated with the window, or NULL on
 *          failure.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 */
SDL_EGLSurface SDL_EGL_GetWindowSurface( window_t window );

/**
 * Sets the callbacks for defining custom EGLAttrib arrays for EGL
 * initialization.
 *
 * Callbacks that aren't needed can be set to NULL.
 *
 * NOTE: These callback pointers will be reset after SDL_GL_ResetAttributes.
 *
 * \param platformAttribCallback callback for attributes to pass to
 *                               eglGetPlatformDisplay. May be NULL.
 * \param surfaceAttribCallback callback for attributes to pass to
 *                              eglCreateSurface. May be NULL.
 * \param contextAttribCallback callback for attributes to pass to
 *                              eglCreateContext. May be NULL.
 * \param userdata a pointer that is passed to the callbacks.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 */
void SDL_EGL_SetAttributeCallbacks(
    SDL_EGLAttribArrayCallback platformAttribCallback,
    SDL_EGLIntArrayCallback surfaceAttribCallback,
    SDL_EGLIntArrayCallback contextAttribCallback,
    void* userdata );

/**
 * Set the swap interval for the current OpenGL context.
 *
 * Some systems allow specifying -1 for the interval, to enable adaptive
 * vsync. Adaptive vsync works the same as vsync, but if you've already missed
 * the vertical retrace for a given frame, it swaps buffers immediately, which
 * might be less jarring for the user during occasional framerate drops. If an
 * application requests adaptive vsync and the system does not support it,
 * this function will fail and return false. In such a case, you should
 * probably retry the call with 1 for the interval.
 *
 * Adaptive vsync is implemented for some glX drivers with
 * GLX_EXT_swap_control_tear, and for some Windows drivers with
 * WGL_EXT_swap_control_tear.
 *
 * Read more on the Khronos wiki:
 * https://www.khronos.org/opengl/wiki/Swap_Interval#Adaptive_Vsync
 *
 * \param interval 0 for immediate updates, 1 for updates synchronized with
 *                 the vertical retrace, -1 for adaptive vsync.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GL_GetSwapInterval
 */
bool SDL_GL_SetSwapInterval( int interval );

/**
 * Get the swap interval for the current OpenGL context.
 *
 * If the system can't determine the swap interval, or there isn't a valid
 * current context, this function will set *interval to 0 as a safe default.
 *
 * \param interval output interval value. 0 if there is no vertical retrace
 *                 synchronization, 1 if the buffer swap is synchronized with
 *                 the vertical retrace, and -1 if late swaps happen
 *                 immediately instead of waiting for the next retrace.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GL_SetSwapInterval
 */
bool SDL_GL_GetSwapInterval( int* interval );

/**
 * Update a window with OpenGL rendering.
 *
 * This is used with double-buffered OpenGL contexts, which are the default.
 *
 * On macOS, make sure you bind 0 to the draw framebuffer before swapping the
 * window, otherwise nothing will happen. If you aren't using
 * glBindFramebuffer(), this is the default and you won't have to do anything
 * extra.
 *
 * \param window the window to change.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 */
bool SDL_GL_SwapWindow( window_t window );

/**
 * Delete an OpenGL context.
 *
 * \param context the OpenGL context to be deleted.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GL_CreateContext
 */
bool SDL_GL_DestroyContext( SDL_GLContext context );

/* @} */ /* OpenGL support functions */

} // namespace slickdl::video
