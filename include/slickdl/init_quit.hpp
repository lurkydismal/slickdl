#pragma once

#include <SDL3/SDL_init.h>

#include <cstdint>

#include "slickdl.hpp"

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
namespace slickdl {

namespace init {

// Initialization flags for SDL_Init and/or SDL_InitSubSystem
//
// These are the flags which may be passed to SDL_Init(). You should specify
// the subsystems which you will be using in your application
using flag_t = enum class flag : uint32_t {
    none = 0,
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

using flagUnderlying_t = std::underlying_type_t< flag_t >;

[[nodiscard]] constexpr auto toLegacy( flag_t _flag ) -> SDL_InitFlags {
    return ( static_cast< SDL_InitFlags >( _flag ) );
}

// SDL_Init() simply forwards to calling SDL_InitSubSystem(). Therefore, the
// two may be used interchangeably. Though for readability of your code
// SDL_InitSubSystem() might be preferred
//
// The file I/O (for example: SDL_IOFromFile) and threading (SDL_CreateThread)
// subsystems are initialized by default. Message boxes
// (SDL_ShowSimpleMessageBox) also attempt to work without initializing the
// video subsystem, in hopes of being useful in showing an error dialog when
// SDL_Init fails. You must specifically initialize other subsystems if you
// use them in your application
//
// Logging (such as SDL_Log) works without initialization, too
//
// `flags` may be any of the following OR'd together:
//
// - `SDL_INIT_AUDIO`: audio subsystem; automatically initializes the events
//   subsystem
// - `SDL_INIT_VIDEO`: video subsystem; automatically initializes the events
//   subsystem, should be initialized on the main thread
// - `SDL_INIT_JOYSTICK`: joystick subsystem; automatically initializes the
//   events subsystem
// - `SDL_INIT_HAPTIC`: haptic (force feedback) subsystem
// - `SDL_INIT_GAMEPAD`: gamepad subsystem; automatically initializes the
//   joystick subsystem
// - `SDL_INIT_EVENTS`: events subsystem
// - `SDL_INIT_SENSOR`: sensor subsystem; automatically initializes the events
//   subsystem
// - `SDL_INIT_CAMERA`: camera subsystem; automatically initializes the events
//   subsystem
//
// Subsystem initialization is ref-counted, you must call SDL_QuitSubSystem()
// for each SDL_InitSubSystem() to correctly shutdown a subsystem manually (or
// call SDL_Quit() to force shutdown). If a subsystem is already loaded then
// this call will increase the ref-count and return
//
// Consider reporting some basic metadata about your application before
// calling SDL_Init, using either SDL_SetAppMetadata() or
// SDL_SetAppMetadataProperty()
// TODO: Rename
inline void all( flag_t _flags ) {
    const bool l_result = SDL_Init( toLegacy( _flags ) );

    assert( l_result );
}

// This function and SDL_Init() are interchangeable
inline void subSystem( flag_t _flags ) {
    const bool l_result = SDL_InitSubSystem( toLegacy( _flags ) );

    assert( l_result );
}

// Get a mask of the specified subsystems which are currently initialized
//
// A mask of all initialized subsystems if `flags` is NULL, otherwise it returns
// the initialization status of the specified subsystems
auto currentFlags( std::optional< init::flag_t > _mask = std::nullopt )
    -> init::flag_t {
    const auto l_result = static_cast< init::flag_t >(
        SDL_WasInit( toLegacy( _mask.value_or( init::flag_t::none ) ) ) );

    assert( static_cast< flagUnderlying_t >( l_result ) != 0 );

    return ( l_result );
}

} // namespace init

namespace quit {

// Clean up all initialized subsystems
//
// You should call this function even if you have already shutdown each
// initialized subsystem with SDL_QuitSubSystem(). It is safe to call this
// function even in the case of errors in initialization
inline void all() {
    SDL_Quit();
}

// You still need to call SDL_Quit() even if you close all open subsystems
// with SDL_QuitSubSystem()
inline void subSystem( init::flag_t _flags ) {
    SDL_QuitSubSystem( init::toLegacy( _flags ) );
}

} // namespace quit

} // namespace slickdl
