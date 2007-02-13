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
// Uniscribe also defines dlig for Hebrew, but we leave this out for now, as it's mostly
// ligatures one does not want in modern Hebrew (as lam-alef ligatures).
#ifndef QT_NO_OPENTYPE
static const QOpenType::Features hebrew_features[] = {
    { FT_MAKE_TAG('c', 'c', 'm', 'p'), CcmpProperty },
    {0, 0}
};
#endif
#ifndef Q_WS_MAC
/* Hebrew shaping. In the non opentype case we try to use the
   presentation forms specified for Hebrew. Especially for the
   ligatures with Dagesh this gives much better results than we could
   achieve manually.
*/
static bool hebrew_shape(QShaperItem *item)
{
    Q_ASSERT(item->script == QUnicodeTables::Hebrew);

#ifndef QT_NO_OPENTYPE
    QOpenType *openType = item->font->openType();

    if (openType && openType->supportsScript(item->script)) {
        openType->selectScript(item, item->script, hebrew_features);

        const int availableGlyphs = item->num_glyphs;
        if (!item->font->stringToCMap(item->string->unicode()+item->from, item->length, item->glyphs, &item->num_glyphs, QFlag(item->flags)))
            return false;

        heuristicSetGlyphAttributes(item);
        openType->shape(item);
        return openType->positionAndAdd(item, availableGlyphs);
    }
#endif

    enum {
        Dagesh = 0x5bc,
        ShinDot = 0x5c1,
        SinDot = 0x5c2,
        Patah = 0x5b7,
        Qamats = 0x5b8,
        Holam = 0x5b9,
        Rafe = 0x5bf
    };
    unsigned short chars[512];
    QChar *shapedChars = item->length > 256 ? (QChar *)::malloc(2*item->length * sizeof(QChar)) : (QChar *)chars;

    const QChar *uc = item->string->unicode() + item->from;
    unsigned short *logClusters = item->log_clusters;
    QGlyphLayout *glyphs = item->glyphs;

    *shapedChars = *uc;
    logClusters[0] = 0;
    int slen = 1;
    int cluster_start = 0;
    for (int i = 1; i < item->length; ++i) {
        ushort base = shapedChars[slen-1].unicode();
        ushort shaped = 0;
        bool invalid = false;
        if (uc[i].unicode() == Dagesh) {
            if (base >= 0x5d0
                && base <= 0x5ea
                && base != 0x5d7
                && base != 0x5dd
                && base != 0x5df
                && base != 0x5e2
                && base != 0x5e5) {
                shaped = base - 0x5d0 + 0xfb30;
            } else if (base == 0xfb2a || base == 0xfb2b /* Shin with Shin or Sin dot */) {
                shaped = base + 2;
            } else {
                invalid = true;
            }
        } else if (uc[i].unicode() == ShinDot) {
            if (base == 0x05e9)
                shaped = 0xfb2a;
            else if (base == 0xfb49)
                shaped = 0xfb2c;
            else
                invalid = true;
        } else if (uc[i].unicode() == SinDot) {
            if (base == 0x05e9)
                shaped = 0xfb2b;
            else if (base == 0xfb49)
                shaped = 0xfb2d;
            else
                invalid = true;
        } else if (uc[i].unicode() == Patah) {
            if (base == 0x5d0)
                shaped = 0xfb2e;
        } else if (uc[i].unicode() == Qamats) {
            if (base == 0x5d0)
                shaped = 0xfb2f;
        } else if (uc[i].unicode() == Holam) {
            if (base == 0x5d5)
                shaped = 0xfb4b;
        } else if (uc[i].unicode() == Rafe) {
            if (base == 0x5d1)
                shaped = 0xfb4c;
            else if (base == 0x5db)
                shaped = 0xfb4d;
            else if (base == 0x5e4)
                shaped = 0xfb4e;
        }

        if (invalid) {
            shapedChars[slen] = 0x25cc;
            glyphs[slen].attributes.clusterStart = true;
            glyphs[slen].attributes.mark = false;
            glyphs[slen].attributes.combiningClass = 0;
            cluster_start = slen;
            ++slen;
        }
        if (shaped) {
            if (item->font->canRender((QChar *)&shaped, 1)) {
                shapedChars[slen-1] = QChar(shaped);
            } else
                shaped = 0;
        }
        if (!shaped) {
            shapedChars[slen] = uc[i];
            if (QChar::category(uc[i].unicode()) != QChar::Mark_NonSpacing) {
                glyphs[slen].attributes.clusterStart = true;
                glyphs[slen].attributes.mark = false;
                glyphs[slen].attributes.combiningClass = 0;
                glyphs[slen].attributes.dontPrint = qIsControlChar(uc[i].unicode());
                cluster_start = slen;
            } else {
                glyphs[slen].attributes.clusterStart = false;
                glyphs[slen].attributes.mark = true;
                glyphs[slen].attributes.combiningClass = QChar::combiningClass(uc[i].unicode());
            }
            ++slen;
        }
        logClusters[i] = cluster_start;
    }

    if (!item->font->stringToCMap(shapedChars, slen, glyphs, &item->num_glyphs, QFlag(item->flags))) {
        if (item->length > 256)
            ::free(shapedChars);
        return false;
    }
    for (int i = 0; i < item->num_glyphs; ++i) {
        if (glyphs[i].attributes.mark) {
            glyphs[i].advance.x = 0;
        }
    }
    qt_heuristicPosition(item);

    if (item->length > 256)
        ::free(shapedChars);
    return true;
}
#endif

#endif
