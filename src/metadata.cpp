#include "slickdl/metadata.hpp"

#include "slickdl.hpp"

namespace slickdl::metadata::property {

void set( std::string_view _name, std::optional< std::string_view > _value ) {
    bool l_result = false;

    if ( _value ) {
        l_result =
            SDL_SetAppMetadataProperty( std::string( _name ).c_str(),
                                        std::string( _value.value() ).c_str() );

    } else {
        l_result =
            SDL_SetAppMetadataProperty( std::string( _name ).c_str(), nullptr );
    }

    assert( l_result );
}

} // namespace slickdl::metadata::property
