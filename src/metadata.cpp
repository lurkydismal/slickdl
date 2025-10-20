#include "slickdl/metadata.hpp"

namespace slickdl::metadata::property {

// TODO: Improve
void set( std::string_view _name, std::optional< std::string_view > _value ) {
    bool l_result = false;

    const std::flat_map< std::string_view, std::string_view > l_map{
        { g_name, "Application name" },
        { g_version, "Application version" },
        { g_identifier, "Application identifier" },
        { g_creator, "Application creator" },
        { g_copyright, "Application copyright" },
        { g_url, "Application URL" },
        { g_type, "Application type" },
    };

    if ( _value ) {
        logg::info( "{} '{}'", l_map.at( _name ), _value.value() );

        l_result =
            SDL_SetAppMetadataProperty( std::string( _name ).c_str(),
                                        std::string( _value.value() ).c_str() );

    } else {
        logg::info( "Removing {}", l_map.at( _name ) );

        l_result =
            SDL_SetAppMetadataProperty( std::string( _name ).c_str(), nullptr );
    }

    assert( l_result );
}

} // namespace slickdl::metadata::property
