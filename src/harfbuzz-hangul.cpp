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

// Hangul is a syllable based script. Unicode reserves a large range
// for precomposed hangul, where syllables are already precomposed to
// their final glyph shape. In addition, a so called jamo range is
// defined, that can be used to express old Hangul. Modern hangul
// syllables can also be expressed as jamo, and should be composed
// into syllables. The operation is rather simple and mathematical.

// Every hangul jamo is classified as being either a Leading consonant
// (L), and intermediat Vowel (V) or a trailing consonant (T). Modern
// hangul syllables (the ones in the precomposed area can be of type
// LV or LVT.
//
// Syllable breaks do _not_ occur between:
//
// L              L, V or precomposed
// V, LV          V, T
// LVT, T         T
//
// A standard syllable is of the form L+V+T*. The above rules allow
// nonstandard syllables L*V*T*. To transform them into standard
// syllables fill characters L_f and V_f can be inserted.

enum {
    Hangul_SBase = 0xac00,
    Hangul_LBase = 0x1100,
    Hangul_VBase = 0x1161,
    Hangul_TBase = 0x11a7,
    Hangul_SCount = 11172,
    Hangul_LCount = 19,
    Hangul_VCount = 21,
    Hangul_TCount = 28,
    Hangul_NCount = 21*28
};

static inline bool hangul_isPrecomposed(unsigned short uc) {
    return (uc >= Hangul_SBase && uc < Hangul_SBase + Hangul_SCount);
}

static inline bool hangul_isLV(unsigned short uc) {
    return ((uc - Hangul_SBase) % Hangul_TCount == 0);
}

enum HangulType {
    L,
    V,
    T,
    LV,
    LVT,
    X
};

static inline HangulType hangul_type(unsigned short uc) {
    if (uc > Hangul_SBase && uc < Hangul_SBase + Hangul_SCount)
        return hangul_isLV(uc) ? LV : LVT;
    if (uc < Hangul_LBase || uc > 0x11ff)
        return X;
    if (uc < Hangul_VBase)
        return L;
    if (uc < Hangul_TBase)
        return V;
    return T;
}

static int hangul_nextSyllableBoundary(const QString &s, int start, int end)
{
    const QChar *uc = s.unicode() + start;

    HangulType state = hangul_type(uc->unicode());
    int pos = 1;

    while (pos < end - start) {
        HangulType newState = hangul_type(uc[pos].unicode());
        switch(newState) {
        case X:
            goto finish;
        case L:
        case V:
        case T:
            if (state > newState)
                goto finish;
            state = newState;
            break;
        case LV:
            if (state > L)
                goto finish;
            state = V;
            break;
        case LVT:
            if (state > L)
                goto finish;
            state = T;
        }
        ++pos;
    }

 finish:
    return start+pos;
}

#ifndef QT_NO_OPENTYPE
static const QOpenType::Features hangul_features [] = {
    { FT_MAKE_TAG('c', 'c', 'm', 'p'), CcmpProperty },
    { FT_MAKE_TAG('l', 'j', 'm', 'o'), CcmpProperty },
    { FT_MAKE_TAG('j', 'j', 'm', 'o'), CcmpProperty },
    { FT_MAKE_TAG('t', 'j', 'm', 'o'), CcmpProperty },
    { 0, 0 }
};
#endif

#if defined(Q_WS_X11) || defined(Q_WS_QWS)
static bool hangul_shape_syllable(QOpenType *openType, QShaperItem *item)
{
    Q_UNUSED(openType)
    const QChar *ch = item->string->unicode() + item->from;

    int i;
    unsigned short composed = 0;
    // see if we can compose the syllable into a modern hangul
    if (item->length == 2) {
        int LIndex = ch[0].unicode() - Hangul_LBase;
        int VIndex = ch[1].unicode() - Hangul_VBase;
        if (LIndex >= 0 && LIndex < Hangul_LCount &&
            VIndex >= 0 && VIndex < Hangul_VCount)
            composed = (LIndex * Hangul_VCount + VIndex) * Hangul_TCount + Hangul_SBase;
    } else if (item->length == 3) {
        int LIndex = ch[0].unicode() - Hangul_LBase;
        int VIndex = ch[1].unicode() - Hangul_VBase;
        int TIndex = ch[2].unicode() - Hangul_TBase;
        if (LIndex >= 0 && LIndex < Hangul_LCount &&
            VIndex >= 0 && VIndex < Hangul_VCount &&
            TIndex >= 0 && TIndex < Hangul_TCount)
            composed = (LIndex * Hangul_VCount + VIndex) * Hangul_TCount + TIndex + Hangul_SBase;
    }


    int len = item->length;
    QChar c(composed);

    // if we have a modern hangul use the composed form
    if (composed) {
        // chars = &c;
        len = 1;
    }

#ifndef QT_NO_OPENTYPE
    const int availableGlyphs = item->num_glyphs;
#endif
    if (!item->font->stringToCMap(ch, len, item->glyphs, &item->num_glyphs, QFlag(item->flags)))
        return false;
    for (i = 0; i < len; i++) {
        item->glyphs[i].attributes.mark = false;
        item->glyphs[i].attributes.clusterStart = false;
        item->glyphs[i].attributes.justification = 0;
        item->glyphs[i].attributes.zeroWidth = false;
        IDEBUG("    %d: %4x", i, ch[i].unicode());
    }

#ifndef QT_NO_OPENTYPE
    if (openType && !composed) {

        QVarLengthArray<unsigned short> logClusters(len);
        for (i = 0; i < len; ++i)
            logClusters[i] = i;
        item->log_clusters = logClusters.data();

        openType->shape(item);
        if (!openType->positionAndAdd(item, availableGlyphs, false))
            return false;

    }
#endif

    item->glyphs[0].attributes.clusterStart = true;
    return true;
}

static bool hangul_shape(QShaperItem *item)
{
    Q_ASSERT(item->script == QUnicodeTables::Hangul);

    const QChar *uc = item->string->unicode() + item->from;

    bool allPrecomposed = true;
    for (int i = 0; i < item->length; ++i) {
        if (!hangul_isPrecomposed(uc[i].unicode())) {
            allPrecomposed = false;
            break;
        }
    }

    if (!allPrecomposed) {
#ifndef QT_NO_OPENTYPE
        QOpenType *openType = item->font->openType();
        if (openType && !openType->supportsScript(item->script))
            openType = 0;
        if (openType)
            openType->selectScript(item, QUnicodeTables::Hangul, hangul_features);
#else
        QOpenType *openType = 0;
#endif

        unsigned short *logClusters = item->log_clusters;

        QShaperItem syllable = *item;
        int first_glyph = 0;

        int sstart = item->from;
        int end = sstart + item->length;
        while (sstart < end) {
            int send = hangul_nextSyllableBoundary(*(item->string), sstart, end);

            syllable.from = sstart;
            syllable.length = send-sstart;
            syllable.glyphs = item->glyphs + first_glyph;
            syllable.num_glyphs = item->num_glyphs - first_glyph;
            if (!hangul_shape_syllable(openType, &syllable)) {
                item->num_glyphs += syllable.num_glyphs;
                return false;
            }
            // fix logcluster array
            for (int i = sstart; i < send; ++i)
                logClusters[i-item->from] = first_glyph;
            sstart = send;
            first_glyph += syllable.num_glyphs;
        }
        item->num_glyphs = first_glyph;
        return true;
    }

    return basic_shape(item);
}
#endif

#endif

