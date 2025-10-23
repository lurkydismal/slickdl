#include <SDL3/SDL_clipboard.h>

#include <string_view>

#include "slickdl.hpp"

// SDL provides access to the system clipboard, both for reading information
// from other processes and publishing information of its own.
//
// This is not just text! SDL apps can access and publish data by mimetype.
//
// ## Basic use (text)
//
// Obtaining and publishing simple text to the system clipboard is as easy as
// calling SDL_GetClipboardText() and SDL_SetClipboardText(), respectively.
// These deal with C strings in UTF-8 encoding. Data transmission and encoding
// conversion is completely managed by SDL.
//
// ## Clipboard callbacks (data other than text)
//
// Things get more complicated when the clipboard contains something other
// than text. Not only can the system clipboard contain data of any type, in
// some cases it can contain the same data in different formats! For example,
// an image painting app might let the user copy a graphic to the clipboard,
// and offers it in .BMP, .JPG, or .PNG format for other apps to consume.
//
// Obtaining clipboard data ("pasting") like this is a matter of calling
// SDL_GetClipboardData() and telling it the mimetype of the data you want.
// But how does one know if that format is available? SDL_HasClipboardData()
// can report if a specific mimetype is offered, and
// SDL_GetClipboardMimeTypes() can provide the entire list of mimetypes
// available, so the app can decide what to do with the data and what formats
// it can support.
//
// Setting the clipboard ("copying") to arbitrary data is done with
// SDL_SetClipboardData. The app does not provide the data in this call, but
// rather the mimetypes it is willing to provide and a callback function.
// During the callback, the app will generate the data. This allows massive
// data sets to be provided to the clipboard, without any data being copied
// before it is explicitly requested. More specifically, it allows an app to
// offer data in multiple formats without providing a copy of all of them
// upfront. If the app has an image that it could provide in PNG or JPG
// format, it doesn't have to encode it to either of those unless and until
// something tries to paste it.
//
// ## Primary Selection
//
// The X11 and Wayland video targets have a concept of the "primary selection"
// in addition to the usual clipboard. This is generally highlighted (but not
// explicitly copied) text from various apps. SDL offers APIs for this through
// SDL_GetPrimarySelectionText() and SDL_SetPrimarySelectionText(). SDL offers
// these APIs on platforms without this concept, too, but only so far that it
// will keep a copy of a string that the app sets for later retrieval; the
// operating system will not ever attempt to change the string externally if
// it doesn't support a primary selection.
namespace slickdl::clipboard {

// Put UTF-8 text into the clipboard.
//
// Should only be called on the main thread.
inline void text( std::string_view _text ) {
    const bool l_result = SDL_SetClipboardText( std::string( _text ).c_str() );

    assert( l_result );
}

// Get UTF-8 text from the clipboard.
//
// This functions returns an empty string if there was not enough memory left
// for a copy of the clipboard's content.
//
// Should only be called on the main thread.
[[nodiscard]] auto text() -> std::string;

// Query whether the clipboard exists and contains a non-empty text string.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto hasText() -> bool {
    return ( SDL_HasClipboardText() );
}

namespace primary_selection {

// Put UTF-8 text into the primary selection.
//
// Should only be called on the main thread.
inline void text( std::string_view _text ) {
    const bool l_result =
        SDL_SetPrimarySelectionText( std::string( _text ).c_str() );

    assert( l_result );
}

// Get UTF-8 text from the primary selection.
//
// This functions returns an empty string if there was not enough memory
// left for a copy of the primary selection's content.
//
// Should only be called on the main thread.
[[nodiscard]] auto text() -> std::string;

// Query whether the primary selection exists and contains a non-empty text
// string.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto hasText() -> bool {
    return SDL_HasPrimarySelectionText();
}

} // namespace primary_selection

// Callback function that will be called when data for the specified
// mime-type is requested by the OS.
//
// The callback function is called with NULL as the mime_type when the
// clipboard is cleared or new data is set. The clipboard is automatically
// cleared in SDL_Quit().
//
// The data for the provided mime-type. Returning NULL or setting length to 0
// will cause no data to be sent to the "receiver". It is up to the receiver to
// handle this. Essentially returning no data is more or less undefined behavior
// and may cause breakage in receiving applications. The returned data will not
// be freed so it needs to be retained and dealt with internally.
using dataCallback_t = SDL_ClipboardDataCallback;

// Callback function that will be called when the clipboard is cleared, or
// new data is set.
using cleanupCallback_t = SDL_ClipboardCleanupCallback;

// Offer clipboard data to the OS.
//
// Tell the operating system that the application is offering clipboard data
// for each of the provided mime-types. Once another application requests
// the data the callback function will be called, allowing it to generate
// and respond with the data for the requested mime-type.
//
// The size of text data does not include any terminator, and the text does
// not need to be null terminated (e.g. you can directly copy a portion of a
// document).
//
// Should only be called on the main thread.
inline void data( dataCallback_t _callback,
                  cleanupCallback_t _cleanup,
                  void* _userData,
                  std::span< const char* > _mimeTypes ) {
    const bool l_result = SDL_SetClipboardData(
        _callback, _cleanup, _userData, _mimeTypes.data(), _mimeTypes.size() );

    assert( l_result );
}

// Clear the clipboard data.
//
// Should only be called on the main thread.
inline void clear() {
    const bool l_result = SDL_ClearClipboardData();

    assert( l_result );
}

// Get the data from clipboard for a given mime type.
//
// The size of text data does not include the terminator, but the text is
// guaranteed to be null terminated.
//
// \returns the retrieved data buffer or NULL on failure; call
// SDL_GetError() for more information. This should be freed with SDL_free()
// when it is no longer needed.
//
// Should only be called on the main thread.
template < typename T >
[[nodiscard]] inline auto data( std::string_view _mimeType ) -> T* {
    size_t l_amount = 0;

    std::unique_ptr< void, void ( * )( void* ) > l_result(
        SDL_GetClipboardData( std::string( _mimeType ).c_str(), &l_amount ),
        SDL_free );

    assert( l_result.get() );
    assert( sizeof( T ) == l_amount );

    return ( std::bit_cast< T* >( l_result.get() ) );
}

// Query whether there is data in the clipboard for the provided mime type.
//
// Should only be called on the main thread.
[[nodiscard]] inline auto hasData( std::string_view _mimeType ) -> bool {
    return ( SDL_HasClipboardData( std::string( _mimeType ).c_str() ) );
}

// Retrieve the list of mime types available in the clipboard.
//
// Should only be called on the main thread.
[[nodiscard]] auto mimeTypes() -> std::vector< std::string >;

} // namespace slickdl::clipboard
