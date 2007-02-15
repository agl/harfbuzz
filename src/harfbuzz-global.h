/*******************************************************************
 *
 *  Copyright 2007  Trolltech ASA
 *
 *  This is part of HarfBuzz, an OpenType Layout engine library.
 *
 *  See the file name COPYING for licensing information.
 *
 ******************************************************************/
#ifndef HARFBUZZ_GLOBAL_H
#define HARFBUZZ_GLOBAL_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include <stdint.h>

typedef uint16_t HB_UChar16;
typedef uint32_t HB_UChar32;
typedef uint32_t HB_Glyph;
typedef uint8_t HB_Bool;
typedef uint32_t HB_Fixed; /* 26.6 */

typedef struct {
    HB_Fixed x;
    HB_Fixed y;
} HB_FixedPoint;

inline HB_Bool HB_IsHighSurrogate(HB_UChar16 ucs) {
    return ((ucs & 0xfc00) == 0xd800);
}

inline HB_Bool HB_IsLowSurrogate(HB_UChar16 ucs) {
    return ((ucs & 0xfc00) == 0xdc00);
}

inline HB_UChar32 HB_SurrogateToUcs4(HB_UChar16 high, HB_UChar16 low) {
    return (((HB_UChar32)high)<<10) + low - 0x35fdc00;
}


#endif
