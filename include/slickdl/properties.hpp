#pragma once

#include <SDL3/SDL_properties.h>

#include <bit>
#include <cstdint>
#include <string_view>
#include <type_traits>

#include "slickdl.hpp"

// A property is a variable that can be created and retrieved by name at
// runtime.
//
// All properties are part of a property group (id_t). A property
// group can be created with the SDL_CreateProperties function and destroyed
// with the SDL_DestroyProperties function.
//
// Properties can be added to and retrieved from a property group through the
// following functions:
//
// - SDL_SetPointerProperty and SDL_GetPointerProperty operate on `void*`
//   pointer types.
// - SDL_SetStringProperty and SDL_GetStringProperty operate on string types.
// - SDL_SetNumberProperty and SDL_GetNumberProperty operate on signed 64-bit
//   integer types.
// - SDL_SetFloatProperty and SDL_GetFloatProperty operate on floating point
//   types.
// - SDL_SetBooleanProperty and SDL_GetBooleanProperty operate on boolean
//   types.
//
// Properties can be removed from a group by using SDL_ClearProperty.
namespace slickdl::properties {

using id_t = uint32_t;

using type_t = enum class type : uint8_t {
    invalid,
    pointer,
    string,
    number,
    floatingPoint,
    boolean,
};

using typeUnderlying_t = std::underlying_type_t< type_t >;

[[nodiscard]] constexpr auto toLegacy( type_t _value ) -> SDL_PropertyType {
    return ( static_cast< SDL_PropertyType >( _value ) );
}

[[nodiscard]] constexpr auto toLegacy( type_t* _value ) -> SDL_PropertyType* {
    return ( std::bit_cast< SDL_PropertyType* >( _value ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_PropertyType _value ) -> type_t {
    return ( static_cast< type_t >( _value ) );
}

// Get the global SDL properties.
[[nodiscard]] inline auto global() -> id_t {
    const id_t l_result = SDL_GetGlobalProperties();

    assert( l_result );

    return ( l_result );
}

// Create a group of properties.
//
// All properties are automatically destroyed when SDL_Quit() is called.
[[nodiscard]] inline auto create() -> id_t {
    const id_t l_result = SDL_CreateProperties();

    assert( l_result );

    return ( l_result );
}

// Copy a group of properties.
//
// Copy all the properties from one group of properties to another, with the
// exception of properties requiring cleanup (set using
// SDL_SetPointerPropertyWithCleanup()), which will not be copied. Any
// property that already exists on `dst` will be overwritten.
inline void copy( id_t _source, id_t _destination ) {
    const bool l_result = SDL_CopyProperties( _source, _destination );

    assert( l_result );
}

// Lock a group of properties.
//
// Obtain a multi-threaded lock for these properties. Other threads will wait
// while trying to lock these properties until they are unlocked. Properties
// must be unlocked before they are destroyed.
//
// The lock is automatically taken when setting individual properties, this
// function is only needed when you want to set several properties atomically
// or want to guarantee that properties being queried aren't freed in another
// thread.
inline void lock( id_t _properties ) {
    const bool l_result = SDL_LockProperties( _properties );

    assert( l_result );
}

// Unlock a group of properties.
inline void unlock( id_t _properties ) {
    SDL_UnlockProperties( _properties );
}

// A callback used to free resources when a property is deleted.
//
// This should release any resources associated with `value` that are no
// longer needed.
//
// This callback is set per-property. Different properties in the same group
// can have different cleanup callbacks.
//
// This callback will be called _during_ SDL_SetPointerPropertyWithCleanup if
// the function fails for any reason.
//
// Pointer assigned to the property to clean up.
//
// This callback may fire without any locks held; if this is a concern, the app
// should provide its own locking.
using cleanupPropertyCallback_t = gsl::not_null< SDL_CleanupPropertyCallback >;

// Set a pointer property in a group of properties with a cleanup function
// that is called when the property is deleted.
//
// The cleanup function is also called if setting the property fails for any
// reason.
//
// For simply setting basic data types, like numbers, bools, or strings, use
// SDL_SetNumberProperty, SDL_SetBooleanProperty, or SDL_SetStringProperty
// instead, as those functions will handle cleanup on your behalf. This
// function is only for more complex, custom data.
//
// New value of the property, or NULL to delete the property.
//
// Cleanup the function to call when this property is deleted, or NULL if no
// cleanup is necessary.
inline void pointerPropertyWithCleanup(
    id_t _properties,
    std::string_view _name,
    std::optional< void* > _value = std::nullopt,
    std::optional< cleanupPropertyCallback_t > _cleanup = std::nullopt,
    void* _userData = nullptr ) {
    bool l_result = false;

    if ( _cleanup ) {
        l_result = SDL_SetPointerPropertyWithCleanup(
            _properties, std::string( _name ).c_str(),
            _value.value_or( nullptr ), _cleanup.value(), _userData );

    } else {
        l_result = SDL_SetPointerPropertyWithCleanup(
            _properties, std::string( _name ).c_str(),
            _value.value_or( nullptr ), nullptr, _userData );
    }

    assert( l_result );
}

// Set a string property in a group of properties.
//
// This function makes a copy of the value string; the caller does not have to
// preserve the data after this call completes.
//
// New value of the property, or NULL to delete the property.
// NULL does not apply to integrals, floating point and boolean.
template < typename T >
inline void set( id_t _properties,
                 std::string_view _name,
                 std::optional< T > _value = std::nullopt ) {
    const auto l_name = std::string( _name );
    std::function< bool( id_t, const char*, T ) > l_setterFunction;
    bool l_result = false;

    // TODO: Improve
    if constexpr ( std::is_same_v< T, const char* > ) {
        l_setterFunction = SDL_SetStringProperty;

    } else if constexpr ( std::is_pointer_v< T > ) {
        l_setterFunction = SDL_SetPointerProperty;

    } else if constexpr ( std::is_same_v< T, bool > ) {
        l_setterFunction = SDL_SetBooleanProperty;

    } else if constexpr ( std::is_integral_v< T > ) {
        l_setterFunction = SDL_SetNumberProperty;

    } else if constexpr ( std::is_same_v< T, float > ) {
        l_setterFunction = SDL_SetFloatProperty;

    } else {
        // TODO: Message
        static_assert( false );
    }

    if ( _value ) {
        l_result =
            l_setterFunction( _properties, l_name.c_str(), _value.value() );

    } else {
        l_result = l_setterFunction( _properties, l_name.c_str(), T{} );
    }

    assert( l_result );
}

// Return whether a property exists in a group of properties.
[[nodiscard]] inline auto exists( id_t _properties, std::string_view _name )
    -> bool {
    return ( SDL_HasProperty( _properties, std::string( _name ).c_str() ) );
}

// Get the type of a property in a group of properties.
[[nodiscard]] inline auto type( id_t _properties, std::string_view _name )
    -> type_t {
    const type_t l_result = fromLegacy(
        SDL_GetPropertyType( _properties, std::string( _name ).c_str() ) );

    assert( l_result != type_t::invalid );

    return ( l_result );
}

// Get a pointer property from a group of properties.
//
// By convention, the names of properties that SDL exposes on objects will
// start with "SDL.", and properties that SDL uses internally will start with
// "SDL.internal.". These should be considered read-only and should not be
// modified by applications.
//
// The data returned is not protected and could potentially be freed if you call
// SDL_SetPointerProperty() or SDL_ClearProperty() on these properties from
// another thread. If you need to avoid this, use SDL_LockProperties() and
// SDL_UnlockProperties().
template < typename T >
[[nodiscard]] inline auto get( id_t _properties,
                               std::string_view _name,
                               std::optional< T > _defaultValue = std::nullopt )
    -> std::optional< T > {
    const auto l_name = std::string( _name );
    std::function< T( id_t, const char*, T ) > l_getterFunction;

    // TODO: Improve
    if constexpr ( std::is_same_v< T, const char* > ) {
        l_getterFunction = SDL_GetStringProperty;

    } else if constexpr ( std::is_pointer_v< T > ) {
        l_getterFunction = SDL_GetPointerProperty;

    } else if constexpr ( std::is_same_v< T, bool > ) {
        l_getterFunction = SDL_GetBooleanProperty;

    } else if constexpr ( std::is_integral_v< T > ) {
        l_getterFunction = SDL_GetNumberProperty;

    } else if constexpr ( std::is_same_v< T, float > ) {
        l_getterFunction = SDL_GetFloatProperty;

    } else {
        // TODO: Message
        static_assert( false );
    }

    if ( _defaultValue ) {
        return ( l_getterFunction( _properties, l_name.c_str(),
                                   _defaultValue.value() ) );

    } else {
        return ( l_getterFunction( _properties, l_name.c_str(), T{} ) );
    }
}

// Clear a property from a group of properties.
inline void clear( id_t _properties, std::string_view _name ) {
    const bool l_result =
        SDL_ClearProperty( _properties, std::string( _name ).c_str() );

    assert( l_result );
}

// A callback used to enumerate all the properties in a group of
// properties.
//
// This callback is called from SDL_EnumerateProperties(), and is called
// once per property in the set.
//
// The id_t that is being enumerated.
//
// The next property name in the enumeration.
//
// enumerateCallback_t holds a lock on `_properties` during this
// callback.
using enumerateCallback_t = gsl::not_null< SDL_EnumeratePropertiesCallback >;

// Enumerate the properties contained in a group of properties.
//
// The callback function is called for each property in the group of
// properties. The properties are locked during enumeration.
inline void enumerate( id_t _properties,
                       enumerateCallback_t _callback,
                       void* _userData = nullptr ) {
    const bool l_result =
        SDL_EnumerateProperties( _properties, _callback, _userData );

    assert( l_result );
}

// Destroy a group of properties.
//
// All properties are deleted and their cleanup functions will be
// called, if any.
//
// Should not be called while these properties are locked or other threads might
// be setting or getting values from these properties.
inline void destroy( id_t _properties ) {
    SDL_DestroyProperties( _properties );
}

} // namespace slickdl::properties
