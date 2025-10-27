#pragma once

#include <SDL3/SDL_gamepad.h>

#include <cmath>
#include <cstdint>
#include <gsl/pointers>
#include <string>
#include <type_traits>

#include "slickdl.hpp"
#include "slickdl/GUID.hpp"
#include "slickdl/joystick.hpp"
#include "slickdl/power.hpp"
#include "slickdl/properties.hpp"
#include "slickdl/sensor.hpp"

// SDL provides a low-level joystick API, which just treats joysticks as an
// arbitrary pile of buttons, axes, and hat switches. If you're planning to
// write your own control configuration screen, this can give you a lot of
// flexibility, but that's a lot of work, and most things that we consider
// "joysticks" now are actually console-style gamepads. So SDL provides the
// gamepad API on top of the lower-level joystick functionality.
//
// The difference between a joystick and a gamepad is that a gamepad tells you
// _where_ a button or axis is on the device. You don't speak to gamepads in
// terms of arbitrary numbers like "button 3" or "axis 2" but in standard
// locations: the d-pad, the shoulder buttons, triggers, A/B/X/Y (or
// X/O/Square/Triangle, if you will).
//
// One turns a joystick into a gamepad by providing a magic configuration
// string, which tells SDL the details of a specific device: when you see this
// specific hardware, if button 2 gets pressed, this is actually D-Pad Up,
// etc.
//
// SDL has many popular controllers configured out of the box, and users can
// add their own controller details through an environment variable if it's
// otherwise unknown to SDL.
//
// In order to use these functions, SDL_Init() must have been called with the
// SDL_INIT_GAMEPAD flag. This causes SDL to scan the system for gamepads, and
// load appropriate drivers.
//
// If you would like to receive gamepad updates while the application is in
// the background, you should set the following hint before calling
// SDL_Init(): SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS
//
// Gamepads support various optional features such as rumble, color LEDs,
// touchpad, gyro, etc. The support for these features varies depending on the
// controller and OS support available. You can check for LED and rumble
// capabilities at runtime by calling SDL_GetGamepadProperties() and checking
// the various capability properties. You can check for touchpad by calling
// SDL_GetNumGamepadTouchpads() and check for gyro and accelerometer by
// calling SDL_GamepadHasSensor().
//
// By default SDL will try to use the most capable driver available, but you
// can tune which OS drivers to use with the various joystick hints in
// SDL_hints.h.
//
// Your application should always support gamepad hotplugging. On some
// platforms like Xbox, Steam Deck, etc., this is a requirement for
// certification. On other platforms, like macOS and Windows when using
// Windows.Gaming.Input, controllers may not be available at startup and will
// come in at some point after you've started processing events.
namespace slickdl::gamepad {

// Standard gamepad types.
//
// This type does not necessarily map to first-party controllers from
// Microsoft/Sony/Nintendo; in many cases, third-party controllers can report
// as these, either because they were designed for a specific console, or they
// simply most closely match that console's controllers (does it have A/B/X/Y
// buttons or X/O/Square/Triangle? Does it have a touchpad? etc).
using type_t = enum class type : uint8_t {
    unknown = 0,
    standard,
    xbox360,
    xboxone,
    ps3,
    ps4,
    ps5,
    nintendoSwitchPro,
    nintendoSwitchJoyconLeft,
    nintendoSwitchJoyconRight,
    nintendoSwitchJoyconPair,
    gamecube,
    count,
};

using typeUnderlying_t = std::underlying_type_t< type_t >;

[[nodiscard]] constexpr auto toLegacy( type_t _value ) -> SDL_GamepadType {
    return ( static_cast< SDL_GamepadType >( _value ) );
}

[[nodiscard]] constexpr auto toLegacy( type_t* _value ) -> SDL_GamepadType* {
    return ( std::bit_cast< SDL_GamepadType* >( _value ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_GamepadType _value ) -> type_t {
    return ( static_cast< type_t >( _value ) );
}

// The list of buttons available on a gamepad
//
// For controllers that use a diamond pattern for the face buttons, the
// south/east/west/north buttons below correspond to the locations in the
// diamond pattern. For Xbox controllers, this would be A/B/X/Y, for Nintendo
// Switch controllers, this would be B/A/Y/X, for GameCube controllers this
// would be A/X/B/Y, for PlayStation controllers this would be
// Cross/Circle/Square/Triangle.
//
// For controllers that don't use a diamond pattern for the face buttons, the
// south/east/west/north buttons indicate the buttons labeled A, B, C, D, or
// 1, 2, 3, 4, or for controllers that aren't labeled, they are the primary,
// secondary, etc. buttons.
//
// The activate action is often the south button and the cancel action is
// often the east button, but in some regions this is reversed, so your game
// should allow remapping actions based on user preferences.
//
// You can query the labels for the face buttons using
// SDL_GetGamepadButtonLabel()
using button_t = enum class button : int8_t {
    invalid = -1,
    south, /**< Bottom face button (e.g. Xbox A button) */
    east,  /**< Right face button (e.g. Xbox B button) */
    west,  /**< Left face button (e.g. Xbox X button) */
    north, /**< Top face button (e.g. Xbox Y button) */
    back,
    guide,
    start,
    leftStick,
    rightStick,
    leftShoulder,
    rightShoulder,
    dpadUp,
    dpadDown,
    dpadLeft,
    dpadRight,
    misc1,        /**< Additional button (e.g. Xbox Series X share
                                        button, PS5 microphone button, Nintendo Switch
                                        Pro capture button, Amazon Luna microphone
                                        button, Google Stadia capture button) */
    rightPaddle1, /**< Upper or primary paddle, under your
                                         right hand (e.g. Xbox Elite paddle P1)
                                       */
    leftPaddle1,  /**< Upper or primary paddle, under your
                                         left hand (e.g. Xbox Elite paddle P3) */
    rightPaddle2, /**< Lower or secondary paddle, under your
                                         right hand (e.g. Xbox Elite paddle P2)
                                       */
    leftPaddle2,  /**< Lower or secondary paddle, under your
                                         left hand (e.g. Xbox Elite paddle P4) */
    touchpad,     /**< PS4/PS5 touchpad button */
    misc2,        /**< Additional button */
    misc3,        /**< Additional button */
    misc4,        /**< Additional button */
    misc5,        /**< Additional button */
    misc6,        /**< Additional button */
    count,
};

using buttonUnderlying_t = std::underlying_type_t< button_t >;

[[nodiscard]] constexpr auto toLegacy( button_t _value ) -> SDL_GamepadButton {
    return ( static_cast< SDL_GamepadButton >( _value ) );
}

[[nodiscard]] constexpr auto toLegacy( button_t* _value )
    -> SDL_GamepadButton* {
    return ( std::bit_cast< SDL_GamepadButton* >( _value ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_GamepadButton _value )
    -> button_t {
    return ( static_cast< button_t >( _value ) );
}

// The set of gamepad button labels
//
// This isn't a complete set, just the face buttons to make it easy to show
// button prompts.
//
// For a complete set, you should look at the button and gamepad type and have
// a set of symbols that work well with your art style.
using buttonLabel_t = enum class buttonLabel : uint8_t {
    unknown,
    a,
    b,
    x,
    y,
    cross,
    circle,
    square,
    triangle,
};

using buttonLabelUnderlying_t = std::underlying_type_t< buttonLabel_t >;

[[nodiscard]] constexpr auto toLegacy( buttonLabel_t _value )
    -> SDL_GamepadButtonLabel {
    return ( static_cast< SDL_GamepadButtonLabel >( _value ) );
}

[[nodiscard]] constexpr auto toLegacy( buttonLabel_t* _value )
    -> SDL_GamepadButtonLabel* {
    return ( std::bit_cast< SDL_GamepadButtonLabel* >( _value ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_GamepadButtonLabel _value )
    -> buttonLabel_t {
    return ( static_cast< buttonLabel_t >( _value ) );
}

// The list of axes available on a gamepad
//
// Thumbstick axis values range from SDL_JOYSTICK_AXIS_MIN to
// SDL_JOYSTICK_AXIS_MAX, and are centered within ~8000 of zero, though
// advanced UI will allow users to set or autodetect the dead zone, which
// varies between gamepads.
//
// Trigger axis values range from 0 (released) to SDL_JOYSTICK_AXIS_MAX (fully
// pressed) when reported by SDL_GetGamepadAxis(). Note that this is not the
// same range that will be reported by the lower-level SDL_GetJoystickAxis().
using axis_t = enum class axis : int8_t {
    invalid = -1,
    leftX,
    leftY,
    rightX,
    rightY,
    leftTrigger,
    rightTrigger,
    count,
};

using axisUnderlying_t = std::underlying_type_t< axis_t >;

[[nodiscard]] constexpr auto toLegacy( axis_t _value ) -> SDL_GamepadAxis {
    return ( static_cast< SDL_GamepadAxis >( _value ) );
}

[[nodiscard]] constexpr auto toLegacy( axis_t* _value ) -> SDL_GamepadAxis* {
    return ( std::bit_cast< SDL_GamepadAxis* >( _value ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_GamepadAxis _value ) -> axis_t {
    return ( static_cast< axis_t >( _value ) );
}

// Types of gamepad control bindings.
//
// A gamepad is a collection of bindings that map arbitrary joystick buttons,
// axes and hat switches to specific positions on a generic console-style
// gamepad. This enum is used as part of SDL_GamepadBinding to specify those
// mappings.
using bindingType_t = enum class bindingType : uint8_t {
    none = 0,
    button,
    axis,
    hat,
};

using bindingTypeUnderlying_t = std::underlying_type_t< bindingType_t >;

[[nodiscard]] constexpr auto toLegacy( bindingType_t _value )
    -> SDL_GamepadBindingType {
    return ( static_cast< SDL_GamepadBindingType >( _value ) );
}

[[nodiscard]] constexpr auto toLegacy( bindingType_t* _value )
    -> SDL_GamepadBindingType* {
    return ( std::bit_cast< SDL_GamepadBindingType* >( _value ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_GamepadBindingType _value )
    -> bindingType_t {
    return ( static_cast< bindingType_t >( _value ) );
}

// A mapping between one joystick input to a gamepad control.
//
// A gamepad has a collection of several bindings, to say, for example, when
// joystick button number 5 is pressed, that should be treated like the
// gamepad's "start" button.
//
// SDL has these bindings built-in for many popular controllers, and can add
// more with a simple text string. Those strings are parsed into a collection
// of these structs to make it easier to operate on the data.
using binding_t = gsl::not_null< SDL_GamepadBinding* >;

#if 0
// TODO: Rename
namespace cap {

constexpr auto g_monoLEDBoolean = joystick::cap::g_monoLEDBoolean;
constexpr auto g_LEDRGBBoolean = joystick::cap::g_LEDRGBBoolean;
constexpr auto g_playerLEDBoolean = joystick::cap::g_playerLEDBoolean;
constexpr auto g_rumbleBoolean = joystick::cap::g_rumbleBoolean;
constexpr auto g_rumbleTriggerBoolean = joystick::cap::g_rumbleTriggerBoolean;

} // namespace cap
#endif

using gamepad_t = struct gamepad {
    using native_t = SDL_Gamepad*;

    gamepad() = delete;

    // Open a gamepad for use.
    //
    // Get the SDL_Gamepad associated with a joystick instance ID, if it has
    // been opened.
    gamepad( joystick::id_t _id, bool _isOpened )
        : _data( ( _isOpened ) ? ( SDL_GetGamepadFromID( _id ) )
                               : ( SDL_OpenGamepad( _id ) ) ) {}

    // Get the SDL_Gamepad associated with a player index.
    gamepad( size_t _playerIndex )
        : _data( SDL_GetGamepadFromPlayerIndex( _playerIndex ) ) {}

    gamepad( const gamepad& ) = default;
    gamepad( gamepad&& ) = default;

    template < typename OtherType >
        requires std::is_convertible_v< OtherType, native_t >
    constexpr gamepad( OtherType&& _other )
        : _data( std::forward< OtherType >( _other ) ) {}

    ~gamepad() = default;
    auto operator=( const gamepad& ) -> gamepad& = default;
    auto operator=( gamepad&& ) -> gamepad& = default;

    [[nodiscard]] constexpr operator native_t() const { return ( _data ); }

    // Get the current mapping of a gamepad.
    //
    // Details about mappings are discussed with SDL_AddGamepadMapping().
    [[nodiscard]] auto mapping() -> std::string;

    // Get the properties associated with an opened gamepad.
    //
    // These properties are shared with the underlying joystick object.
    //
    // The following read-only properties are provided by SDL:
    //
    // - `SDL_PROP_GAMEPAD_CAP_MONO_LED_BOOLEAN`: true if this gamepad has an
    // LED that has adjustable brightness
    // - `SDL_PROP_GAMEPAD_CAP_RGB_LED_BOOLEAN`: true if this gamepad has an LED
    //   that has adjustable color
    // - `SDL_PROP_GAMEPAD_CAP_PLAYER_LED_BOOLEAN`: true if this gamepad has a
    //   player LED
    // - `SDL_PROP_GAMEPAD_CAP_RUMBLE_BOOLEAN`: true if this gamepad has
    //   left/right rumble
    // - `SDL_PROP_GAMEPAD_CAP_TRIGGER_RUMBLE_BOOLEAN`: true if this gamepad has
    //   simple trigger rumble
    [[nodiscard]] auto properties() -> properties::id_t {
        const properties::id_t l_result = SDL_GetGamepadProperties( _data );

        assert( l_result );

        return ( l_result );
    }

    // Get the instance ID of an opened gamepad.
    [[nodiscard]] auto id() -> joystick::id_t {
        const joystick::id_t l_result = SDL_GetGamepadID( _data );

        assert( l_result );

        return ( l_result );
    }

    // Get the implementation-dependent name for an opened gamepad.
    [[nodiscard]] auto name() -> std::string_view {
        return { gsl::make_not_null( SDL_GetGamepadName( _data ) ) };
    }

    // Get the implementation-dependent path for an opened gamepad.
    [[nodiscard]] auto path() -> std::string_view {
        return { gsl::make_not_null( SDL_GetGamepadPath( _data ) ) };
    }

    // Get the type of an opened gamepad.
    [[nodiscard]] auto type() -> type_t {
        const type_t l_result = fromLegacy( SDL_GetGamepadType( _data ) );

        assert( l_result != type_t::unknown );

        return ( l_result );
    }

    // Get the type of an opened gamepad, ignoring any mapping override.
    [[nodiscard]] auto realType() -> type_t {
        const type_t l_result = fromLegacy( SDL_GetRealGamepadType( _data ) );

        assert( l_result != type_t::unknown );

        return ( l_result );
    }

    // Get the player index of an opened gamepad.
    //
    // For XInput gamepads this returns the XInput user index.
    //
    // -1 if it's not available.
    [[nodiscard]] auto playerIndex() -> size_t {
        const ssize_t l_result = SDL_GetGamepadPlayerIndex( _data );

        assert( l_result );

        return ( l_result );
    }

    // Set the player index of an opened gamepad.
    //
    // NULL to clear the player index and turn off player LEDs.
    void playerIndex( std::optional< size_t > _playerIndex ) {
        bool l_result = false;

        if ( _playerIndex ) {
            l_result = SDL_SetGamepadPlayerIndex( _data, _playerIndex.value() );

        } else {
            l_result = SDL_SetGamepadPlayerIndex( _data, -1 );
        }

        assert( l_result );
    }

    // Get the USB vendor ID of an opened gamepad, if available.
    //
    // If the vendor ID isn't available this function returns 0.
    //
    // 0 if unavailable.
    [[nodiscard]] auto vendor() -> uint16_t {
        return ( SDL_GetGamepadVendor( _data ) );
    }

    // Get the USB product ID of an opened gamepad, if available.
    //
    // If the product ID isn't available this function returns 0.
    //
    // 0 if unavailable.
    [[nodiscard]] auto product() -> uint16_t {
        return ( SDL_GetGamepadProduct( _data ) );
    }

    // Get the product version of an opened gamepad, if available.
    //
    // If the product version isn't available this function returns 0.
    //
    // 0 if unavailable.
    [[nodiscard]] auto etGamepadProductVersion() -> uint16_t {
        return ( SDL_GetGamepadProductVersion( _data ) );
    }

    // Get the firmware version of an opened gamepad, if available.
    //
    // If the firmware version isn't available this function returns 0.
    //
    // 0 if unavailable.
    [[nodiscard]] auto firmwareVersion() -> uint16_t {
        return ( SDL_GetGamepadFirmwareVersion( _data ) );
    }

    // Get the serial number of an opened gamepad, if available.
    //
    // Returns the serial number of the gamepad, or NULL if it is not available.
    [[nodiscard]] auto serial() -> std::optional< std::string_view > {
        const char* l_result = SDL_GetGamepadSerial( _data );

        if ( l_result ) {
            return ( l_result );

        } else {
            return ( std::nullopt );
        }
    }

    // Get the Steam Input handle of an opened gamepad, if available.
    //
    // Returns an InputHandle_t for the gamepad that can be used with Steam
    // Input API: https://partner.steamgames.com/doc/api/ISteamInput
    //
    // 0 if unavailable.
    [[nodiscard]] auto steamHandle() -> uint64_t {
        return ( SDL_GetGamepadSteamHandle( _data ) );
    }

    // Get the connection state of a gamepad.
    [[nodiscard]] auto etGamepadConnectionState()
        -> joystick::connectionState_t {
        const joystick::connectionState_t l_result =
            joystick::fromLegacy( SDL_GetGamepadConnectionState( _data ) );

        assert( l_result != joystick::connectionState_t::invalid );

        return ( l_result );
    }

    // Get the battery state of a gamepad.
    //
    // You should never take a battery status as absolute truth. Batteries
    // (especially failing batteries) are delicate hardware, and the values
    // reported here are best estimates based on what that hardware reports.
    // It's not uncommon for older batteries to lose stored power much faster
    // than it reports, or completely drain when reporting it has 20 percent
    // left, etc.
    //
    // The percentage of battery life left, between 0 and 100.
    // This will be filled in with -1 we can't determine a value or there is no
    // battery.
    [[nodiscard]] auto powerInfo() -> std::pair< power::state_t, ssize_t > {
        ssize_t l_percent = 0;

        const power::state_t l_result =
            power::fromLegacy( SDL_GetGamepadPowerInfo(
                _data, std::bit_cast< int* >( &l_percent ) ) );

        assert( l_result != power::state_t::error );

        return { l_result, l_percent };
    }

    // Check if a gamepad has been opened and is currently connected.
    [[nodiscard]] auto isConnected() -> bool {
        return ( SDL_GamepadConnected( _data ) );
    }

    // Get the underlying joystick from a gamepad.
    //
    // This function will give you a SDL_Joystick object, which allows you to
    // use the SDL_Joystick functions with a SDL_Gamepad object. This would be
    // useful for getting a joystick's position at any given time, even if it
    // hasn't moved (moving it would produce an event, which would have the
    // axis' value).
    //
    // The pointer returned is owned by the SDL_Gamepad. You should not call
    // SDL_CloseJoystick() on it, for example, since doing so will likely cause
    // SDL to crash.
    [[nodiscard]] auto joystick() -> joystick::joystick_t {
        return ( SDL_GetGamepadJoystick( _data ) );
    }

    // Get the SDL joystick layer bindings for a gamepad.
    [[nodiscard]] auto bindings() -> std::vector< binding_t >;

    // Query whether a gamepad has a given axis.
    //
    // This merely reports whether the gamepad's mapping defined this axis, as
    // that is all the information SDL has about the physical device.
    [[nodiscard]] auto hasAxis( axis_t _axis ) -> bool {
        return ( SDL_GamepadHasAxis( _data, toLegacy( _axis ) ) );
    }

    // Get the current state of an axis control on a gamepad.
    //
    // The axis indices start at index 0.
    //
    // For thumbsticks, the state is a value ranging from -32768 (up/left) to
    // 32767 (down/right).
    //
    // Triggers range from 0 when released to 32767 when fully pressed, and
    // never return a negative value. Note that this differs from the value
    // reported by the lower-level SDL_GetJoystickAxis(), which normally uses
    // the full range.
    //
    // Note that for invalid gamepads or axes, this will return 0. Zero is also
    // a valid value in normal operation; usually it means a centered axis.
    [[nodiscard]] auto axis( axis_t _axis ) -> int16_t {
        return ( SDL_GetGamepadAxis( _data, toLegacy( _axis ) ) );
    }

    // Query whether a gamepad has a given button.
    //
    // This merely reports whether the gamepad's mapping defined this button, as
    // that is all the information SDL has about the physical device.
    [[nodiscard]] auto hasButton( button_t _button ) -> bool {
        return ( SDL_GamepadHasButton( _data, toLegacy( _button ) ) );
    }

    // Get the current state of a button on a gamepad.
    [[nodiscard]] auto isPressed( button_t _button ) -> bool {
        return ( SDL_GetGamepadButton( _data, toLegacy( _button ) ) );
    }

    // Get the label of a button on a gamepad.
    [[nodiscard]] auto buttonLabel( button_t _button ) -> buttonLabel_t {
        return ( fromLegacy(
            SDL_GetGamepadButtonLabel( _data, toLegacy( _button ) ) ) );
    }

    // Get the number of touchpads on a gamepad.
    [[nodiscard]] auto touchpadsAmount() -> ssize_t {
        return ( SDL_GetNumGamepadTouchpads( _data ) );
    }

    // Get the number of supported simultaneous fingers on a touchpad on a game
    // gamepad.
    [[nodiscard]] auto touchpadFingers( ssize_t _touchpad ) -> ssize_t {
        return ( SDL_GetNumGamepadTouchpadFingers( _data, _touchpad ) );
    }

    // Get the current state of a finger on a touchpad on a gamepad.
    //
    // Position, normalized 0 to 1, with the origin in the upper left.
    [[nodiscard]] auto touchpadFinger( ssize_t _touchpad, ssize_t _finger )
        -> std::tuple< bool, point_t< float >, float > {
        bool l_isDown = false;
        point_t< float > l_point;
        float l_pressure = NAN;

        const bool l_result =
            SDL_GetGamepadTouchpadFinger( _data, _touchpad, _finger, &l_isDown,
                                          &l_point.x, &l_point.y, &l_pressure );

        assert( l_result );

        return { l_isDown, l_point, l_pressure };
    }

    // Return whether a gamepad has a particular sensor.
    [[nodiscard]] auto hasSensor( sensors::type_t _type ) -> bool {
        return ( SDL_GamepadHasSensor( _data, sensors::toLegacy( _type ) ) );
    }

    // Set whether data reporting for a gamepad sensor is enabled.
    void sensorToggle( sensors::type_t _type, bool _isEnabled ) {
        const bool l_result = SDL_SetGamepadSensorEnabled(
            _data, sensors::toLegacy( _type ), _isEnabled );

        assert( l_result );
    }

    // Query whether sensor data reporting is enabled for a gamepad.
    [[nodiscard]] auto isEnabled( sensors::type_t _type ) -> bool {
        return (
            SDL_GamepadSensorEnabled( _data, sensors::toLegacy( _type ) ) );
    }

    // Get the data rate (number of events per second) of a gamepad sensor.
    //
    // 0.0f if the data rate is not available.
    [[nodiscard]] auto sensorDataRate( sensors::type_t _type ) -> float {
        return SDL_GetGamepadSensorDataRate( _data,
                                             sensors::toLegacy( _type ) );
    }

    // Get the current state of a gamepad sensor.
    //
    // The number of values and interpretation of the data is sensor dependent.
    // See SDL_sensor.h for the details for each type of sensor.
    //
    // The number of values to write to data.
    template < size_t N >
        requires( N > 0 )
    [[nodiscard]] auto sensorData( sensors::type_t _type )
        -> std::array< float, N > {
        std::array< float, N > l_data{};

        const bool l_result = SDL_GetGamepadSensorData(
            _data, sensors::toLegacy( _type ), l_data.data(), l_data.size() );

        assert( l_result );

        return ( l_data );
    }

    // Start a rumble effect on a gamepad.
    //
    // Each call to this function cancels any previous rumble effect, and
    // calling it with 0 intensity stops any rumbling.
    //
    // This function requires you to process SDL events or call
    // SDL_UpdateJoysticks() to update rumble state.
    //
    // The intensity of the low frequency (left) rumble motor, from 0 to 0xFFFF.
    //
    // The intensity of the high frequency (right) rumble motor, from 0 to
    // 0xFFFF.
    //
    // The duration of the rumble effect, in milliseconds.
    void rumble( uint16_t _lowFrequencyRumble,
                 uint16_t _highFrequencyRumble,
                 std::chrono::milliseconds _duration ) {
        const bool l_result =
            SDL_RumbleGamepad( _data, _lowFrequencyRumble, _highFrequencyRumble,
                               _duration.count() );

        assert( l_result );
    }

    // Start a rumble effect in the gamepad's triggers.
    //
    // Each call to this function cancels any previous trigger rumble effect,
    // and calling it with 0 intensity stops any rumbling.
    //
    // Note that this is rumbling of the _triggers_ and not the gamepad as a
    // whole. This is currently only supported on Xbox One gamepads. If you want
    // the (more common) whole-gamepad rumble, use SDL_RumbleGamepad() instead.
    //
    // This function requires you to process SDL events or call
    // SDL_UpdateJoysticks() to update rumble state.
    //
    // The intensity of the left trigger rumble motor, from 0 to 0xFFFF.
    //
    // The intensity of the right trigger rumble motor, from 0 to 0xFFFF.
    //
    // The duration of the rumble effect, in milliseconds.
    void rumbleTriggers( uint16_t _leftRumble,
                         uint16_t _rightRumble,
                         std::chrono::milliseconds _duration ) {
        const bool l_result = SDL_RumbleGamepadTriggers(
            _data, _leftRumble, _rightRumble, _duration.count() );

        assert( l_result );
    }

    // Update a gamepad's LED color.
    //
    // An example of a joystick LED is the light on the back of a PlayStation
    // 4's DualShock 4 controller.
    //
    // For gamepads with a single color LED, the maximum of the RGB values will
    // be used as the LED brightness.
    //
    // Alpha is ignored.
    void LED( color_t _color ) {
        const bool l_result =
            SDL_SetGamepadLED( _data, _color.red, _color.green, _color.blue );

        assert( l_result );
    }

    // Send a gamepad specific effect packet.
    void effect( std::span< const std::byte > _effect ) {
        SDL_SendGamepadEffect( _data, _effect.data(), _effect.size() );
    }

    // Close a gamepad previously opened with SDL_OpenGamepad().
    void close() { SDL_CloseGamepad( _data ); }

    struct appleSfSymbols {
        // Return the sfSymbolsName for a given button on a gamepad on Apple
        // platforms.
        [[nodiscard]] auto name( const gamepad& _gamepad, button_t _button )
            -> std::string_view {
            return {
                gsl::make_not_null( SDL_GetGamepadAppleSFSymbolsNameForButton(
                    _gamepad, toLegacy( _button ) ) ) };
        }

        // Return the sfSymbolsName for a given axis on a gamepad on Apple
        // platforms.
        [[nodiscard]] auto name( const gamepad& _gamepad, axis_t _axis )
            -> std::string_view {
            return {
                gsl::make_not_null( SDL_GetGamepadAppleSFSymbolsNameForAxis(
                    _gamepad, toLegacy( _axis ) ) ) };
        }
    };

    friend struct appleSfSymbols;

    // Variables
private:
    gsl::not_null< native_t > _data;
};

// Add support for gamepads that SDL is unaware of or change the binding of an
// existing gamepad.
//
// The mapping string has the format "GUID,name,mapping", where GUID is the
// string value from SDL_GUIDToString(), name is the human readable string for
// the device and mappings are gamepad mappings to joystick ones. Under
// Windows there is a reserved GUID of "xinput" that covers all XInput
// devices. The mapping format for joystick is:
//
// - `bX`: a joystick button, index X
// - `hX.Y`: hat X with value Y
// - `aX`: axis X of the joystick
//
// Buttons can be used as a gamepad axes and vice versa.
//
// If a device with this GUID is already plugged in, SDL will generate an
// SDL_EVENT_GAMEPAD_ADDED event.
//
// This string shows an example of a valid mapping for a gamepad:
//
// ```c
// "341a3608000000000000504944564944,Afterglow PS3
// Controller,a:b1,b:b2,y:b3,x:b0,start:b9,guide:b12,back:b8,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftshoulder:b4,rightshoulder:b5,leftstick:b10,rightstick:b11,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:b6,righttrigger:b7"
// ```
//
// true if a new mapping is added, false if an existing mapping is updated.
[[nodiscard]] inline auto mapping( std::string_view _mapping ) -> bool {
    const int l_result =
        SDL_AddGamepadMapping( std::string( _mapping ).c_str() );

    assert( l_result == -1 );

    return ( l_result );
}

// Load a set of gamepad mappings from an SDL_IOStream.
//
// You can call this function several times, if needed, to load different
// database files.
//
// If a new mapping is loaded for an already known gamepad GUID, the later
// version will overwrite the one currently loaded.
//
// Any new mappings for already plugged in controllers will generate
// SDL_EVENT_GAMEPAD_ADDED events.
//
// Mappings not belonging to the current platform or with no platform field
// specified will be ignored (i.e. mappings for Linux will be ignored in
// Windows, etc).
//
// This function will load the text database entirely in memory before
// processing it, so take this into consideration if you are in a memory
// constrained environment.
//
// if true, calls SDL_CloseIO() on `src` before returning, even in the case of
// an error.
[[nodiscard]] inline auto mappings( SDL_IOStream& _source, bool _closeIO )
    -> size_t {
    const int l_result = SDL_AddGamepadMappingsFromIO( &_source, _closeIO );

    assert( l_result == -1 );

    return ( l_result );
}

// Load a set of gamepad mappings from a file.
//
// You can call this function several times, if needed, to load different
// database files.
//
// If a new mapping is loaded for an already known gamepad GUID, the later
// version will overwrite the one currently loaded.
//
// Any new mappings for already plugged in controllers will generate
// SDL_EVENT_GAMEPAD_ADDED events.
//
// Mappings not belonging to the current platform or with no platform field
// specified will be ignored (i.e. mappings for Linux will be ignored in
// Windows, etc).
[[nodiscard]] inline auto mappings( std::string_view _path ) -> size_t {
    const int l_result =
        SDL_AddGamepadMappingsFromFile( std::string( _path ).c_str() );

    assert( l_result == -1 );

    return ( l_result );
}

// Reinitialize the SDL mapping database to its initial state.
//
// This will generate gamepad events as needed if device mappings change.
inline void reloadMappings() {
    const bool l_result = SDL_ReloadGamepadMappings();

    assert( l_result );
}

// Get the current gamepad mappings.
[[nodiscard]] auto mappings() -> std::vector< std::string >;

// Get the gamepad mapping string for a given GUID.
[[nodiscard]] auto mapping( GUID_t _GUID ) -> std::string;

// Set the current mapping of a joystick or gamepad.
//
// Details about mappings are discussed with SDL_AddGamepadMapping().
//
// The mapping to use for this device, or NULL to clear the mapping.
inline void mapping(
    joystick::id_t _id,
    std::optional< std::string_view > _mapping = std::nullopt ) {
    if ( _mapping ) {
        SDL_SetGamepadMapping( _id, std::string( _mapping.value() ).c_str() );

    } else {
        SDL_SetGamepadMapping( _id, nullptr );
    }
}

// Return whether a gamepad is currently connected.
[[nodiscard]] inline auto hasAny() -> bool {
    return ( SDL_HasGamepad() );
}

// Get a list of currently connected gamepads.
[[nodiscard]] auto all() -> std::vector< joystick::id_t >;

// Check if the given joystick is supported by the gamepad interface.
//
// true if the given joystick is supported by the gamepad interface, false if it
// isn't or it's an invalid index.
// FIX: Maybe assert
[[nodiscard]] inline auto isAny( joystick::id_t _id ) -> bool {
    return ( SDL_IsGamepad( _id ) );
}

// Get the implementation dependent name of a gamepad.
//
// This can be called before any gamepads are opened.
[[nodiscard]] inline auto name( joystick::id_t _id ) -> std::string_view {
    return { gsl::make_not_null( SDL_GetGamepadNameForID( _id ) ) };
}

// Get the implementation dependent path of a gamepad.
//
// This can be called before any gamepads are opened.
[[nodiscard]] inline auto path( joystick::id_t _id ) -> std::string_view {
    return { gsl::make_not_null( SDL_GetGamepadPathForID( _id ) ) };
}

// Get the player index of a gamepad.
//
// This can be called before any gamepads are opened.
//
// The player index of a gamepad, or -1 if it's not available.
[[nodiscard]] inline auto playerIndex( joystick::id_t _id ) -> ssize_t {
    return ( SDL_GetGamepadPlayerIndexForID( _id ) );
}

// Get the implementation-dependent GUID of a gamepad.
//
// This can be called before any gamepads are opened.
[[nodiscard]] inline auto GUID( joystick::id_t _id ) -> GUID_t {
    const GUID_t l_result = SDL_GetGamepadGUIDForID( _id );

    assert( !l_result.empty() );

    return ( l_result );
}

// Get the USB vendor ID of a gamepad, if available.
//
// This can be called before any gamepads are opened. If the vendor ID isn't
// available this function asserts.
[[nodiscard]] inline auto vendor( joystick::id_t _id ) -> uint16_t {
    const uint16_t l_result = SDL_GetGamepadVendorForID( _id );

    assert( l_result );

    return ( l_result );
}

// Get the USB product ID of a gamepad, if available.
//
// This can be called before any gamepads are opened. If the product ID
// isn't available this function asserts.
[[nodiscard]] inline auto product( joystick::id_t _id ) -> uint16_t {
    const uint16_t l_result = SDL_GetGamepadProductForID( _id );

    assert( l_result );

    return ( l_result );
}

// Get the product version of a gamepad, if available.
//
// This can be called before any gamepads are opened. If the product version
// isn't available this function returns 0.
[[nodiscard]] inline auto productVersion( joystick::id_t _id ) -> uint16_t {
    const uint16_t l_result = SDL_GetGamepadProductVersionForID( _id );

    assert( l_result );

    return ( l_result );
}

// Get the type of a gamepad.
//
// This can be called before any gamepads are opened.
[[nodiscard]] inline auto type( joystick::id_t _id ) -> type_t {
    return ( fromLegacy( SDL_GetGamepadTypeForID( _id ) ) );
}

// Get the type of a gamepad, ignoring any mapping override.
//
// This can be called before any gamepads are opened.
[[nodiscard]] inline auto realType( joystick::id_t _id ) -> type_t {
    return ( fromLegacy( SDL_GetRealGamepadTypeForID( _id ) ) );
}

// Get the mapping of a gamepad.
//
// This can be called before any gamepads are opened.
[[nodiscard]] auto mapping( joystick::id_t _id ) -> std::string;

// Set the state of gamepad event processing.
//
// If gamepad events are disabled, you must call SDL_UpdateGamepads()
// yourself and check the state of the gamepad when you want gamepad
// information.
inline void eventsToggle( bool _isEnabled ) {
    SDL_SetGamepadEventsEnabled( _isEnabled );
}

// Query the state of gamepad event processing.
//
// If gamepad events are disabled, you must call SDL_UpdateGamepads()
// yourself and check the state of the gamepad when you want gamepad
// information.
[[nodiscard]] inline auto areEventsEnabled() -> bool {
    return ( SDL_GamepadEventsEnabled() );
}

// Manually pump gamepad updates if not using the loop.
//
// This function is called automatically by the event loop if events are
// enabled. Under such circumstances, it will not be necessary to call this
// function.
inline void update() {
    SDL_UpdateGamepads();
}

// Convert a string into type_t enum.
//
// This function is called internally to translate SDL_Gamepad mapping
// strings for the underlying joystick device into the consistent
// SDL_Gamepad mapping. You do not normally need to call this function
// unless you are parsing SDL_Gamepad mappings in your own code.
[[nodiscard]] inline auto type( std::string_view _string ) -> type_t {
    const type_t l_result = fromLegacy(
        SDL_GetGamepadTypeFromString( std::string( _string ).c_str() ) );

    assert( l_result != type_t::unknown );

    return ( l_result );
}

// Convert from an type_t enum to a string.
//
// The string returned is of the format used by SDL_Gamepad mapping strings.
[[nodiscard]] inline auto toString( type_t _type ) -> std::string_view {
    const char* l_result = SDL_GetGamepadStringForType( toLegacy( _type ) );

    assert( l_result );

    return { l_result };
}

// Convert a string into axis_t enum.
//
// This function is called internally to translate SDL_Gamepad mapping
// strings for the underlying joystick device into the consistent
// SDL_Gamepad mapping. You do not normally need to call this function
// unless you are parsing SDL_Gamepad mappings in your own code.
//
// Note specially that "righttrigger" and "lefttrigger" map to
// `SDL_GAMEPAD_AXIS_RIGHT_TRIGGER` and `SDL_GAMEPAD_AXIS_LEFT_TRIGGER`,
// respectively.
//
// String representing a SDL_Gamepad axis.
//
// The axis_t enum corresponding to the input string.
[[nodiscard]] inline auto axis( std::string_view _string ) -> axis_t {
    const axis_t l_result = fromLegacy(
        SDL_GetGamepadAxisFromString( std::string( _string ).c_str() ) );

    assert( l_result != axis_t::invalid );

    return ( l_result );
}

// Convert from an axis_t enum to a string.
//
// The string returned is of the format used by SDL_Gamepad mapping strings.
[[nodiscard]] inline auto toString( axis_t _axis ) -> std::string_view {
    return { gsl::make_not_null(
        SDL_GetGamepadStringForAxis( toLegacy( _axis ) ) ) };
}

// Convert a string into an button_t enum.
//
// This function is called internally to translate SDL_Gamepad mapping
// strings for the underlying joystick device into the consistent
// SDL_Gamepad mapping. You do not normally need to call this function
// unless you are parsing SDL_Gamepad mappings in your own code.
[[nodiscard]] inline auto button( std::string_view _string ) -> button_t {
    const button_t l_result = fromLegacy(
        SDL_GetGamepadButtonFromString( std::string( _string ).c_str() ) );

    assert( l_result != button_t::invalid );

    return ( l_result );
}

// Convert from an button_t enum to a string.
//
// The string returned is of the format used by SDL_Gamepad mapping strings.
[[nodiscard]] inline auto toString( button_t _button ) -> std::string_view {
    return { gsl::make_not_null(
        SDL_GetGamepadStringForButton( toLegacy( _button ) ) ) };
}

// Get the label of a button on a gamepad.
[[nodiscard]] inline auto butotnLabel( type_t _type, button_t _button )
    -> buttonLabel_t {
    return ( fromLegacy( SDL_GetGamepadButtonLabelForType(
        toLegacy( _type ), toLegacy( _button ) ) ) );
}

} // namespace slickdl::gamepad
