#pragma once

#include <SDL3/SDL_scancode.h>

#include <cstdint>
#include <format>

namespace slickdl {

using scancode_t = enum class scancode : uint16_t {
    unknown = 0,

    //  These values are from usage page 0x07 (USB keyboard page)

    a = 4,
    b = 5,
    c = 6,
    d = 7,
    e = 8,
    f = 9,
    g = 10,
    h = 11,
    i = 12,
    j = 13,
    k = 14,
    l = 15,
    m = 16,
    n = 17,
    o = 18,
    p = 19,
    q = 20,
    r = 21,
    s = 22,
    t = 23,
    u = 24,
    v = 25,
    w = 26,
    x = 27,
    y = 28,
    z = 29,

    number1 = 30,
    number2 = 31,
    number3 = 32,
    number4 = 33,
    number5 = 34,
    number6 = 35,
    number7 = 36,
    number8 = 37,
    number9 = 38,
    number0 = 39,

    enter = 40,
    escape = 41,
    backspace = 42,
    tab = 43,
    space = 44,

    minus = 45,
    equals = 46,
    leftParenthesis = 47,
    rightParenthesis = 48,
    backslash = 49, /**< Located at the lower left of the return
                     *   key on ISO keyboards and at the right end
                     *   of the QWERTY row on ANSI keyboards
                     *   Produces REVERSE SOLIDUS (backslash) and
                     *   VERTICAL LINE in a US layout, REVERSE
                     *   SOLIDUS and VERTICAL LINE in a UK Mac
                     *   layout, NUMBER SIGN and TILDE in a UK
                     *   Windows layout, DOLLAR SIGN and POUND SIGN
                     *   in a Swiss German layout, NUMBER SIGN and
                     *   APOSTROPHE in a German layout, GRAVE
                     *   ACCENT and POUND SIGN in a French Mac
                     *   layout, and ASTERISK and MICRO SIGN in a
                     *   French Windows layout
                     */
    nonushash = 50, /**< ISO USB keyboards actually use this code
                     *   instead of 49 for the same key, but all
                     *   OSes I've seen treat the two codes
                     *   identically. So, as an implementor, unless
                     *   your keyboard generates both of those
                     *   codes and your OS treats them differently,
                     *   you should generate backslash
                     *   instead of this code. As a user, you
                     *   should not rely on this code because SDL
                     *   will never generate it with most (all?)
                     *   keyboards
                     */
    semicolon = 51,
    apostrophe = 52,
    grave = 53, /**< Located in the top left corner (on both ANSI
                 *   and ISO keyboards). Produces GRAVE ACCENT and
                 *   TILDE in a US Windows layout and in US and UK
                 *   Mac layouts on ANSI keyboards, GRAVE ACCENT
                 *   and NOT SIGN in a UK Windows layout, SECTION
                 *   SIGN and PLUS-MINUS SIGN in US and UK Mac
                 *   layouts on ISO keyboards, SECTION SIGN and
                 *   DEGREE SIGN in a Swiss German layout (Mac:
                 *   only on ISO keyboards), CIRCUMFLEX ACCENT and
                 *   DEGREE SIGN in a German layout (Mac: only on
                 *   ISO keyboards), SUPERSCRIPT TWO and TILDE in a
                 *   French Windows layout, COMMERCIAL AT and
                 *   NUMBER SIGN in a French Mac layout on ISO
                 *   keyboards, and LESS-THAN SIGN and GREATER-THAN
                 *   SIGN in a Swiss German, German, or French Mac
                 *   layout on ANSI keyboards
                 */
    comma = 54,
    period = 55,
    slash = 56,

    capsLock = 57,

    f1 = 58,
    f2 = 59,
    f3 = 60,
    f4 = 61,
    f5 = 62,
    f6 = 63,
    f7 = 64,
    f8 = 65,
    f9 = 66,
    f10 = 67,
    f11 = 68,
    f12 = 69,

    printScreen = 70,
    scrollLock = 71,
    pause = 72,
    insert = 73, /**< insert on PC, help on some Mac keyboards (but
                                   does send code 73, not 117) */
    home = 74,
    pageup = 75,
    erase = 76, /**< delete */
    end = 77,
    pagedown = 78,
    right = 79,
    left = 80,
    down = 81,
    up = 82,

    numLockClear = 83, /**< num lock on PC, clear on Mac keyboards
                        */
    kpDivide = 84,
    kpMultiply = 85,
    kpMinus = 86,
    kpPlus = 87,
    kpEnter = 88,
    kp1 = 89,
    kp2 = 90,
    kp3 = 91,
    kp4 = 92,
    kp5 = 93,
    kp6 = 94,
    kp7 = 95,
    kp8 = 96,
    kp9 = 97,
    kp0 = 98,
    kpPeriod = 99,

    nonusBackslash = 100, /**< This is the additional key that ISO
                           *   keyboards have over ANSI ones,
                           *   located between left shift and Y
                           *   Produces GRAVE ACCENT and TILDE in a
                           *   US or UK Mac layout, REVERSE SOLIDUS
                           *   (backslash) and VERTICAL LINE in a
                           *   US or UK Windows layout, and
                           *   LESS-THAN SIGN and GREATER-THAN SIGN
                           *   in a Swiss German, German, or French
                           *   layout. */
    application = 101,    /**< windows contextual menu, compose */
    power = 102,          /**< The USB document says this is a status flag,
                           *   not a physical key - but some Mac keyboards
                           *   do have a power key. */
    kpEquals = 103,
    f13 = 104,
    f14 = 105,
    f15 = 106,
    f16 = 107,
    f17 = 108,
    f18 = 109,
    f19 = 110,
    f20 = 111,
    f21 = 112,
    f22 = 113,
    f23 = 114,
    f24 = 115,
    execute = 116,
    help = 117, /**< AL Integrated Help Center */
    menu = 118, /**< Menu (show menu) */
    select = 119,
    stop = 120,  /**< AC Stop */
    again = 121, /**< AC Redo/Repeat */
    undo = 122,  /**< AC Undo */
    cut = 123,   /**< AC Cut */
    copy = 124,  /**< AC Copy */
    paste = 125, /**< AC Paste */
    find = 126,  /**< AC Find */
    mute = 127,
    volumeUp = 128,
    volumeDown = 129,
    // not sure whether there's a reason to enable these
    //     lockingCapsLock = 130,
    //     lockingNumLock = 131,
    //     lockingScrollLock = 132,
    kpComma = 133,
    kpEqualsas400 = 134,

    international1 = 135, /**< used on Asian keyboards, see
                                            footnotes in USB doc */
    international2 = 136,
    international3 = 137, /**< Yen */
    international4 = 138,
    international5 = 139,
    international6 = 140,
    international7 = 141,
    international8 = 142,
    international9 = 143,
    lang1 = 144, /**< Hangul/English toggle */
    lang2 = 145, /**< Hanja conversion */
    lang3 = 146, /**< Katakana */
    lang4 = 147, /**< Hiragana */
    lang5 = 148, /**< Zenkaku/Hankaku */
    lang6 = 149, /**< reserved */
    lang7 = 150, /**< reserved */
    lang8 = 151, /**< reserved */
    lang9 = 152, /**< reserved */

    alterase = 153, /**< Erase-Eaze */
    sysreq = 154,
    cancel = 155, /**< AC Cancel */
    clear = 156,
    prior = 157,
    enter2 = 158,
    separator = 159,
    out = 160,
    oper = 161,
    clearAgain = 162,
    crsel = 163,
    exsel = 164,

    kp00 = 176,
    kp000 = 177,
    thousandsSeparator = 178,
    decimalSeparator = 179,
    currencyUnit = 180,
    currencySubunit = 181,
    kpLeftParenthesis = 182,
    kpRightParenthesis = 183,
    kpLeftBrace = 184,
    kpRightBrace = 185,
    kpTab = 186,
    kpBackspace = 187,
    kpA = 188,
    kpB = 189,
    kpC = 190,
    kpD = 191,
    kpE = 192,
    kpF = 193,
    kpXor = 194,
    kpPower = 195,
    kpPercent = 196,
    kpLess = 197,
    kpGreater = 198,
    kpAmpersand = 199,
    kpDoubleampersand = 200,
    kpVerticalbar = 201,
    kpDoubleverticalbar = 202,
    kpColon = 203,
    kpHash = 204,
    kpSpace = 205,
    kpAt = 206,
    kpExclam = 207,
    kpMemStore = 208,
    kpMemRecall = 209,
    kpMemClear = 210,
    kpMemAdd = 211,
    kpMemSubtract = 212,
    kpMemMultiply = 213,
    kpMemDivide = 214,
    kpPlusMinus = 215,
    kpClear = 216,
    kpClearEntry = 217,
    kpBinary = 218,
    kpOctal = 219,
    kpDecimal = 220,
    kpHexadecimal = 221,

    leftCtrl = 224,
    leftShift = 225,
    leftAlt = 226,  /**< alt, option */
    leftMeta = 227, /**< windows, command (apple), meta */
    rightCtrl = 228,
    rightShift = 229,
    rightAlt = 230,  /**< alt gr, option */
    rightMeta = 231, /**< windows, command (apple), meta */

    mode = 257, /**< I'm not sure if this is really not covered
                 *   by any of the above, but since there's a
                 *   special KMOD_MODE for it I'm adding it here
                 */

    //  These values are mapped from usage page 0x0C (USB consumer page)
    //
    //  There are way more keys in the spec than we can represent in the
    //  current scancode range, so pick the ones that commonly come up in
    //  real world usage

    sleep = 258, /**< Sleep */
    wake = 259,  /**< Wake */

    channelIncrement = 260, /**< Channel Increment */
    channelDecrement = 261, /**< Channel Decrement */

    mediaPlay = 262,          /**< Play */
    mediaPause = 263,         /**< Pause */
    mediaRecord = 264,        /**< Record */
    mediaFastForward = 265,   /**< Fast Forward */
    mediaRewind = 266,        /**< Rewind */
    mediaNextTrack = 267,     /**< Next Track */
    mediaPreviousTrack = 268, /**< Previous Track */
    mediaStop = 269,          /**< Stop */
    mediaEject = 270,         /**< Eject */
    mediaPlayPause = 271,     /**< Play / Pause */
    mediaSelect = 272,        /**< Media Select */

    acNew = 273,        /**< AC New */
    acOpen = 274,       /**< AC Open */
    acClose = 275,      /**< AC Close */
    acExit = 276,       /**< AC Exit */
    acSave = 277,       /**< AC Save */
    acPrint = 278,      /**< AC Print */
    acProperties = 279, /**< AC Properties */

    acSearch = 280,    /**< AC Search */
    acHome = 281,      /**< AC Home */
    acBack = 282,      /**< AC Back */
    acForward = 283,   /**< AC Forward */
    acStop = 284,      /**< AC Stop */
    acRefresh = 285,   /**< AC Refresh */
    acBookmarks = 286, /**< AC Bookmarks */

    //  Mobile keys
    //
    //  These are values that are often used on mobile phones

    softLeft = 287,  /**< Usually situated below the display on phones and
                          used as a multi-function feature key for selecting
                          a software defined function shown on the bottom left
                          of the display. */
    softRight = 288, /**< Usually situated below the display on phones and
                          used as a multi-function feature key for selecting
                          a software defined function shown on the bottom right
                          of the display. */
    call = 289,      /**< Used for accepting phone calls. */
    endCall = 290,   /**< Used for rejecting phone calls. */

    // Add any other keys here

    reserved = 400, /**< 400-500 reserved for dynamic keycodes */

    count = 512
};

// Legacy
[[nodiscard]] constexpr auto toLegacy( scancode_t _scancode ) -> SDL_Scancode {
    return ( static_cast< SDL_Scancode >( _scancode ) );
}

[[nodiscard]] constexpr auto toLegacy( scancode_t* _scancode )
    -> SDL_Scancode* {
    return ( std::bit_cast< SDL_Scancode* >( _scancode ) );
}

[[nodiscard]] constexpr auto fromLegacy( SDL_Scancode _scancode )
    -> scancode_t {
    return ( static_cast< scancode_t >( _scancode ) );
}

} // namespace slickdl

template <>
struct std::formatter< slickdl::scancode_t, char > {
    constexpr auto parse( std::format_parse_context& _context ) {
        return ( _context.begin() );
    }

    auto format( slickdl::scancode_t& _value,
                 std::format_context& _context ) const {
        return ( std::format_to( _context.out(), "{}", _value ) );
    }
};
