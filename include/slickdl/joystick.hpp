#pragma once

#include <SDL3/SDL_joystick.h>

#include <cstdint>
#include <gsl/pointers>
#include <tuple>

#include "slickdl.hpp"
#include "slickdl/GUID.hpp"
#include "slickdl/color.hpp"
#include "slickdl/point.hpp"
#include "slickdl/power.hpp"
#include "slickdl/properties.hpp"
#include "slickdl/sensor.hpp"

// SDL joystick support.
//
// This is the lower-level joystick handling. If you want the simpler option,
// where what each button does is well-defined, you should use the gamepad API
// instead.
//
// The term "instance_id" is the current instantiation of a joystick device in
// the system, if the joystick is removed and then re-inserted then it will
// get a new instance_id, instance_id's are monotonically increasing
// identifiers of a joystick plugged in.
//
// The term "player_index" is the number assigned to a player on a specific
// controller. For XInput controllers this returns the XInput user index. Many
// joysticks will not be able to supply this information.
//
// SDL_GUID is used as a stable 128-bit identifier for a joystick device that
// does not change over time. It identifies class of the device (a X360 wired
// controller for example). This identifier is platform dependent.
//
// In order to use these functions, SDL_Init() must have been called with the
// SDL_INIT_JOYSTICK flag. This causes SDL to scan the system for joysticks,
// and load appropriate drivers.
//
// If you would like to receive joystick updates while the application is in
// the background, you should set the following hint before calling
// SDL_Init(): SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS
namespace slickdl::joystick {

// An enum of some common joystick types.
//
// In some cases, SDL can identify a low-level joystick as being a certain
// type of device, and will report it through SDL_GetJoystickType (or
// SDL_GetJoystickTypeForID).
//
// This is by no means a complete list of everything that can be plugged into
// a computer.
//
// You may refer to
// XInput Controller Types
// https://learn.microsoft.com/en-us/windows/win32/xinput/xinput-and-controller-subtypes
// table for a general understanding of each joystick type.
using type_t = enum class type : uint8_t {
    unknown,
    gamepad,
    wheel,
    arcadeStick,
    flightStick,
    dancePad,
    guitar,
    drumKit,
    arcadePad,
    throttle,
    count,
};

using typeUnderlying_t = std::underlying_type_t< type_t >;

[[nodiscard]] constexpr auto toLegacy( type_t _value ) -> SDL_JoystickType {
    return ( static_cast< SDL_JoystickType >( _value ) );
}

[[nodiscard]] constexpr auto toLegacy( type_t* _value ) -> SDL_JoystickType* {
    return ( std::bit_cast< SDL_JoystickType* >( _value ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_JoystickType _value ) -> type_t {
    return ( static_cast< type_t >( _value ) );
}

// Possible connection states for a joystick device.
using connectionState_t = enum class connectionState : int8_t {
    invalid = -1,
    unknown,
    wired,
    wireless,
};

using connectionStateUnderlying_t = std::underlying_type_t< connectionState_t >;

[[nodiscard]] constexpr auto toLegacy( connectionState_t _value )
    -> SDL_JoystickConnectionState {
    return ( static_cast< SDL_JoystickConnectionState >( _value ) );
}

[[nodiscard]] constexpr auto toLegacy( connectionState_t* _value )
    -> SDL_JoystickConnectionState* {
    return ( std::bit_cast< SDL_JoystickConnectionState* >( _value ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_JoystickConnectionState _value )
    -> connectionState_t {
    return ( static_cast< connectionState_t >( _value ) );
}

// The structure that describes a virtual joystick touchpad.
using virtualTouchpadDescription_t =
    gsl::not_null< SDL_VirtualJoystickTouchpadDesc* >;

// The structure that describes a virtual joystick sensor.
using virtualSensorDescription_t =
    gsl::not_null< SDL_VirtualJoystickSensorDesc* >;

using virtualDescription_t = gsl::not_null< SDL_VirtualJoystickDesc* >;

// The structure that describes a virtual joystick.
//
// This structure should be initialized using SDL_INIT_INTERFACE(). All
// elements of this structure are optional.
#if 0
using virtualDescription_t = struct virtualDescription {
    using native_t = SDL_VirtualJoystickDesc;

    constexpr operator native_t*() { return ( _data ); }
    constexpr operator const native_t*() const { return ( _data ); }

    // Variables
private:
    gsl::not_null< native_t* > _data;
#if 0
    Uint32 _version;      /**< the version of this interface */
    uint16_t _type;       /**< `type_t` */
    uint16_t _padding;    /**< unused */
    uint16_t _vendorId;   /**< the USB vendor ID of this joystick */
    uint16_t _productId;  /**< the USB product ID of this joystick */
    uint16_t _naxes;      /**< the number of axes on this joystick */
    uint16_t _nbuttons;   /**< the number of buttons on this joystick */
    uint16_t _nballs;     /**< the number of balls on this joystick */
    uint16_t _nhats;      /**< the number of hats on this joystick */
    uint16_t _ntouchpads; /**< the number of touchpads on this joystick,
                          requires `touchpads` to point at valid descriptions */
    uint16_t _nsensors;   /**< the number of sensors on this joystick, requires
                          `sensors` to point at valid descriptions */
    std::array< uint16_t, 2 > _padding2; /**< unused */
    Uint32 _buttonMask; /**< A mask of which buttons are valid for this
                           controller   e.g. (1 << SDL_GAMEPAD_BUTTON_SOUTH) */
    Uint32 _axisMask;   /**< A mask of which axes are valid for this controller
                             e.g. (1 << SDL_GAMEPAD_AXIS_LEFTX) */
    const char* _name;  /**< the name of the joystick */
    const virtualTouchpadDescription_t*
        _touchpads; /**< A pointer to an array of touchpad descriptions,
                      required if `ntouchpads` is > 0 */
    const virtualSensorDescription_t*
        _sensors; /**< A pointer to an array of sensor descriptions, required if
                    `nsensors` is > 0 */

    void* _userdata; /**< User data pointer passed to callbacks */
    void ( *_update )( void* _userdata ); /**< Called when the joystick state
                                           should be updated */
    void ( *_setPlayerIndex )(
        void* _userdata,
        int _playerIndex ); /**< Called when the player index is set */
    bool ( *_rumble )(
        void* _userdata,
        uint16_t _lowFrequencyRumble,
        uint16_t _highFrequencyRumble ); /**< Implements SDL_RumbleJoystick() */
    bool ( *_rumbleTriggers )(
        void* _userdata,
        uint16_t _leftRumble,
        uint16_t _rightRumble ); /**< Implements SDL_RumbleJoystickTriggers() */
    bool ( *_setLed )( void* _userdata,
                       Uint8 _red,
                       Uint8 _green,
                       Uint8 _blue ); /**< Implements SDL_SetJoystickLED() */
    bool ( *_sendEffect )(
        void* _userdata,
        const void* _data,
        int _size ); /**< Implements SDL_SendJoystickEffect() */
    bool ( *_setSensorsEnabled )(
        void* _userdata,
        bool _enabled ); /**< Implements SDL_SetGamepadSensorEnabled() */
    void ( *_cleanup )( void* _userdata ); /**< Cleans up the userdata when the
                                            joystick is detached */
#endif
};
#endif

// This is a unique ID for a joystick for the time it is connected to the
// system, and is never reused for the lifetime of the application.
//
// If the joystick is disconnected and reconnected, it will get a new ID.
//
// The value 0 is an invalid ID.
using id_t = struct id {
    using native_t = uint32_t;

    id() = default;

    constexpr id( const native_t& _other ) : _data( _other ) {
        assert( _data );
    }

    // Attach a new virtual joystick.
    //
    // Description, initialized using SDL_INIT_INTERFACE().
    id( const virtualTouchpadDescription_t& _description )
        : _data( SDL_AttachVirtualJoystick(
              std::bit_cast< const SDL_VirtualJoystickDesc* >(
                  &_description ) ) ) {
        assert( _data );
    }

    id( const id& ) = default;
    id( id&& ) = default;

    template < typename OtherType >
        requires std::is_convertible_v< OtherType, native_t >
    constexpr id( OtherType&& _other )
        : _data( std::forward< OtherType >( _other ) ) {
        assert( _data );
    }

    ~id() = default;
    auto operator=( const id& ) -> id& = default;
    auto operator=( id&& ) -> id& = default;

    [[nodiscard]] constexpr operator native_t() const { return ( _data ); }

    // Get a list of currently connected joysticks.
    [[nodiscard]] static auto all() -> std::vector< id >;

    // Get the implementation dependent name of a joystick.
    //
    // This can be called before any joysticks are opened.
    [[nodiscard]] auto name() -> std::string_view {
        return { gsl::make_not_null( SDL_GetJoystickNameForID( _data ) ) };
    }

    // Get the implementation dependent path of a joystick.
    //
    // This can be called before any joysticks are opened.
    [[nodiscard]] auto path() -> std::string_view {
        return { gsl::make_not_null( SDL_GetJoystickPathForID( _data ) ) };
    }

    // Get the player index of a joystick.
    //
    // This can be called before any joysticks are opened.
    //
    // -1 if it's not available.
    [[nodiscard]] auto playerIndex() -> size_t {
        const ssize_t l_result = SDL_GetJoystickPlayerIndexForID( _data );

        assert( l_result != -1 );

        return ( l_result );
    }

    // Get the implementation-dependent GUID of a joystick.
    //
    // This can be called before any joysticks are opened.
    [[nodiscard]] auto GUID() -> GUID_t {
        const GUID_t l_result = SDL_GetJoystickGUIDForID( _data );

        assert( !l_result.empty() );

        return ( l_result );
    }

    // Get the USB vendor ID of a joystick, if available.
    //
    // This can be called before any joysticks are opened. If the vendor ID
    // isn't available this function asserts.
    [[nodiscard]] auto vendor() -> uint16_t {
        const uint16_t l_result = SDL_GetJoystickVendorForID( _data );

        assert( l_result );

        return ( l_result );
    }

    // Get the USB product ID of a joystick, if available.
    //
    // This can be called before any joysticks are opened. If the product ID
    // isn't available this function asserts.
    [[nodiscard]] auto product() -> uint16_t {
        const uint16_t l_result = SDL_GetJoystickProductForID( _data );

        assert( l_result );

        return ( l_result );
    }

    // Get the product version of a joystick, if available.
    //
    // This can be called before any joysticks are opened. If the product
    // version isn't available this function asserts.
    [[nodiscard]] auto productVersion() -> uint16_t {
        const uint16_t l_result = SDL_GetJoystickProductVersionForID( _data );

        assert( l_result );

        return ( l_result );
    }

    // Get the type of a joystick, if available.
    //
    // This can be called before any joysticks are opened.
    [[nodiscard]] auto type() -> type_t {
        const type_t l_result = fromLegacy( SDL_GetJoystickTypeForID( _data ) );

        assert( l_result != type_t::unknown );

        return ( l_result );
    }

    // Detach a virtual joystick.
    //
    // The joystick instance ID, previously returned from
    // SDL_AttachVirtualJoystick().
    void detach() {
        const bool l_result = SDL_DetachVirtualJoystick( _data );

        assert( l_result );
    }

    // Query whether or not a joystick is virtual.
    [[nodiscard]] auto isVirtual() -> bool {
        return ( SDL_IsJoystickVirtual( _data ) );
    }

    // Variables
private:
    native_t _data;
};

// The largest value an SDL_Joystick's axis can report.
constexpr size_t g_axisMax = 32767;

// The smallest value an SDL_Joystick's axis can report.
//
// This is a negative number!
constexpr ssize_t g_axisMin = -32768;

// TODO: Rename
namespace cap {
constexpr std::string_view g_monoLEDBoolean = "SDL.joystick.cap.mono_led";
constexpr std::string_view g_LEDRGBBoolean = "SDL.joystick.cap.rgb_led";
constexpr std::string_view g_playerLEDBoolean = "SDL.joystick.cap.player_led";
constexpr std::string_view g_rumbleBoolean = "SDL.joystick.cap.rumble";
constexpr std::string_view g_rumbleTriggerBoolean =
    "SDL.joystick.cap.trigger_rumble";
}; // namespace cap

constexpr size_t g_centered = 0U;
constexpr size_t g_up = 0x1U;
constexpr size_t g_right = 0x2U;
constexpr size_t g_down = 0x4U;
constexpr size_t g_left = 0x8U;
constexpr size_t g_rightUp = ( SDL_HAT_RIGHT | SDL_HAT_UP );
constexpr size_t g_rightDown = ( SDL_HAT_RIGHT | SDL_HAT_DOWN );
constexpr size_t g_leftUp = ( SDL_HAT_LEFT | SDL_HAT_UP );
constexpr size_t g_leftDown = ( SDL_HAT_LEFT | SDL_HAT_DOWN );

// This is opaque data.
using joystick_t = struct joystick {
    using native_t = SDL_Joystick*;

    joystick() = delete;

    // Open a joystick for use.
    //
    // The joystick subsystem must be initialized before a joystick can be
    // opened for use.
    //
    // Get the SDL_Joystick associated with an instance ID, if it has been
    // opened.
    joystick( id_t _id, bool _isOpened )
        : _data( ( _isOpened ) ? ( SDL_GetJoystickFromID( _id ) )
                               : ( SDL_OpenJoystick( _id ) ) ) {}

    // Get the SDL_Joystick associated with a player index.
    joystick( int _playerIndex )
        : _data( SDL_GetJoystickFromPlayerIndex( _playerIndex ) ) {}

    constexpr joystick( const native_t& _other ) : _data( _other ) {}

    joystick( const joystick& ) = default;
    joystick( joystick&& ) = default;

    template < typename OtherType >
        requires std::is_convertible_v< OtherType, native_t >
    constexpr joystick( OtherType&& _other )
        : _data( std::forward< OtherType >( _other ) ) {}

    ~joystick() = default;
    auto operator=( const joystick& ) -> joystick& = default;
    auto operator=( joystick&& ) -> joystick& = default;

    [[nodiscard]] constexpr operator native_t() const { return ( _data ); }

    // Set the state of an axis on an opened virtual joystick.
    //
    // Please note that values set here will not be applied until the next call
    // to SDL_UpdateJoysticks, which can either be called directly, or can be
    // called indirectly through various other SDL APIs, including, but not
    // limited to the following: SDL_PollEvent, SDL_PumpEvents,
    // SDL_WaitEventTimeout, SDL_WaitEvent.
    //
    // Note that when sending trigger axes, you should scale the value to the
    // full range of int16. For example, a trigger at rest would have the value
    // of `SDL_JOYSTICK_AXIS_MIN`.
    void virtualAxis( size_t _axisIndex, int16_t _value ) {
        const bool l_result =
            SDL_SetJoystickVirtualAxis( _data, _axisIndex, _value );

        assert( l_result );
    }

    // Generate ball motion on an opened virtual joystick.
    //
    // Please note that values set here will not be applied until the next call
    // to SDL_UpdateJoysticks, which can either be called directly, or can be
    // called indirectly through various other SDL APIs, including, but not
    // limited to the following: SDL_PollEvent, SDL_PumpEvents,
    // SDL_WaitEventTimeout, SDL_WaitEvent.
    void virtualBall( size_t _ballIndex, int16_t _offsetX, int16_t _offsetY ) {
        const bool l_result =
            SDL_SetJoystickVirtualBall( _data, _ballIndex, _offsetX, _offsetY );

        assert( l_result );
    }

    // Set the state of a button on an opened virtual joystick.
    //
    // Please note that values set here will not be applied until the next call
    // to SDL_UpdateJoysticks, which can either be called directly, or can be
    // called indirectly through various other SDL APIs, including, but not
    // limited to the following: SDL_PollEvent, SDL_PumpEvents,
    // SDL_WaitEventTimeout, SDL_WaitEvent.
    void virtualButton( size_t _buttonIndex, bool _isPressed ) {
        const bool l_result =
            SDL_SetJoystickVirtualButton( _data, _buttonIndex, _isPressed );

        assert( l_result );
    }

    // Set the state of a hat on an opened virtual joystick.
    //
    // Please note that values set here will not be applied until the next call
    // to SDL_UpdateJoysticks, which can either be called directly, or can be
    // called indirectly through various other SDL APIs, including, but not
    // limited to the following: SDL_PollEvent, SDL_PumpEvents,
    // SDL_WaitEventTimeout, SDL_WaitEvent.
    void virtualHat( size_t _hatIndex, uint8_t _value ) {
        const bool l_result =
            SDL_SetJoystickVirtualHat( _data, _hatIndex, _value );

        assert( l_result );
    }

    // Set touchpad finger state on an opened virtual joystick.
    //
    // Please note that values set here will not be applied until the next call
    // to SDL_UpdateJoysticks, which can either be called directly, or can be
    // called indirectly through various other SDL APIs, including, but not
    // limited to the following: SDL_PollEvent, SDL_PumpEvents,
    // SDL_WaitEventTimeout, SDL_WaitEvent.
    //
    // Coordinates of the finger on the touchpad, normalized 0 to 1,
    //          with the origin in the upper left.
    void virtualTouchpad( size_t _touchpadIndex,
                          size_t _fingerIndex,
                          bool _isPressed,
                          point_t< float > _point,
                          float _pressure ) {
        const bool l_result = SDL_SetJoystickVirtualTouchpad(
            _data, _touchpadIndex, _fingerIndex, _isPressed, _point.x, _point.y,
            _pressure );

        assert( l_result );
    }

    // Send a sensor update for an opened virtual joystick.
    //
    // Please note that values set here will not be applied until the next call
    // to SDL_UpdateJoysticks, which can either be called directly, or can be
    // called indirectly through various other SDL APIs, including, but not
    // limited to the following: SDL_PollEvent, SDL_PumpEvents,
    // SDL_WaitEventTimeout, SDL_WaitEvent.
    //
    // A 64-bit timestamp in nanoseconds associated with the sensor reading.
    //
    // The data associated with the sensor reading.
    void virtualSensorData( sensors::type_t _type,
                            uint64_t _sensorTimestamp,
                            std::span< const float > _sensorData ) {
        const bool l_result = SDL_SendJoystickVirtualSensorData(
            _data, sensors::toLegacy( _type ), _sensorTimestamp,
            _sensorData.data(), _sensorData.size() );

        assert( l_result );
    }

    // Get the properties associated with a joystick.
    //
    // The following read-only properties are provided by SDL:
    //
    // - `SDL_PROP_JOYSTICK_CAP_MONO_LED_BOOLEAN`: true if this joystick has an
    //   LED that has adjustable brightness
    // - `SDL_PROP_JOYSTICK_CAP_RGB_LED_BOOLEAN`: true if this joystick has an
    // LED
    //   that has adjustable color
    // - `SDL_PROP_JOYSTICK_CAP_PLAYER_LED_BOOLEAN`: true if this joystick has a
    //   player LED
    // - `SDL_PROP_JOYSTICK_CAP_RUMBLE_BOOLEAN`: true if this joystick has
    //   left/right rumble
    // - `SDL_PROP_JOYSTICK_CAP_TRIGGER_RUMBLE_BOOLEAN`: true if this joystick
    // has
    //   simple trigger rumble
    [[nodiscard]] auto properties() -> properties::id_t {
        const properties::id_t l_result = SDL_GetJoystickProperties( _data );

        assert( l_result );

        return ( l_result );
    }

    // Get the implementation dependent name of a joystick.
    [[nodiscard]] auto name() -> std::string_view {
        return { gsl::make_not_null( SDL_GetJoystickName( _data ) ) };
    }

    // Get the implementation dependent path of a joystick.
    [[nodiscard]] auto path() -> std::string_view {
        return { gsl::make_not_null( SDL_GetJoystickPath( _data ) ) };
    }

    // Get the player index of an opened joystick.
    //
    // For XInput controllers this returns the XInput user index. Many joysticks
    // will not be able to supply this information.
    [[nodiscard]] auto playerIndex() -> size_t {
        const ssize_t l_result = SDL_GetJoystickPlayerIndex( _data );

        assert( l_result != -1 );

        return ( l_result );
    }
    // Set the player index of an opened joystick.
    //
    // NULL to clear the player index and turn off player LEDs.
    void playerIndex( std::optional< size_t > _playerIndex = std::nullopt ) {
        bool l_result = false;

        if ( _playerIndex ) {
            l_result =
                SDL_SetJoystickPlayerIndex( _data, _playerIndex.value() );

        } else {
            l_result = SDL_SetJoystickPlayerIndex( _data, -1 );
        }

        assert( l_result );
    }

    // Get the implementation-dependent GUID for the joystick.
    //
    // This function requires an open joystick.
    [[nodiscard]] auto GUID() -> GUID_t {
        const GUID_t l_result = SDL_GetJoystickGUID( _data );

        assert( !l_result.empty() );

        return ( l_result );
    }

    // Get the USB vendor ID of an opened joystick, if available.
    //
    // If the vendor ID isn't available this function returns 0.
    [[nodiscard]] auto vendor() -> uint16_t {
        return ( SDL_GetJoystickVendor( _data ) );
    }

    // Get the USB product ID of an opened joystick, if available.
    //
    // If the product ID isn't available this function returns 0.
    [[nodiscard]] auto product() -> uint16_t {
        return ( SDL_GetJoystickProduct( _data ) );
    }

    // Get the product version of an opened joystick, if available.
    //
    // If the product version isn't available this function returns 0.
    [[nodiscard]] auto productVersion() -> uint16_t {
        return ( SDL_GetJoystickProductVersion( _data ) );
    }

    // Get the firmware version of an opened joystick, if available.
    //
    // If the firmware version isn't available this function returns 0.
    [[nodiscard]] auto firmwareVersion() -> uint16_t {
        return ( SDL_GetJoystickFirmwareVersion( _data ) );
    }

    // Get the serial number of an opened joystick, if available.
    //
    // Returns the serial number of the joystick, or NULL if it is not
    // available.
    [[nodiscard]] auto serial() -> std::string_view {
        const char* l_result = SDL_GetJoystickSerial( _data );

        if ( l_result ) {
            return { l_result };

        } else {
            return {};
        }
    }

    // Get the type of an opened joystick.
    [[nodiscard]] auto type() -> type_t {
        return ( fromLegacy( SDL_GetJoystickType( _data ) ) );
    }

    // Get the status of a specified joystick.
    // FIX: Maybe this does not what I think it does
    [[nodiscard]] auto isConnected() -> bool {
        return ( SDL_JoystickConnected( _data ) );
    }

    // Get the instance ID of an opened joystick.
    [[nodiscard]] auto id() -> id_t {
        const id_t l_result = SDL_GetJoystickID( _data );

        assert( l_result );

        return ( l_result );
    }

    // Get the number of general axis controls on a joystick.
    //
    // Often, the directional pad on a game controller will either look like 4
    // separate buttons or a POV hat, and not axes, but all of this is up to the
    // device and platform.
    [[nodiscard]] auto axesAmount() -> size_t {
        const ssize_t l_result = SDL_GetNumJoystickAxes( _data );

        assert( l_result == -1 );

        return ( l_result );
    }

    // Get the number of trackballs on a joystick.
    //
    // Joystick trackballs have only relative motion events associated with them
    // and their state cannot be polled.
    //
    // Most joysticks do not have trackballs.
    [[nodiscard]] auto ballsAmount() -> size_t {
        const ssize_t l_result = SDL_GetNumJoystickBalls( _data );

        assert( l_result == -1 );

        return ( l_result );
    }

    // Get the number of POV hats on a joystick.
    [[nodiscard]] auto hatsAmount() -> size_t {
        const ssize_t l_result = SDL_GetNumJoystickHats( _data );

        assert( l_result == -1 );

        return ( l_result );
    }

    // Get the number of buttons on a joystick.
    [[nodiscard]] auto buttonAmount() -> size_t {
        const ssize_t l_result = SDL_GetNumJoystickButtons( _data );

        assert( l_result == -1 );

        return ( l_result );
    }

    // Get the current state of an axis control on a joystick.
    //
    // SDL makes no promises about what part of the joystick any given axis
    // refers to. Your game should have some sort of configuration UI to let
    // users specify what each axis should be bound to. Alternately, SDL's
    // higher-level Game Controller API makes a great effort to apply order to
    // this lower-level interface, so you know that a specific axis is the "left
    // thumb stick," etc.
    //
    // The value returned by SDL_GetJoystickAxis() is a signed integer (-32768
    // to 32767) representing the current position of the axis. It may be
    // necessary to impose certain tolerances on these values to account for
    // jitter.
    //
    // The axis indices start at index 0.
    [[nodiscard]] auto axis( size_t _axis ) -> int16_t {
        const int16_t l_result = SDL_GetJoystickAxis( _data, _axis );

        assert( l_result == 0 );

        return ( l_result );
    }

    // Get the initial state of an axis control on a joystick.
    //
    // The state is a value ranging from -32768 to 32767.
    //
    // The axis indices start at index 0.
    //
    // State upon return, the initial value is supplied here.
    //
    // True if this axis has any initial value, or false if not.
    [[nodiscard]] auto axisInitialState( size_t _axis )
        -> std::optional< int16_t > {
        int16_t l_state = 0;

        const bool l_result =
            SDL_GetJoystickAxisInitialState( _data, _axis, &l_state );

        if ( l_result ) {
            return ( l_state );

        } else {
            return ( std::nullopt );
        }
    }

    // Get the ball axis change since the last poll.
    //
    // Trackballs can only return relative motion since the last call to
    // SDL_GetJoystickBall(), these motion deltas are placed into `dx` and `dy`.
    //
    // Most joysticks do not have trackballs.
    //
    // Ball indices start at index 0.
    //
    // The difference in the axis positions since the last poll.
    // X, Y.
    [[nodiscard]] auto ball( size_t _ball ) -> std::pair< ssize_t, ssize_t > {
        ssize_t l_differenceX = 0;
        ssize_t l_differenceY = 0;

        const bool l_result = SDL_GetJoystickBall(
            _data, _ball, std::bit_cast< int* >( &l_differenceX ),
            std::bit_cast< int* >( &l_differenceY ) );

        assert( l_result );

        return { l_differenceX, l_differenceY };
    }

    // Get the current state of a POV hat on a joystick.
    //
    // The returned value will be one of the `SDL_HAT_*` values.
    //
    // Hat indices start at index 0.
    [[nodiscard]] auto hat( size_t _hat ) -> uint8_t {
        return ( SDL_GetJoystickHat( _data, _hat ) );
    }

    // Get the current state of a button on a joystick.
    //
    // Button indices start at index 0.
    //
    // true if the button is pressed, false otherwise.
    [[nodiscard]] auto button( size_t _button ) -> bool {
        return ( SDL_GetJoystickButton( _data, _button ) );
    }

    // Start a rumble effect.
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
    void rumble( uint16_t _lowFrequencyRumble,
                 uint16_t _highFrequencyRumble,
                 std::chrono::milliseconds _duration ) {
        const bool l_result =
            SDL_RumbleJoystick( _data, _lowFrequencyRumble,
                                _highFrequencyRumble, _duration.count() );

        assert( l_result );
    }

    // Start a rumble effect in the joystick's triggers.
    //
    // Each call to this function cancels any previous trigger rumble effect,
    // and calling it with 0 intensity stops any rumbling.
    //
    // Note that this is rumbling of the _triggers_ and not the game controller
    // as a whole. This is currently only supported on Xbox One controllers. If
    // you want the (more common) whole-controller rumble, use
    // SDL_RumbleJoystick() instead.
    //
    // This function requires you to process SDL events or call
    // SDL_UpdateJoysticks() to update rumble state.
    //
    // The intensity of the low frequency (left) rumble motor, from 0 to 0xFFFF.
    //
    // The intensity of the high frequency (right) rumble motor, from 0 to
    // 0xFFFF.
    void rumbleTriggers( uint16_t _leftRumble,
                         uint16_t _rightRumble,
                         std::chrono::milliseconds _duration ) {
        const bool l_result = SDL_RumbleJoystickTriggers(
            _data, _leftRumble, _rightRumble, _duration.count() );

        assert( l_result );
    }

    // Update a joystick's LED color.
    //
    // An example of a joystick LED is the light on the back of a PlayStation
    // 4's DualShock 4 controller.
    //
    // For joysticks with a single color LED, the maximum of the RGB values will
    // be used as the LED brightness.
    //
    // The intensity of the LED.
    //
    // Alpha is ignored
    void LED( color_t _color ) {
        const bool l_result =
            SDL_SetJoystickLED( _data, _color.red, _color.green, _color.blue );

        assert( l_result );
    }

    // Send a joystick specific effect packet.
    void effect( std::span< const std::byte > _effect ) {
        const bool l_result =
            SDL_SendJoystickEffect( _data, _effect.data(), _effect.size() );

        assert( l_result );
    }

    // Close a joystick previously opened with SDL_OpenJoystick().
    void close() { SDL_CloseJoystick( _data ); }

    // Get the connection state of a joystick.
    [[nodiscard]] auto connectionState() -> connectionState_t {
        const connectionState_t l_result =
            fromLegacy( SDL_GetJoystickConnectionState( _data ) );

        assert( l_result != connectionState_t::invalid );

        return ( l_result );
    }

    // Get the battery state of a joystick.
    //
    // You should never take a battery status as absolute truth. Batteries
    // (especially failing batteries) are delicate hardware, and the values
    // reported here are best estimates based on what that hardware reports.
    // It's not uncommon for older batteries to lose stored power much faster
    // than it reports, or completely drain when reporting it has 20 percent
    // left, etc.
    //
    // The percentage of battery life left, between 0 and 100, or NULL to
    // ignore. This will be filled in with -1 if there is no battery.
    [[nodiscard]] auto powerInfo() -> std::pair< power::state_t, ssize_t > {
        ssize_t l_percent = 0;

        const power::state_t l_result =
            power::fromLegacy( SDL_GetJoystickPowerInfo(
                _data, std::bit_cast< int* >( &l_percent ) ) );

        assert( l_result != power::state_t::error );

        return { l_result, l_percent };
    }

    // Variables
private:
    gsl::not_null< native_t > _data;
};

// Locking for atomic access to the joystick API.
//
// The SDL joystick functions are thread-safe, however you can lock the
// joysticks while processing to guarantee that the joystick list won't change
// and joystick and gamepad events will not be delivered.
inline void lock() {
    SDL_LockJoysticks();
}

// Unlocking for atomic access to the joystick API.
inline void unlock() {
    SDL_UnlockJoysticks();
}

// Return whether a joystick is currently connected.
[[nodiscard]] inline auto hasAny() -> bool {
    return ( SDL_HasJoystick() );
}

// Get the device information encoded in a SDL_GUID structure.
//
// The device VID, or 0 if not available.
//
// The device PID, or 0 if not available.
//
// The device version, or 0 if not available.
//
// A CRC used to distinguish different products with the same VID/PID, or 0 if
// not available.
[[nodiscard]] inline auto info( GUID_t _GUID )
    -> std::tuple< uint16_t, uint16_t, uint16_t, uint16_t > {
    uint16_t l_vendor = 0;
    uint16_t l_product = 0;
    uint16_t l_version = 0;
    uint16_t l_crc16 = 0;

    SDL_GetJoystickGUIDInfo( _GUID, &l_vendor, &l_product, &l_version,
                             &l_crc16 );

    return { l_vendor, l_product, l_version, l_crc16 };
}

// Set the state of joystick event processing.
//
// If joystick events are disabled, you must call SDL_UpdateJoysticks()
// yourself and check the state of the joystick when you want joystick
// information.
inline void eventsToggle( bool _isEnabled ) {
    SDL_SetJoystickEventsEnabled( _isEnabled );
}

// Query the state of joystick event processing.
//
// If joystick events are disabled, you must call SDL_UpdateJoysticks()
// yourself and check the state of the joystick when you want joystick
// information.
//
// \returns true if joystick events are being processed, false otherwise.
[[nodiscard]] inline auto areEventsEnabled() -> bool {
    return ( SDL_JoystickEventsEnabled() );
}

// Update the current state of the open joysticks.
//
// This is called automatically by the event loop if any joystick events are
// enabled.
inline void update() {
    SDL_UpdateJoysticks();
}

} // namespace slickdl::joystick
