#include "slickdl/pixels_palette.hpp"

#include "slickdl.hpp"
#include "stdflat_map.hpp"

namespace slickdl {

void palette_t::colors( std::span< const color_t > _colors,
                        size_t _firstEntryIndex ) {
    const std::vector< SDL_Color > l_colors =
        stdfunc::spanToVector< color_t, SDL_Color >( _colors );

    const bool l_result = SDL_SetPaletteColors(
        _data, l_colors.data(), _firstEntryIndex, l_colors.size() );

    assert( l_result );
}

namespace pixels {

[[nodiscard]] auto name( format_t _format ) -> std::string_view {
    std::string_view l_returnValue;

    static const stdfunc::flatMap_t< format_t, std::string_view > l_lookUpTable{
        { format_t::index1LSB, "index1LSB" },
        { format_t::index1MSB, "index1MSB" },
        { format_t::index2LSB, "index2LSB" },
        { format_t::index2MSB, "index2MSB" },
        { format_t::index4LSB, "index4LSB" },
        { format_t::index4MSB, "index4MSB" },
        { format_t::index8, "index8" },
        { format_t::fRGB332, "fRGB332" },
        { format_t::fXRGB4444, "fXRGB4444" },
        { format_t::fXBGR4444, "fXBGR4444" },
        { format_t::fXRGB1555, "fXRGB1555" },
        { format_t::fXBGR1555, "fXBGR1555" },
        { format_t::fARGB4444, "fARGB4444" },
        { format_t::fRGBA4444, "fRGBA4444" },
        { format_t::fABGR4444, "fABGR4444" },
        { format_t::fBGRA4444, "fBGRA4444" },
        { format_t::fARGB1555, "fARGB1555" },
        { format_t::fRGBA5551, "fRGBA5551" },
        { format_t::fABGR1555, "fABGR1555" },
        { format_t::fBGRA5551, "fBGRA5551" },
        { format_t::fRGB565, "fRGB565" },
        { format_t::fBGR565, "fBGR565" },
        { format_t::fRGB24, "fRGB24" },
        { format_t::fBGR24, "fBGR24" },
        { format_t::fXRGB8888, "fXRGB8888" },
        { format_t::fRGBX8888, "fRGBX8888" },
        { format_t::fXBGR8888, "fXBGR8888" },
        { format_t::fBGRX8888, "fBGRX8888" },
        { format_t::fARGB8888, "fARGB8888" },
        { format_t::fRGBA8888, "fRGBA8888" },
        { format_t::fABGR8888, "fABGR8888" },
        { format_t::fBGRA8888, "fBGRA8888" },
        { format_t::fXRGB2101010, "fXRGB2101010" },
        { format_t::fXBGR2101010, "fXBGR2101010" },
        { format_t::fARGB2101010, "fARGB2101010" },
        { format_t::fABGR2101010, "fABGR2101010" },
        { format_t::fRGB48, "fRGB48" },
        { format_t::fBGR48, "fBGR48" },
        { format_t::fRGBA64, "fRGBA64" },
        { format_t::fARGB64, "fARGB64" },
        { format_t::fBGRA64, "fBGRA64" },
        { format_t::fABGR64, "fABGR64" },
        { format_t::fRGB48Float, "fRGB48Float" },
        { format_t::fBGR48Float, "fBGR48Float" },
        { format_t::fRGBA64Float, "fRGBA64Float" },
        { format_t::fARGB64Float, "fARGB64Float" },
        { format_t::fBGRA64Float, "fBGRA64Float" },
        { format_t::fABGR64Float, "fABGR64Float" },
        { format_t::fRGB96Float, "fRGB96Float" },
        { format_t::fBGR96Float, "fBGR96Float" },
        { format_t::fRGBA128Float, "fRGBA128Float" },
        { format_t::fARGB128Float, "fARGB128Float" },
        { format_t::fBGRA128Float, "fBGRA128Float" },
        { format_t::fABGR128Float, "fABGR128Float" },
        { format_t::fYV12, "fYV12" },
        { format_t::fIYUV, "fIYUV" },
        { format_t::fYUY2, "fYUY2" },
        { format_t::fUYVY, "fUYVY" },
        { format_t::fYVYU, "fYVYU" },
        { format_t::fNV12, "fNV12" },
        { format_t::fNV21, "fNV21" },
        { format_t::fP010, "fP010" },
        { format_t::externalOES, "externalOES" },
        { format_t::fMJPG, "fMJPG" },
    };

    if ( l_lookUpTable.contains( _format ) ) {
        l_returnValue = l_lookUpTable.at( _format );

    } else {
        l_returnValue = ( "unknown" );
    }

    return ( l_returnValue );
}

[[nodiscard]] auto pixelFormatDetails( format_t _format )
    -> const pixelFormatDetails_t {
    return (
        std::bit_cast< SDL_PixelFormatDetails* >( SDL_GetPixelFormatDetails(
            static_cast< SDL_PixelFormat >( _format ) ) ) );
}

} // namespace pixels

} // namespace slickdl
