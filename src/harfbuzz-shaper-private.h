/*******************************************************************
 *
 *  Copyright 2007  Trolltech ASA
 *
 *  This is part of HarfBuzz, an OpenType Layout engine library.
 *
 *  See the file name COPYING for licensing information.
 *
 ******************************************************************/
#ifndef HARFBUZZ_SHAPER_PRIVATE_H
#define HARFBUZZ_SHAPER_PRIVATE_H

FT_BEGIN_HEADER

typedef struct {
    HB_Script script;
    const HB_UChar16 *string;
    int from;
    int length;
    HB_Font *font;
    HB_GlyphLayout *glyphs;
    int num_glyphs; // in: available glyphs out: glyphs used/needed
    unsigned short *log_clusters;
    int flags; // HB_StringToGlyphsFlags
    int shaperFlags; // HB_ShaperFlags
    HB_Bool kerning_applied; // out: kerning applied by shaper
    const HB_CharAttributes *charAttributes;
} HB_ShaperItem;

// return true if ok.
typedef HB_Bool (*HB_ShapeFunction)();
typedef void (*HB_AttributeFunction)(HB_Script script, const HB_UChar16 *string, uint32_t from, uint32_t len, HB_CharAttributes *attributes);

typedef struct {
    HB_ShapeFunction shape;
    HB_AttributeFunction charAttributes;
} HB_ScriptEngine;

extern const HB_ScriptEngine hb_scriptEngines[];

extern HB_Bool HB_TibetanShape();

extern void HB_TibetanAttributes(HB_Script script, const HB_UChar16 *string, uint32_t from, uint32_t len, HB_CharAttributes *attributes);

extern void HB_MyanmarAttributes(HB_Script script, const HB_UChar16 *string, uint32_t from, uint32_t len, HB_CharAttributes *attributes);

extern void HB_KhmerAttributes(HB_Script script, const HB_UChar16 *string, uint32_t from, uint32_t len, HB_CharAttributes *attributes);

extern void HB_IndicAttributes(HB_Script script, const HB_UChar16 *string, uint32_t from, uint32_t len, HB_CharAttributes *attributes);

typedef struct {
    uint32_t tag;
    uint32_t property;
} HB_OpenTypeFeature;

enum { PositioningProperties = 0x80000000 };

void HB_SelectScript(HB_Face *face, HB_Script script, int flags, HB_OpenTypeFeature *features);

FT_END_HEADER

#endif // HARFBUZZ_SHAPER_PRIVATE_H
