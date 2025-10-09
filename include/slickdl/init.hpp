#pragma once

#include <cstdint>

// All SDL programs need to initialize the library before starting to work
// with it
//
// Almost everything can simply call SDL_Init() near startup, with a handful
// of flags to specify subsystems to touch. These are here to make sure SDL
// does not even attempt to touch low-level pieces of the operating system
// that you don't intend to use. For example, you might be using SDL for video
// and input but chose an external library for audio, and in this case you
// would just need to leave off the `SDL_INIT_AUDIO` flag to make sure that
// external library has complete control
//
// Most apps, when terminating, should call SDL_Quit(). This will clean up
// (nearly) everything that SDL might have allocated, and crucially, it'll
// make sure that the display's resolution is back to what the user expects if
// you had previously changed it for your game
//
// SDL3 apps are strongly encouraged to call SDL_SetAppMetadata() at startup
// to fill in details about the program. This is completely optional, but it
// helps in small ways (we can provide an About dialog box for the macOS menu,
// we can name the app in the system's audio mixer, etc). Those that want to
// provide a _lot_ of information should look at the more-detailed
// SDL_SetAppMetadataProperty()
namespace slickdl::init {

/* As of version 0.5, SDL is loaded dynamically into the application */

/**
 * Initialization flags for SDL_Init and/or SDL_InitSubSystem
 *
 * These are the flags which may be passed to SDL_Init(). You should specify
 * the subsystems which you will be using in your application
 *
 * \since This datatype is available since SDL 3.2.0
 *
 * \sa SDL_Init
 * \sa SDL_Quit
 * \sa SDL_InitSubSystem
 * \sa SDL_QuitSubSystem
 * \sa SDL_WasInit
 */
using flag_t = enum class flag : uint32_t {
    audio = 0x10U,     /**< `SDL_INIT_AUDIO` implies `SDL_INIT_EVENTS` */
    video = 0x20U,     /**< `SDL_INIT_VIDEO` implies `SDL_INIT_EVENTS`, should
                               be \ initialized on the main thread */
    joystick = 0x200U, /**< `SDL_INIT_JOYSTICK` implies `SDL_INIT_EVENTS` */
    haptic = 0x1000U,
    gamepad = 0x2000U, /**< `SDL_INIT_GAMEPAD` implies `SDL_INIT_JOYSTICK` */
    events = 0x4000U,
    sensor = 0x8000U,  /**< `SDL_INIT_SENSOR` implies `SDL_INIT_EVENTS` */
    camera = 0x10000U, /**< `SDL_INIT_CAMERA` implies `SDL_INIT_EVENTS` */
};

/**
 * Return values for optional main callbacks
 *
 * Returning SDL_APP_SUCCESS or SDL_APP_FAILURE from SDL_AppInit,
 * SDL_AppEvent, or SDL_AppIterate will terminate the program and report
 * success/failure to the operating system. What that means is
 * platform-dependent. On Unix, for example, on success, the process error
 * code will be zero, and on failure it will be 1. This interface doesn't
 * allow you to return specific exit codes, just whether there was an error
 * generally or not
 *
 * Returning SDL_APP_CONTINUE from these functions will let the app continue
 * to run
 *
 * See
 * [Main callbacks in
 * SDL3](https://wiki.libsdl.org/SDL3/README/main-functions#main-callbacks-in-sdl3)
 * for complete details
 *
 * \since This enum is available since SDL 3.2.0
 */
using appResult_t = enum class appResult : uint8_t {
    remain,  /**< Value that requests that the app continue from the
                          main callbacks. */
    success, /**< Value that requests termination with success from the
                        main callbacks. */
    failure  /**< Value that requests termination with error from the
                        main callbacks. */
};

/**
 * Initialize the SDL library
 *
 * SDL_Init() simply forwards to calling SDL_InitSubSystem(). Therefore, the
 * two may be used interchangeably. Though for readability of your code
 * SDL_InitSubSystem() might be preferred
 *
 * The file I/O (for example: SDL_IOFromFile) and threading (SDL_CreateThread)
 * subsystems are initialized by default. Message boxes
 * (SDL_ShowSimpleMessageBox) also attempt to work without initializing the
 * video subsystem, in hopes of being useful in showing an error dialog when
 * SDL_Init fails. You must specifically initialize other subsystems if you
 * use them in your application
 *
 * Logging (such as SDL_Log) works without initialization, too
 *
 * `flags` may be any of the following OR'd together:
 *
 * - `SDL_INIT_AUDIO`: audio subsystem; automatically initializes the events
 *   subsystem
 * - `SDL_INIT_VIDEO`: video subsystem; automatically initializes the events
 *   subsystem, should be initialized on the main thread
 * - `SDL_INIT_JOYSTICK`: joystick subsystem; automatically initializes the
 *   events subsystem
 * - `SDL_INIT_HAPTIC`: haptic (force feedback) subsystem
 * - `SDL_INIT_GAMEPAD`: gamepad subsystem; automatically initializes the
 *   joystick subsystem
 * - `SDL_INIT_EVENTS`: events subsystem
 * - `SDL_INIT_SENSOR`: sensor subsystem; automatically initializes the events
 *   subsystem
 * - `SDL_INIT_CAMERA`: camera subsystem; automatically initializes the events
 *   subsystem
 *
 * Subsystem initialization is ref-counted, you must call SDL_QuitSubSystem()
 * for each SDL_InitSubSystem() to correctly shutdown a subsystem manually (or
 * call SDL_Quit() to force shutdown). If a subsystem is already loaded then
 * this call will increase the ref-count and return
 *
 * Consider reporting some basic metadata about your application before
 * calling SDL_Init, using either SDL_SetAppMetadata() or
 * SDL_SetAppMetadataProperty()
 *
 * \param flags subsystem initialization flags
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information
 *
 * \since This function is available since SDL 3.2.0
 *
 * \sa SDL_SetAppMetadata
 * \sa SDL_SetAppMetadataProperty
 * \sa SDL_InitSubSystem
 * \sa SDL_Quit
 * \sa SDL_SetMainReady
 * \sa SDL_WasInit
 */
extern auto init( flag_t _flags ) -> bool;

/**
 * Compatibility function to initialize the SDL library
 *
 * This function and SDL_Init() are interchangeable
 *
 * \param flags any of the flags used by SDL_Init(); see SDL_Init for details
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information
 *
 * \since This function is available since SDL 3.2.0
 *
 * \sa SDL_Init
 * \sa SDL_Quit
 * \sa SDL_QuitSubSystem
 */
extern auto initSubSystem( flag_t _flags ) -> bool;

/**
 * Shut down specific SDL subsystems
 *
 * You still need to call SDL_Quit() even if you close all open subsystems
 * with SDL_QuitSubSystem()
 *
 * \param flags any of the flags used by SDL_Init(); see SDL_Init for details
 *
 * \since This function is available since SDL 3.2.0
 *
 * \sa SDL_InitSubSystem
 * \sa SDL_Quit
 */
extern void quitSubSystem( flag_t _flags );

/**
 * Get a mask of the specified subsystems which are currently initialized
 *
 * \param flags any of the flags used by SDL_Init(); see SDL_Init for details
 * \returns a mask of all initialized subsystems if `flags` is 0, otherwise it
 *          returns the initialization status of the specified subsystems
 *
 * \since This function is available since SDL 3.2.0
 *
 * \sa SDL_Init
 * \sa SDL_InitSubSystem
 */
extern auto wasInit( flag_t _flags ) -> flag_t;

/**
 * Clean up all initialized subsystems
 *
 * You should call this function even if you have already shutdown each
 * initialized subsystem with SDL_QuitSubSystem(). It is safe to call this
 * function even in the case of errors in initialization
 *
 * You can use this function with atexit() to ensure that it is run when your
 * application is shutdown, but it is not wise to do this from a library or
 * other dynamically loaded code
 *
 * \since This function is available since SDL 3.2.0
 *
 * \sa SDL_Init
 * \sa SDL_QuitSubSystem
 */
extern void quit();

} // namespace slickdl::init
