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

typedef enum 
{
    HB_Combining_BelowLeftAttached       = 200,
    HB_Combining_BelowAttached           = 202,
    HB_Combining_BelowRightAttached      = 204,
    HB_Combining_LeftAttached            = 208,
    HB_Combining_RightAttached           = 210,
    HB_Combining_AboveLeftAttached       = 212,
    HB_Combining_AboveAttached           = 214,
    HB_Combining_AboveRightAttached      = 216,

    HB_Combining_BelowLeft               = 218,
    HB_Combining_Below                   = 220,
    HB_Combining_BelowRight              = 222,
    HB_Combining_Left                    = 224,
    HB_Combining_Right                   = 226,
    HB_Combining_AboveLeft               = 228,
    HB_Combining_Above                   = 230,
    HB_Combining_AboveRight              = 232,

    HB_Combining_DoubleBelow             = 233,
    HB_Combining_DoubleAbove             = 234,
    HB_Combining_IotaSubscript           = 240
} HB_CombiningClass;


// return true if ok.
typedef HB_Bool (*HB_ShapeFunction)(HB_ShaperItem *shaper_item);
typedef void (*HB_AttributeFunction)(HB_Script script, const HB_UChar16 *string, uint32_t from, uint32_t len, HB_CharAttributes *attributes);

typedef struct {
    HB_ShapeFunction shape;
    HB_AttributeFunction charAttributes;
} HB_ScriptEngine;

extern const HB_ScriptEngine hb_scriptEngines[];

extern HB_Bool HB_TibetanShape(HB_ShaperItem *shaper_item);

extern void HB_TibetanAttributes(HB_Script script, const HB_UChar16 *string, uint32_t from, uint32_t len, HB_CharAttributes *attributes);

extern void HB_MyanmarAttributes(HB_Script script, const HB_UChar16 *string, uint32_t from, uint32_t len, HB_CharAttributes *attributes);

extern void HB_KhmerAttributes(HB_Script script, const HB_UChar16 *string, uint32_t from, uint32_t len, HB_CharAttributes *attributes);

extern void HB_IndicAttributes(HB_Script script, const HB_UChar16 *string, uint32_t from, uint32_t len, HB_CharAttributes *attributes);

typedef struct {
    uint32_t tag;
    uint32_t property;
} HB_OpenTypeFeature;

enum { PositioningProperties = 0x80000000 };

void HB_SelectScript(HB_Face *face, HB_Script script, int flags, const HB_OpenTypeFeature *features);
HB_Bool HB_OpenTypeShape(HB_ShaperItem *item, const uint32_t *properties);
HB_Bool HB_OpenTypePosition(HB_ShaperItem *item, int availableGlyphs, HB_Bool doLogClusters);

FT_END_HEADER

#endif // HARFBUZZ_SHAPER_PRIVATE_H
