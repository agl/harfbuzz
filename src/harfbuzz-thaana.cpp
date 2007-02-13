/*******************************************************************
 *
 *  Copyright 2007  Trolltech ASA
 *
 *  This is part of HarfBuzz, an OpenType Layout engine library.
 *
 *  See the file name COPYING for licensing information.
 *
 ******************************************************************/

#if 0
static bool thaana_shape(QShaperItem *item)
{
    Q_ASSERT(item->script == QUnicodeTables::Thaana);

#ifndef QT_NO_OPENTYPE
    QOpenType *openType = item->font->openType();

    if (openType && openType->supportsScript(item->script)) {
        openType->selectScript(item, QUnicodeTables::Thaana);
        const int availableGlyphs = item->num_glyphs;
        if (!item->font->stringToCMap(item->string->unicode()+item->from, item->length, item->glyphs, &item->num_glyphs, QFlag(item->flags)))
            return false;
        heuristicSetGlyphAttributes(item);
        openType->shape(item);
        return openType->positionAndAdd(item, availableGlyphs);
    }
#endif
    return basic_shape(item);
}
#endif
