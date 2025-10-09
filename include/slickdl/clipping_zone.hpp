#pragma once

#include "slickdl.hpp"

namespace slickdl {

template < typename T >
    requires is_int_or_float< T >
struct clippingZone {
    [[nodiscard]] constexpr auto operator<=>(
        const clippingZone< T >& _box ) const = default;

    T minX, minY, maxX, maxY;
};

template < typename T >
    requires is_int_or_float< T >
using clippingZone_t = struct clippingZone< T >;

} // namespace slickdl
