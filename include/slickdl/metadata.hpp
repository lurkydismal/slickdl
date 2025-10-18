#pragma once

#include <SDL3/SDL_init.h>

#include <gsl/pointers>
#include <optional>
#include <string>
#include <string_view>

namespace slickdl::metadata {

namespace property {

constexpr std::string_view g_name = "SDL.app.metadata.name";
constexpr std::string_view g_version = "SDL.app.metadata.version";
constexpr std::string_view g_identifier = "SDL.app.metadata.identifier";
constexpr std::string_view g_creator = "SDL.app.metadata.creator";
constexpr std::string_view g_copyright = "SDL.app.metadata.copyright";
constexpr std::string_view g_url = "SDL.app.metadata.url";
constexpr std::string_view g_type = "SDL.app.metadata.type";

// There are several locations where SDL can make use of metadata (an "About"
// box in the macOS menu bar, the name of the app can be shown on some audio
// mixers, etc). Any piece of metadata can be left out, if a specific detail
// doesn't make sense for the app
//
// This function should be called as early as possible, before SDL_Init
// Multiple calls to this function are allowed, but various state might not
// change once it has been set up with a previous call to this function
//
// Once set, this metadata can be read using SDL_GetAppMetadataProperty()
//
// These are the supported properties:
//
// - `SDL_PROP_APP_METADATA_NAME_STRING`: The human-readable name of the
//   application, like "My Game 2: Bad Guy's Revenge!". This will show up
//   anywhere the OS shows the name of the application separately from window
//   titles, such as volume control applets, etc. This defaults to "SDL
//   Application"
// - `SDL_PROP_APP_METADATA_VERSION_STRING`: The version of the app that is
//   running; there are no rules on format, so "1.0.3beta2" and "April 22nd,
//   2024" and a git hash are all valid options. This has no default
// - `SDL_PROP_APP_METADATA_IDENTIFIER_STRING`: A unique string that
//   identifies this app. This must be in reverse-domain format, like
//   "com.example.mygame2". This string is used by desktop compositors to
//   identify and group windows together, as well as match applications with
//   associated desktop settings and icons. If you plan to package your
//   application in a container such as Flatpak, the app ID should match the
//   name of your Flatpak container as well. This has no default
// - `SDL_PROP_APP_METADATA_CREATOR_STRING`: The human-readable name of the
//   creator/developer/maker of this app, like "MojoWorkshop, LLC"
// - `SDL_PROP_APP_METADATA_COPYRIGHT_STRING`: The human-readable copyright
//   notice, like "Copyright (c) 2024 MojoWorkshop, LLC" or whatnot. Keep this
//   to one line, don't paste a copy of a whole software license in here. This
//   has no default
// - `SDL_PROP_APP_METADATA_URL_STRING`: A URL to the app on the web. Maybe a
//   product page, or a storefront, or even a GitHub repository, for user's
//   further information This has no default
// - `SDL_PROP_APP_METADATA_TYPE_STRING`: The type of application this is
//   Currently this string can be "game" for a video game, "mediaplayer" for a
//   media player, or generically "application" if nothing else applies
//   Future versions of SDL might add new types. This defaults to
//   "application"
//
// _value can be nullopt to remove property
// TODO: Improve
void set( std::string_view _name,
          std::optional< std::string_view > _value = std::nullopt );

// Returns the current value of the metadata property, asserts for
// properties with no default
//
// The string returned is not protected and could potentially be freed if you
// call SDL_SetAppMetadataProperty() to set that property from another thread
[[nodiscard]] inline auto get( std::string_view _name ) -> std::string_view {
    return ( std::string_view{ gsl::make_not_null(
        SDL_GetAppMetadataProperty( std::string( _name ).c_str() ) ) } );
}

} // namespace property

// You can optionally provide metadata about your app to SDL. This is not
// required, but strongly encouraged
//
// There are several locations where SDL can make use of metadata (an "About"
// box in the macOS menu bar, the name of the app can be shown on some audio
// mixers, etc). Any piece of metadata can be left as NULL, if a specific
// detail doesn't make sense for the app
//
// This function should be called as early as possible, before SDL_Init
// Multiple calls to this function are allowed, but various state might not
// change once it has been set up with a previous call to this function
//
// Passing a NULL removes any previous metadata
//
// This is a simplified interface for the most important information. You can
// supply significantly more detailed metadata with SDL_SetAppMetadataProperty()
inline void set(
    std::optional< std::string_view > _applicationName = std::nullopt,
    std::optional< std::string_view > _applicationVersion = std::nullopt,
    std::optional< std::string_view > _applicationIdentifier = std::nullopt ) {
    property::set( property::g_name, _applicationName );
    property::set( property::g_version, _applicationVersion );
    property::set( property::g_identifier, _applicationIdentifier );
}

} // namespace slickdl::metadata
