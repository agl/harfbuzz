/*******************************************************************
 *
 *  Copyright 2007  Trolltech ASA
 *
 *  This is part of HarfBuzz, an OpenType Layout engine library.
 *
 *  See the file name COPYING for licensing information.
 *
 ******************************************************************/
#ifndef HARFBUZZ_SHAPER_H
#define HARFBUZZ_SHAPER_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include <harfbuzz.h>

FT_BEGIN_HEADER

#include <stdint.h>

typedef enum {
        HB_Script_Common,
        HB_Script_Greek,
        HB_Script_Cyrillic,
        HB_Script_Armenian,
        HB_Script_Hebrew,
        HB_Script_Arabic,
        HB_Script_Syriac,
        HB_Script_Thaana,
        HB_Script_Devanagari,
        HB_Script_Bengali,
        HB_Script_Gurmukhi,
        HB_Script_Gujarati,
        HB_Script_Oriya,
        HB_Script_Tamil,
        HB_Script_Telugu,
        HB_Script_Kannada,
        HB_Script_Malayalam,
        HB_Script_Sinhala,
        HB_Script_Thai,
        HB_Script_Lao,
        HB_Script_Tibetan,
        HB_Script_Myanmar,
        HB_Script_Georgian,
        HB_Script_Hangul,
        HB_Script_Ogham,
        HB_Script_Runic,
        HB_Script_Khmer,
        HB_Script_Inherited,
        HB_ScriptCount = HB_Script_Inherited
        /*
        HB_Script_Latin = Common,
        HB_Script_Ethiopic = Common,
        HB_Script_Cherokee = Common,
        HB_Script_CanadianAboriginal = Common,
        HB_Script_Mongolian = Common,
        HB_Script_Hiragana = Common,
        HB_Script_Katakana = Common,
        HB_Script_Bopomofo = Common,
        HB_Script_Han = Common,
        HB_Script_Yi = Common,
        HB_Script_OldItalic = Common,
        HB_Script_Gothic = Common,
        HB_Script_Deseret = Common,
        HB_Script_Tagalog = Common,
        HB_Script_Hanunoo = Common,
        HB_Script_Buhid = Common,
        HB_Script_Tagbanwa = Common,
        HB_Script_Limbu = Common,
        HB_Script_TaiLe = Common,
        HB_Script_LinearB = Common,
        HB_Script_Ugaritic = Common,
        HB_Script_Shavian = Common,
        HB_Script_Osmanya = Common,
        HB_Script_Cypriot = Common,
        HB_Script_Braille = Common,
        HB_Script_Buginese = Common,
        HB_Script_Coptic = Common,
        HB_Script_NewTaiLue = Common,
        HB_Script_Glagolitic = Common,
        HB_Script_Tifinagh = Common,
        HB_Script_SylotiNagri = Common,
        HB_Script_OldPersian = Common,
        HB_Script_Kharoshthi = Common,
        HB_Script_Balinese = Common,
        HB_Script_Cuneiform = Common,
        HB_Script_Phoenician = Common,
        HB_Script_PhagsPa = Common,
        HB_Script_Nko = Common
        */
} HB_Script;

typedef uint16_t HB_UChar16;
typedef uint32_t HB_UChar32;
typedef uint32_t HB_Glyph;
typedef uint8_t HB_Bool;
typedef uint32_t HB_Fixed; /* 26.6 */

typedef struct
{
    uint32_t pos;
    HB_Script script;
    uint8_t bidiLevel;
} HB_ScriptItem;

typedef enum {
    HB_NoBreak,
    HB_SoftHyphen,
    HB_Break,
    HB_ForcedBreak
} HB_LineBreakType;

// see http://www.unicode.org/reports/tr14/tr14-19.html
// we don't use the XX, AI and CB properties and map them to AL instead.
// as we don't support any EBDIC based OS'es, NL is ignored and mapped to AL as well.
typedef enum {
    HB_LineBreak_OP, HB_LineBreak_CL, HB_LineBreak_QU, HB_LineBreak_GL, HB_LineBreak_NS,
    HB_LineBreak_EX, HB_LineBreak_SY, HB_LineBreak_IS, HB_LineBreak_PR, HB_LineBreak_PO,
    HB_LineBreak_NU, HB_LineBreak_AL, HB_LineBreak_ID, HB_LineBreak_IN, HB_LineBreak_HY,
    HB_LineBreak_BA, HB_LineBreak_BB, HB_LineBreak_B2, HB_LineBreak_ZW, HB_LineBreak_CM,
    HB_LineBreak_WJ, HB_LineBreak_H2, HB_LineBreak_H3, HB_LineBreak_JL, HB_LineBreak_JV,
    HB_LineBreak_JT, HB_LineBreak_SA, HB_LineBreak_SG,
    HB_LineBreak_SP, HB_LineBreak_CR, HB_LineBreak_LF, HB_LineBreak_BK
} HB_LineBreakClass;

/* needs to be provided by the application/library */
HB_LineBreakClass HB_GetLineBreakClass(HB_UChar32 ch);

typedef struct {
    HB_LineBreakType lineBreakType  :2;
    HB_Bool whiteSpace              :1;     // A unicode whitespace character, except NBSP, ZWNBSP
    HB_Bool charStop                :1;     // Valid cursor position (for left/right arrow)
    uint8_t unused                  :4;
} HB_CharAttributes;

void HB_GetCharAttributes(const HB_UChar16 *string, uint32_t stringLength,
                          const HB_ScriptItem *items, uint32_t numItems,
                          HB_CharAttributes *attributes);

inline HB_Bool HB_IsHighSurrogate(HB_UChar16 ucs) {
    return ((ucs & 0xfc00) == 0xd800);
}

inline HB_Bool HB_IsLowSurrogate(HB_UChar16 ucs) {
    return ((ucs & 0xfc00) == 0xdc00);
}

inline HB_UChar32 HB_SurrogateToUcs4(HB_UChar16 high, HB_UChar16 low) {
    return (((HB_UChar32)high)<<10) + low - 0x35fdc00;
}

enum {
    HB_LeftToRight = 0,
    HB_RightToLeft = 1
} HB_StringToGlyphsFlags;

typedef struct HB_Font_ HB_Font;

typedef struct {
    HB_Bool (*stringToGlyphs)(HB_Font *font, const HB_UChar16 *string, uint32_t length, HB_Glyph *glyphs, uint32_t *numGlyphs, uint32_t flags);
    void    (*getMetrics)(HB_Font *font, const HB_Glyph *glyphs, int numGlyphs, HB_Fixed *advances);
} HB_FontClass;

typedef struct HB_Font_ {
    HB_FontClass *klass;
    FT_Face face;
} HB_Font;

typedef struct {
} HB_GlyphLayout;

enum {
    HB_ShaperFlag_Default = 0,
    HB_ShaperFlag_NoKerning = 1
} HB_ShaperFlag;

typedef struct {
    FT_Face freetypeFace;

    HB_GDEF gdef;
    HB_GSUB gsub;
    HB_GPOS gpos;
    HB_Bool supported_scripts[HB_ScriptCount];
    HB_Buffer buffer;
    HB_Script current_script;
    int current_flags;
    HB_Bool has_opentype_kerning;
#if 0
    HB_Bool positioned;
    HB_Bool glyphs_substituted;
    QGlyphLayout::Attributes *tmpAttributes;
    unsigned int *tmpLogClusters;
#endif
    int length;
    int orig_nglyphs;
} HB_Face;

HB_Face *HB_NewFace(FT_Face ftface);
void HB_FreeFace(HB_Face *face);

FT_END_HEADER

#endif // HARFBUZZ_SHAPER_H
