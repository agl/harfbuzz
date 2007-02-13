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

FT_END_HEADER

#endif // HARFBUZZ_SHAPER_PRIVATE_H
