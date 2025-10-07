#pragma once

#include <algorithm>
#include <gsl/pointers>
#include <optional>
#include <ranges>

#include "stdfunc.hpp"

namespace slickdl {

// TODO: Move out
template < typename T >
concept is_int_or_float =
    ( std::is_same_v< T, int > || std::is_same_v< T, float > );

// The structure that defines a point
template < typename T >
    requires is_int_or_float< T >
struct point {
    [[nodiscard]] constexpr auto operator<=>( const point< T >& _box ) const =
        default;

    T x, y;
};

template < typename T >
using point_t = point< T >;

template < typename T >
    requires is_int_or_float< T >
struct line {
    [[nodiscard]] constexpr auto operator<=>( const line< T >& _box ) const =
        default;

    point_t< T > start;
    point_t< T > end;
};

template < typename T >
using line_t = line< T >;

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

// A box, with the origin at the upper left
template < typename T >
    requires is_int_or_float< T >
struct box {
    // TODO: Improve
    using clippingZone_t = clippingZone_t< T >;

    [[nodiscard]] constexpr auto operator<=>( const box< T >& _box ) const =
        default;

    template < typename T2 >
    [[nodiscard]] constexpr operator box< T2 >() const {
        return ( box< T2 >{
            static_cast< T2 >( x ),
            static_cast< T2 >( y ),
            static_cast< T2 >( width ),
            static_cast< T2 >( height ),
        } );
    }

    // Determine whether a box has no area
    //
    // A box is considered "empty" for this function if
    // its width and/ or height are <= 0
    [[nodiscard]] constexpr auto empty() const -> bool {
        return ( !height || !height );
    }

    // Determine whether a point resides inside a box
    //
    // A point is considered part of a box if points x and y coordinates
    // are >= to the boxs top left corner, and < the boxs x +
    // width and y + height. So a 1x1 box considers point ( 0, 0 ) as
    // "inside" and ( 0, 1 ) as not
    [[nodiscard]] constexpr auto contains( const point_t< T >& _point ) const
        -> bool {
        return ( _inRange1D( _point.x, x, _right( *this ) ) &&
                 _inRange1D( _point.y, y, _bottom( *this ) ) );
    }

    // Determine whether a box resides inside a box
    [[nodiscard]] constexpr auto contains( const box< T >& _box ) const
        -> bool {
        return ( ( x <= _box.x ) && ( y <= _box.y ) &&
                 ( _right( *this ) >= _rigth( _box ) ) &&
                 ( _bottom( *this ) >= _bottom( _box ) ) );
    }

    // Determine whether two boxs intersect
    [[nodiscard]] constexpr auto intersects( const box< T >& _box ) const
        -> bool {
        bool l_returnValue = false;

        do {
            if ( empty() || _box.empty() ) {
                break;
            }

#if 0
    // FIX: Check if my is the same
    // Horizontal intersection
    Amin = A->x;
    Amax = Amin + A->w;
    Bmin = B->x;
    Bmax = Bmin + B->w;
    if (Bmin > Amin) {
        Amin = Bmin;
    }
    if (Bmax < Amax) {
        Amax = Bmax;
    }
    if ((Amax - ENCLOSEPOINTS_EPSILON) < Amin) {
        return false;
    }
    // Vertical intersection
    Amin = A->y;
    Amax = Amin + A->h;
    Bmin = B->y;
    Bmax = Bmin + B->h;
    if (Bmin > Amin) {
        Amin = Bmin;
    }
    if (Bmax < Amax) {
        Amax = Bmax;
    }
    if ((Amax - ENCLOSEPOINTS_EPSILON) < Amin) {
        return false;
    }
    return true;
#endif

            l_returnValue =
                ( x < _right( _box ) ) && ( _right( *this ) > _box.x ) &&
                ( y < _bottom( _box ) ) && ( _bottom( *this ) > _box.y );
        } while ( false );

        return ( l_returnValue );
    }

    // Calculate the intersection of two boxs
    [[nodiscard]] constexpr auto intersection( const box< T >& _box ) const
        -> std::optional< box< T > > {
        std::optional< box< T > > l_returnValue = std::nullopt;

        do {
            if ( empty() || _box.empty() ) {
                break;
            }

#if 0
        // FIX: Check if my is the same
        // Horizontal intersection
        Amin = A->x;
        Amax = Amin + A->w;
        Bmin = B->x;
        Bmax = Bmin + B->w;
        if (Bmin > Amin) {
            Amin = Bmin;
        }
        result->x = Amin;
        if (Bmax < Amax) {
            Amax = Bmax;
        }
        result->w = Amax - Amin;

        // Vertical intersection
        Amin = A->y;
        Amax = Amin + A->h;
        Bmin = B->y;
        Bmax = Bmin + B->h;
        if (Bmin > Amin) {
            Amin = Bmin;
        }
        result->y = Amin;
        if (Bmax < Amax) {
            Amax = Bmax;
        }
        result->h = Amax - Amin;
#endif

            const box< T > l_composite = composite( _box );

            if ( ( l_composite ) &&
                 ( ( l_composite.x < l_composite.width ) &&
                   ( l_composite.y < l_composite.height ) ) ) {
                l_returnValue = l_composite;
            }
        } while ( false );

        return ( l_returnValue );
    }

    // Calculate the composite of two boxs
    [[nodiscard]] constexpr auto composite( const box< T >& _box ) const
        -> std::optional< box< T > > {
        std::optional< box< T > > l_returnValue = std::nullopt;

        do {
            // TODO: Maybe improve all IFs
            if ( empty() && _box.empty() ) {
                break;
            }

            if ( !empty() && _box.empty() ) {
                l_returnValue = *this;
            }

            if ( empty() && !_box.empty() ) {
                l_returnValue = _box;
            }

#if 0
            // FIX: CHeck if my is the same
            // Horizontal union
            Amin = A->x;
            Amax = Amin + A->w;
            Bmin = B->x;
            Bmax = Bmin + B->w;
            if ( Bmin < Amin ) {
                Amin = Bmin;
            }
            result->x = Amin;
            if ( Bmax > Amax ) {
                Amax = Bmax;
            }
            result->w = Amax - Amin;

            // Vertical union
            Amin = A->y;
            Amax = Amin + A->h;
            Bmin = B->y;
            Bmax = Bmin + B->h;
            if ( Bmin < Amin ) {
                Amin = Bmin;
            }
            result->y = Amin;
            if ( Bmax > Amax ) {
                Amax = Bmax;
            }
            result->h = Amax - Amin;
#endif

            T l_x1 = std::min( x, _box.x );
            T l_y1 = std::min( y, _box.y );
            T l_x2 = std::max( _right( *this ), _right( _box ) );
            T l_y2 = std::max( _bottom( *this ), _bottom( _box ) );

            l_returnValue = {
                l_x1,
                l_y1,
                ( l_x2 - l_x1 ),
                ( l_y2 - l_y1 ),
            };
        } while ( false );

        return ( l_returnValue );
    }

    // View of points inside a clipping zone
    // TODO :Improve return
    [[nodiscard]] constexpr auto encloses(
        std::span< const point_t< T > > _points,
        std::optional< const box< T > >& _clippingZone = std::nullopt ) const
        -> auto {
        if ( _clippingZone.empty() ) {
            return ( std::views::empty< const box< T > > );
        }

        clippingZone_t l_clippingZone = {
            _clippingZone.x, _clippingZone.y,
            _right( _clippingZone ),  // - ENCLOSEPOINTS_EPSILON
            _bottom( _clippingZone ), // - ENCLOSEPOINTS_EPSILON
        };

        return (
            _points |
            std::views::filter( [ & ]( const point_t< T >& _point ) -> bool {
                return ( _inRange2D( _point, l_clippingZone ) );
            } ) );
    }

    // Calculate a minimal box enclosing a set of points
    [[nodiscard]] constexpr auto enclosing(
        std::span< const point_t< T > > _points,
        std::optional< const box< T > >& _clippingZone = std::nullopt ) const
        -> std::optional< box< T > > {
        std::optional< box< T > > l_returnValue = std::nullopt;

        do {
            // TODO: Maybe improve
            if ( _points.empty() ) {
                break;
            }

            constexpr auto l_tryExpandClippingZone =
                []( const clippingZone_t& _clippingZone,
                    const point_t< T >& _point ) -> clippingZone_t {
                return ( clippingZone_t{
                    std::min( _clippingZone.minX, _point.x ),
                    std::min( _clippingZone.minY, _point.y ),
                    std::max( _clippingZone.maxX, _point.x ),
                    std::max( _clippingZone.maxY, _point.y ),
                } );
            };

            std::optional< clippingZone_t > l_clippingZone;

            if ( _clippingZone ) {
                l_clippingZone = std::ranges::fold_left(
                    encloses( _points, _clippingZone ),
                    std::optional< clippingZone_t >{},
                    []( const std::optional< clippingZone_t >& _accumulator,
                        const point_t< T >& _point ) -> clippingZone_t {
                        clippingZone_t l_returnValue;

                        if ( !_accumulator ) {
                            // First point initializes the box
                            l_returnValue = {
                                _point.x,
                                _point.y,
                                _point.x,
                                _point.y,
                            };

                        } else {
                            // Expand the box
                            l_returnValue =
                                l_tryExpandClippingZone( _accumulator, _point );
                        }

                        return ( l_returnValue );
                    } );

            } else {
                // No clipping, always add the first point
                const auto& l_point = _points.front();

                l_clippingZone =
                    std::ranges::fold_left( _points | std::views::drop( 1 ),
                                            clippingZone_t{
                                                l_point.x,
                                                l_point.y,
                                                l_point.x,
                                                l_point.y,
                                            },
                                            l_tryExpandClippingZone );
            }

            if ( l_clippingZone ) {
                l_returnValue.x = l_clippingZone.minX;
                l_returnValue.y = l_clippingZone.minY;
                l_returnValue.w =
                    ( l_clippingZone.maxX -
                      l_clippingZone.minX ); // + ENCLOSEPOINTS_EPSILON
                l_returnValue.h =
                    ( l_clippingZone.maxY -
                      l_clippingZone.minY ); // + ENCLOSEPOINTS_EPSILON
            }
        } while ( false );

        return ( l_returnValue );
    }

    // Calculate the intersection of a box and line segment
    //
    // This function is used to clip a line segment to a box. A line segment
    // contained entirely within the box or that does not intersect will
    // remain unchanged. A line segment that crosses the box at either or
    // both ends will be clipped to the boundary of the box
    [[nodiscard]] constexpr auto intersection( const line_t< T >& _line ) const
        -> std::optional< line_t< T > > {
        std::optional< line_t< T > > l_returnValue = std::nullopt;

        using bigT_t =
            std::conditional_t< std::is_integral_v< T >, int64_t, double >;

        const size_t l_enclosePointsEpsilon =
            ( ( std::is_integral_v< T > ) ? ( 1 ) : ( 0 ) );

        const size_t l_bottom = 1;
        const size_t l_top = 2;
        const size_t l_left = 4;
        const size_t l_right = 8;

        // Cohen-Sutherland algorithm for line clipping
        auto l_computeOutCode = [ & ]( T _x, T _y ) -> size_t {
            size_t l_code = 0;

            if ( _y < y ) {
                l_code |= l_top;

            } else if ( _y > ( _bottom( *this ) - l_enclosePointsEpsilon ) ) {
                l_code |= l_bottom;
            }

            if ( _x < x ) {
                l_code |= l_left;

            } else if ( _x > ( _right( *this ) - l_enclosePointsEpsilon ) ) {
                l_code |= l_right;
            }

            return ( l_code );
        };

        T l_x123 = 0;
        T l_y123 = 0;
        int l_outcode1 = 0, l_outcode2 = 0;

        if ( empty() ) {
            return std::nullopt;
        }

        line_t< T > l_line = _line;

        T l_rectx1 = x;
        T l_recty1 = y;
        T l_rectx2 = x + width - l_enclosePointsEpsilon;
        T l_recty2 = y + height - l_enclosePointsEpsilon;

        // Check to see if entire line is inside rect
        if ( l_line.start.x >= l_rectx1 && l_line.start.x <= l_rectx2 &&
             l_line.end.x >= l_rectx1 && l_line.end.x <= l_rectx2 &&
             l_line.start.y >= l_recty1 && l_line.start.y <= l_recty2 &&
             l_line.end.y >= l_recty1 && l_line.end.y <= l_recty2 ) {
            return true;
        }

        // Check to see if entire line is to one side of rect
        if ( ( l_line.start.x < l_rectx1 && l_line.end.x < l_rectx1 ) ||
             ( l_line.start.x > l_rectx2 && l_line.end.x > l_rectx2 ) ||
             ( l_line.start.y < l_recty1 && l_line.end.y < l_recty1 ) ||
             ( l_line.start.y > l_recty2 && l_line.end.y > l_recty2 ) ) {
            return false;
        }

        if ( l_line.start.y == l_line.end.y ) { // Horizontal line, easy to clip
            if ( l_line.start.x < l_rectx1 ) {
                l_line.start.x = l_rectx1;

            } else if ( l_line.start.x > l_rectx2 ) {
                l_line.start.x = l_rectx2;
            }

            if ( l_line.end.x < l_rectx1 ) {
                l_line.end.x = l_rectx1;

            } else if ( l_line.end.x > l_rectx2 ) {
                l_line.end.x = l_rectx2;
            }

            return true;
        }

        if ( l_line.start.x == l_line.end.x ) { // Vertical line, easy to clip
            if ( l_line.start.y < l_recty1 ) {
                l_line.start.y = l_recty1;
            } else if ( l_line.start.y > l_recty2 ) {
                l_line.start.y = l_recty2;
            }

            if ( l_line.end.y < l_recty1 ) {
                l_line.end.y = l_recty1;

            } else if ( l_line.end.y > l_recty2 ) {
                l_line.end.y = l_recty2;
            }

            return true;
        }

        // More complicated Cohen-Sutherland algorithm
        l_outcode1 = l_computeOutCode( l_line.start.x, l_line.start.y );
        l_outcode2 = l_computeOutCode( l_line.end.x, l_line.end.y );

        while ( l_outcode1 || l_outcode2 ) {
            if ( l_outcode1 & l_outcode2 ) {
                return false;
            }

            if ( l_outcode1 ) {
                if ( l_outcode1 & l_top ) {
                    l_y123 = l_recty1;
                    l_x123 =
                        ( T )( l_line.start.x +
                               ( ( bigT_t )( l_line.end.x - l_line.start.x ) *
                                 ( l_y123 - l_line.start.y ) ) /
                                   ( l_line.end.y - l_line.start.y ) );
                } else if ( l_outcode1 & l_bottom ) {
                    l_y123 = l_recty2;
                    l_x123 =
                        ( T )( l_line.start.x +
                               ( ( bigT_t )( l_line.end.x - l_line.start.x ) *
                                 ( l_y123 - l_line.start.y ) ) /
                                   ( l_line.end.y - l_line.start.y ) );
                } else if ( l_outcode1 & l_left ) {
                    l_x123 = l_rectx1;
                    l_y123 =
                        ( T )( l_line.start.y +
                               ( ( bigT_t )( l_line.end.y - l_line.start.y ) *
                                 ( l_x123 - l_line.start.x ) ) /
                                   ( l_line.end.x - l_line.start.x ) );
                } else if ( l_outcode1 & l_right ) {
                    l_x123 = l_rectx2;
                    l_y123 =
                        ( T )( l_line.start.y +
                               ( ( bigT_t )( l_line.end.y - l_line.start.y ) *
                                 ( l_x123 - l_line.start.x ) ) /
                                   ( l_line.end.x - l_line.start.x ) );
                }

                l_line.start.x = l_x123;
                l_line.start.y = l_y123;

                l_outcode1 = l_computeOutCode( l_x123, l_y123 );

            } else {
                if ( l_outcode2 & l_top ) {
                    stdfunc::assert(
                        l_line.end.y !=
                        l_line.start.y ); // if equal: division by zero.

                    l_y123 = l_recty1;
                    l_x123 =
                        ( T )( l_line.start.x +
                               ( ( bigT_t )( l_line.end.x - l_line.start.x ) *
                                 ( l_y123 - l_line.start.y ) ) /
                                   ( l_line.end.y - l_line.start.y ) );

                } else if ( l_outcode2 & l_bottom ) {
                    stdfunc::assert(
                        l_line.end.y !=
                        l_line.start.y ); // if equal: division by zero.

                    l_y123 = l_recty2;
                    l_x123 =
                        ( T )( l_line.start.x +
                               ( ( bigT_t )( l_line.end.x - l_line.start.x ) *
                                 ( l_y123 - l_line.start.y ) ) /
                                   ( l_line.end.y - l_line.start.y ) );
                } else if ( l_outcode2 & l_left ) {
                    /* If this assertion ever fires, here's the static analysis
                       that warned about it:
                       http://buildbot.libsdl.org/sdl-static-analysis/sdl-macosx-static-analysis/sdl-macosx-static-analysis-1101/report-b0d01a.html#EndPath
                     */
                    stdfunc::assert(
                        l_line.end.x !=
                        l_line.start.x ); // if equal: division by zero.

                    l_x123 = l_rectx1;
                    l_y123 =
                        ( T )( l_line.start.y +
                               ( ( bigT_t )( l_line.end.y - l_line.start.y ) *
                                 ( l_x123 - l_line.start.x ) ) /
                                   ( l_line.end.x - l_line.start.x ) );
                } else if ( l_outcode2 & l_right ) {
                    /* If this assertion ever fires, here's the static analysis
                       that warned about it:
                       http://buildbot.libsdl.org/sdl-static-analysis/sdl-macosx-static-analysis/sdl-macosx-static-analysis-1101/report-39b114.html#EndPath
                     */
                    stdfunc::assert(
                        l_line.end.x !=
                        l_line.start.x ); // if equal: division by zero.

                    l_x123 = l_rectx2;
                    l_y123 =
                        ( T )( l_line.start.y +
                               ( ( bigT_t )( l_line.end.y - l_line.start.y ) *
                                 ( l_x123 - l_line.start.x ) ) /
                                   ( l_line.end.x - l_line.start.x ) );
                }

                l_line.end.x = l_x123;
                l_line.end.y = l_y123;

                l_outcode2 = l_computeOutCode( l_x123, l_y123 );
            }
        }

        l_returnValue = l_line;

        return ( l_returnValue );
    }

    T x, y, width, height;

    // Helpers
private:
    [[nodiscard]] constexpr auto _inRange1D( T _point, T _min, T _max ) const
        -> bool {
        return ( ( _point >= _min ) && ( _point < _max ) );
    }

    [[nodiscard]] constexpr auto _inRange2D(
        const point_t< T >& _point,
        const clippingZone_t& _clippingZone ) const -> bool {
        return (
            _inRange1D( _point.x, _clippingZone.minX, _clippingZone.maxX ) &&
            _inRange1D( _point.y, _clippingZone.minY, _clippingZone.maxY ) );
    }

    [[nodiscard]] constexpr auto _right( const box< T >& _box ) const -> T {
        return ( _box.x + _box.width );
    }

    [[nodiscard]] constexpr auto _bottom( const box< T >& _box ) const -> T {
        return ( _box.y + _box.height );
    }
};

template < typename T >
using box_t = box< T >;

#if 0
// Determine whether two floating point boxs are equal, within some
// given epsilon
//
// boxs are considered equal if  each of their x, y, width and height are within
// `epsilon` of each other. If you dont know what value to use for `epsilon`,
// you should call the SDL_RectsEqualFloat function instead
[[nodiscard]] constexpr auto rectsEqualEpsilon( const box<float> a,
                               const box<float> b,
                               float epsilon ) -> bool {
    return ( ( ( a == b ) || ( ( SDL_fabsf( a->x - b->x ) <= epsilon ) &&
                               ( SDL_fabsf( a->y - b->y ) <= epsilon ) &&
                               ( SDL_fabsf( a->w - b->w ) <= epsilon ) &&
                               ( SDL_fabsf( a->h - b->h ) <= epsilon ) ) ) );
}
#endif

} // namespace slickdl
