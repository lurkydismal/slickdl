#pragma once

#include <SDL3/SDL_sensor.h>

#include <gsl/pointers>
#include <type_traits>

#include "slickdl.hpp"
#include "slickdl/properties.hpp"

// SDL sensor management.
//
// These APIs grant access to gyros and accelerometers on various platforms.
//
// In order to use these functions, SDL_Init() must have been called with the
// SDL_INIT_SENSOR flag. This causes SDL to scan the system for sensors, and
// load appropriate drivers.
namespace slickdl::sensors {

// This is a unique ID for a sensor for the time it is connected to the
// system, and is never reused for the lifetime of the application.
//
// The value 0 is an invalid ID.
using id_t = uint32_t;

// A constant to represent standard gravity for accelerometer sensors.
//
// The accelerometer returns the current acceleration in SI meters per
// second squared. This measurement includes the force of gravity, so a
// device at rest will have an value of SDL_STANDARD_GRAVITY away from the
// center of the earth, which is a positive Y value.
constexpr float g_standardGravity = 9.80665f;

// The different sensors defined by SDL.
//
// Additional sensors may be available, using platform dependent semantics.
//
// Here are the additional Android sensors:
//
// https://developer.android.com/reference/android/hardware/SensorEvent.html#values
//
// Accelerometer sensor notes:
//
// The accelerometer returns the current acceleration in SI meters per
// second squared. This measurement includes the force of gravity, so a
// device at rest will have an value of SDL_STANDARD_GRAVITY away from the
// center of the earth, which is a positive Y value.
//
// - `values[0]`: Acceleration on the x axis
// - `values[1]`: Acceleration on the y axis
// - `values[2]`: Acceleration on the z axis
//
// For phones and tablets held in natural orientation and game controllers
// held in front of you, the axes are defined as follows:
//
// - -X ... +X : left ... right
// - -Y ... +Y : bottom ... top
// - -Z ... +Z : farther ... closer
//
// The accelerometer axis data is not changed when the device is rotated.
//
// Gyroscope sensor notes:
//
// The gyroscope returns the current rate of rotation in radians per second.
// The rotation is positive in the counter-clockwise direction. That is, an
// observer looking from a positive location on one of the axes would see
// positive rotation on that axis when it appeared to be rotating
// counter-clockwise.
//
// - `values[0]`: Angular speed around the x axis (pitch)
// - `values[1]`: Angular speed around the y axis (yaw)
// - `values[2]`: Angular speed around the z axis (roll)
//
// For phones and tablets held in natural orientation and game controllers
// held in front of you, the axes are defined as follows:
//
// - -X ... +X : left ... right
// - -Y ... +Y : bottom ... top
// - -Z ... +Z : farther ... closer
//
// The gyroscope axis data is not changed when the device is rotated.
using type_t = enum class type : int8_t {
    invalid = -1,       /**< Returned for an invalid sensor */
    unknown,            /**< Unknown sensor type */
    accelerometer,      /**< Accelerometer */
    gyro,               /**< Gyroscope */
    accelerometerLeft,  /**< Accelerometer for left Joy-Con controller and
                           Wii  nunchuk */
    gyroLeft,           /**< Gyroscope for left Joy-Con controller */
    accelerometerRight, /**< Accelerometer for right Joy-Con controller */
    gyroRight,          /**< Gyroscope for right Joy-Con controller */
};

using typeUnderlying_t = std::underlying_type_t< type_t >;

[[nodiscard]] constexpr auto toLegacy( type_t _type ) -> SDL_SensorType {
    return ( static_cast< SDL_SensorType >( _type ) );
}

[[nodiscard]] constexpr auto toLegacy( type_t* _type ) -> SDL_SensorType* {
    return ( std::bit_cast< SDL_SensorType* >( _type ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_SensorType _type ) -> type_t {
    return ( static_cast< type_t >( _type ) );
}

// The opaque structure used to identify an opened SDL sensor.
using sensor_t = gsl::not_null< SDL_Sensor* >;

// Get a list of currently connected sensors.
[[nodiscard]] auto all() -> std::vector< id_t >;

// Get the instance ID of a sensor.
[[nodiscard]] inline auto id( sensor_t _sensor ) -> id_t {
    const id_t l_result = SDL_GetSensorID( _sensor );

    assert( l_result );

    return ( l_result );
}

// Open a sensor for use.
[[nodiscard]] inline auto open( id_t _id ) -> sensor_t {
    return { SDL_OpenSensor( _id ) };
}

// Return the SDL_Sensor associated with an instance ID.
[[nodiscard]] inline auto fromId( id_t _id ) -> sensor_t {
    return { SDL_GetSensorFromID( _id ) };
}

// Get the properties associated with a sensor.
[[nodiscard]] inline auto properties( sensor_t _sensor ) -> properties::id_t {
    const properties::id_t l_result = SDL_GetSensorProperties( _sensor );

    assert( l_result );

    return ( l_result );
}

// Get the implementation dependent name of a sensor.
//
// This can be called before any sensors are opened.
//
// \param instance_id the sensor instance ID.
// \returns the sensor name, or NULL if `instance_id` is not valid.
[[nodiscard]] inline auto name( id_t _id ) -> std::string_view {
    return { gsl::make_not_null( SDL_GetSensorNameForID( _id ) ) };
}

// Get the implementation dependent name of a sensor.
[[nodiscard]] inline auto name( sensor_t _sensor ) -> std::string_view {
    return { gsl::make_not_null( SDL_GetSensorName( _sensor ) ) };
}

// Get the type of a sensor.
//
// This can be called before any sensors are opened.
[[nodiscard]] inline auto type( id_t _id ) -> type_t {
    const type_t l_result = fromLegacy( SDL_GetSensorTypeForID( _id ) );

    assert( l_result != type_t::invalid );

    return ( l_result );
}

// Get the type of a sensor.
[[nodiscard]] inline auto type( sensor_t _sensor ) -> type_t {
    const type_t l_result = fromLegacy( SDL_GetSensorType( _sensor ) );

    assert( l_result != type_t::invalid );

    return ( l_result );
}

// Get the platform dependent type of a sensor.
//
// This can be called before any sensors are opened.
[[nodiscard]] inline auto nonPortableType( id_t _id ) -> int {
    const int l_result = SDL_GetSensorNonPortableTypeForID( _id );

    assert( l_result != -1 );

    return ( l_result );
}

// Get the platform dependent type of a sensor.
[[nodiscard]] inline auto nonPortableType( sensor_t _sensor ) -> int {
    const int l_result = SDL_GetSensorNonPortableType( _sensor );

    assert( l_result != -1 );

    return ( l_result );
}

// Get the current state of an opened sensor.
//
// The number of values and interpretation of the data is sensor dependent.
//
// Amount of values to write to data.
template < size_t N >
    requires( N > 0 )
[[nodiscard]] inline auto data( sensor_t _sensor ) -> std::array< float, N > {
    std::array< float, N > l_data{};

    const bool l_result = SDL_GetSensorData( _sensor, l_data.data(), N );

    assert( l_result );

    return ( l_data );
}

// Close a sensor previously opened with SDL_OpenSensor().
inline void close( sensor_t _sensor ) {
    SDL_CloseSensor( _sensor );
}

// Update the current state of the open sensors.
//
// This is called automatically by the event loop if sensor events are
// enabled.
//
// Needs to be called from the thread that initialized the sensor subsystem.
inline void update() {
    SDL_UpdateSensors();
}

} // namespace slickdl::sensors
