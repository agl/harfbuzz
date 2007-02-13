/*******************************************************************
 *
 *  Copyright 2007  Trolltech ASA
 *
 *  This is part of HarfBuzz, an OpenType Layout engine library.
 *
 *  See the file name COPYING for licensing information.
 *
 ******************************************************************/

#include "harfbuzz-shaper.h"
#include "harfbuzz-shaper-private.h"

// -----------------------------------------------------------------------------------------------------
//
// The line break algorithm. See http://www.unicode.org/reports/tr14/tr14-13.html
//
// -----------------------------------------------------------------------------------------------------

/* The Unicode algorithm does in our opinion allow line breaks at some
   places they shouldn't be allowed. The following changes were thus
   made in comparison to the Unicode reference:

   EX->AL from DB to IB
   SY->AL from DB to IB
   SY->PO from DB to IB
   SY->PR from DB to IB
   SY->OP from DB to IB
   AL->PR from DB to IB
   AL->PO from DB to IB
   PR->PR from DB to IB
   PO->PO from DB to IB
   PR->PO from DB to IB
   PO->PR from DB to IB
   HY->PO from DB to IB
   HY->PR from DB to IB
   HY->OP from DB to IB
   NU->EX from PB to IB
   EX->PO from DB to IB
*/

// The following line break classes are not treated by the table:
//  AI, BK, CB, CR, LF, NL, SA, SG, SP, XX

enum break_class {
    // the first 4 values have to agree with the enum in QCharAttributes
    ProhibitedBreak,            // PB in table
    DirectBreak,                // DB in table
    IndirectBreak,              // IB in table
    CombiningIndirectBreak,     // CI in table
    CombiningProhibitedBreak,   // CP in table
};
#define DB DirectBreak
#define IB IndirectBreak
#define CI CombiningIndirectBreak
#define CP CombiningProhibitedBreak
#define PB ProhibitedBreak

static const uint8_t breakTable[HB_LineBreak_JT+1][HB_LineBreak_JT+1] =
{
/*          OP  CL  QU  GL  NS  EX  SY  IS  PR  PO  NU  AL  ID  IN  HY  BA  BB  B2  ZW  CM  WJ  H2  H3  JL  JV  JT */
/* OP */ { PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, CP, PB, PB, PB, PB, PB, PB },
/* CL */ { DB, PB, IB, IB, PB, PB, PB, PB, IB, IB, IB, IB, DB, DB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB },
/* QU */ { PB, PB, IB, IB, IB, PB, PB, PB, IB, IB, IB, IB, IB, IB, IB, IB, IB, IB, PB, CI, PB, IB, IB, IB, IB, IB },
/* GL */ { IB, PB, IB, IB, IB, PB, PB, PB, IB, IB, IB, IB, IB, IB, IB, IB, IB, IB, PB, CI, PB, IB, IB, IB, IB, IB },
/* NS */ { DB, PB, IB, IB, IB, PB, PB, PB, DB, DB, DB, DB, DB, DB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB },
/* EX */ { DB, PB, IB, IB, IB, PB, PB, PB, DB, IB, DB, IB, DB, DB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB },
/* SY */ { IB, PB, IB, IB, IB, PB, PB, PB, IB, IB, IB, IB, DB, DB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB },
/* IS */ { DB, PB, IB, IB, IB, PB, PB, PB, DB, DB, IB, IB, DB, DB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB },
/* PR */ { IB, PB, IB, IB, IB, PB, PB, PB, IB, IB, IB, IB, IB, DB, IB, IB, DB, DB, PB, CI, PB, IB, IB, IB, IB, IB },
/* PO */ { IB, PB, IB, IB, IB, PB, PB, PB, IB, IB, IB, IB, DB, DB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB },
/* NU */ { IB, PB, IB, IB, IB, IB, PB, PB, IB, IB, IB, IB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB },
/* AL */ { IB, PB, IB, IB, IB, PB, PB, PB, IB, IB, IB, IB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB },
/* ID */ { DB, PB, IB, IB, IB, PB, PB, PB, DB, IB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB },
/* IN */ { DB, PB, IB, IB, IB, PB, PB, PB, DB, DB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB },
/* HY */ { IB, PB, IB, IB, IB, PB, PB, PB, IB, IB, IB, DB, DB, DB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB },
/* BA */ { DB, PB, IB, IB, IB, PB, PB, PB, DB, DB, DB, DB, DB, DB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB },
/* BB */ { IB, PB, IB, IB, IB, PB, PB, PB, IB, IB, IB, IB, IB, IB, IB, IB, IB, IB, PB, CI, PB, IB, IB, IB, IB, IB },
/* B2 */ { DB, PB, IB, IB, IB, PB, PB, PB, DB, DB, DB, DB, DB, DB, IB, IB, DB, PB, PB, CI, PB, DB, DB, DB, DB, DB },
/* ZW */ { DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, PB, DB, DB, DB, DB, DB, DB, DB },
/* CM */ { DB, PB, IB, IB, IB, PB, PB, PB, DB, DB, IB, IB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB },
/* WJ */ { IB, PB, IB, IB, IB, PB, PB, PB, IB, IB, IB, IB, IB, IB, IB, IB, IB, IB, PB, CI, PB, IB, IB, IB, IB, IB },
/* H2 */ { DB, PB, IB, IB, IB, PB, PB, PB, DB, IB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, IB, IB },
/* H3 */ { DB, PB, IB, IB, IB, PB, PB, PB, DB, IB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, IB },
/* JL */ { DB, PB, IB, IB, IB, PB, PB, PB, DB, IB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, IB, IB, IB, IB, DB },
/* JV */ { DB, PB, IB, IB, IB, PB, PB, PB, DB, IB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, IB, IB },
/* JT */ { DB, PB, IB, IB, IB, PB, PB, PB, DB, IB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, IB }
};
#undef DB
#undef IB
#undef CI
#undef CP
#undef PB


static void calcLineBreaks(const HB_UChar16 *uc, uint32_t len, HB_CharAttributes *charAttributes)
{
    if (!len)
        return;

    // ##### can this fail if the first char is a surrogate?
    int cls = HB_GetLineBreakClass(*uc);
    // handle case where input starts with an LF
    if (cls == HB_LineBreak_LF)
        cls = HB_LineBreak_BK;

    charAttributes[0].whiteSpace = (cls == HB_LineBreak_SP || cls == HB_LineBreak_BK);
    charAttributes[0].charStop = true;

    int lcls = cls;
    for (int i = 1; i < len; ++i) {
        charAttributes[i].whiteSpace = false;
        charAttributes[i].charStop = true;

        int ncls = HB_GetLineBreakClass(uc[i]);
        // handle surrogates
        if (ncls == HB_LineBreak_SG) {
            if (HB_IsHighSurrogate(uc[i]) && i < len - 1 && HB_IsLowSurrogate(uc[i+1])) {
                continue;
            } else if (HB_IsLowSurrogate(uc[i]) && HB_IsHighSurrogate(uc[i-1])) {
                HB_UChar32 code = HB_SurrogateToUcs4(uc[i-1], uc[i]);
                ncls = HB_GetLineBreakClass(code);
                charAttributes[i].charStop = false;
            } else {
                ncls = HB_LineBreak_AL;
            }
        }

        // set white space and char stop flag
        if (ncls >= HB_LineBreak_SP)
            charAttributes[i].whiteSpace = true;
        if (ncls == HB_LineBreak_CM)
            charAttributes[i].charStop = false;

        HB_LineBreakType lineBreakType = HB_NoBreak;
        if (cls >= HB_LineBreak_LF) {
            lineBreakType = HB_ForcedBreak;
        } else if(cls == HB_LineBreak_CR) {
            lineBreakType = (ncls == HB_LineBreak_LF) ? HB_NoBreak : HB_ForcedBreak;
        }

        if (ncls == HB_LineBreak_SP)
            goto next_no_cls_update;
        if (ncls >= HB_LineBreak_CR)
            goto next;

        // two complex chars (thai or lao), thai_attributes might override, but here we do a best guess
	if (cls == HB_LineBreak_SA && ncls == HB_LineBreak_SA) {
            lineBreakType = HB_Break;
            goto next;
        }

        {
            int tcls = ncls;
            if (tcls >= HB_LineBreak_SA)
                tcls = HB_LineBreak_ID;
            if (cls >= HB_LineBreak_SA)
                cls = HB_LineBreak_ID;

            int brk = breakTable[cls][tcls];
            switch (brk) {
            case DirectBreak:
                lineBreakType = HB_Break;
                if (uc[i-1] == 0xad) // soft hyphen
                    lineBreakType = HB_SoftHyphen;
                break;
            case IndirectBreak:
                lineBreakType = (lcls == HB_LineBreak_SP) ? HB_Break : HB_NoBreak;
                break;
            case CombiningIndirectBreak:
                lineBreakType = HB_NoBreak;
                if (lcls == HB_LineBreak_SP){
                    if (i > 1)
                        charAttributes[i-2].lineBreakType = HB_Break;
                } else {
                    goto next_no_cls_update;
                }
                break;
            case CombiningProhibitedBreak:
                lineBreakType = HB_NoBreak;
                if (lcls != HB_LineBreak_SP)
                    goto next_no_cls_update;
            case ProhibitedBreak:
            default:
                break;
            }
        }
    next:
        cls = ncls;
    next_no_cls_update:
        lcls = ncls;
        charAttributes[i-1].lineBreakType = lineBreakType;
    }
    charAttributes[len-1].lineBreakType = HB_ForcedBreak;
}

#if 0

// --------------------------------------------------------------------------------------------------------------------------------------------
//
// Basic processing
//
// --------------------------------------------------------------------------------------------------------------------------------------------

static inline void positionCluster(QShaperItem *item, int gfrom,  int glast)
{
    int nmarks = glast - gfrom;
    if (nmarks <= 0) {
        qWarning("Qt: No marks to position in positionCluster()");
        return;
    }

    QGlyphLayout *glyphs = item->glyphs;
    QFontEngine *f = item->font;

    glyph_metrics_t baseInfo = f->boundingBox(glyphs[gfrom].glyph);

    if (item->script == QUnicodeTables::Hebrew)
        // we need to attach below the baseline, because of the hebrew iud.
        baseInfo.height = qMax(baseInfo.height, -baseInfo.y);

    QRectF baseRect(baseInfo.x.toReal(), baseInfo.y.toReal(), baseInfo.width.toReal(), baseInfo.height.toReal());

//     qDebug("---> positionCluster: cluster from %d to %d", gfrom, glast);
//     qDebug("baseInfo: %f/%f (%f/%f) off=%f/%f", baseInfo.x, baseInfo.y, baseInfo.width, baseInfo.height, baseInfo.xoff, baseInfo.yoff);

    qreal size = (f->ascent()/10).toReal();
    qreal offsetBase = (size - 4) / 4 + qMin<qreal>(size, 4) + 1;
//     qDebug("offset = %f", offsetBase);

    bool rightToLeft = item->flags & QTextEngine::RightToLeft;

    int i;
    unsigned char lastCmb = 0;
    QRectF attachmentRect;

    for(i = 1; i <= nmarks; i++) {
        glyph_t mark = glyphs[gfrom+i].glyph;
        QPointF p;
        glyph_metrics_t markInfo = f->boundingBox(mark);
        QRectF markRect(markInfo.x.toReal(), markInfo.y.toReal(), markInfo.width.toReal(), markInfo.height.toReal());
//          qDebug("markInfo: %f/%f (%f/%f) off=%f/%f", markInfo.x, markInfo.y, markInfo.width, markInfo.height, markInfo.xoff, markInfo.yoff);

        qreal offset = offsetBase;
        unsigned char cmb = glyphs[gfrom+i].attributes.combiningClass;

        // ### maybe the whole position determination should move down to heuristicSetGlyphAttributes. Would save some
        // bits  in the glyphAttributes structure.
        if (cmb < 200) {
            // fixed position classes. We approximate by mapping to one of the others.
            // currently I added only the ones for arabic, hebrew, lao and thai.

            // for Lao and Thai marks with class 0, see below (heuristicSetGlyphAttributes)

            // add a bit more offset to arabic, a bit hacky
            if (cmb >= 27 && cmb <= 36 && offset < 3)
                offset +=1;
            // below
            if ((cmb >= 10 && cmb <= 18) ||
                 cmb == 20 || cmb == 22 ||
                 cmb == 29 || cmb == 32)
                cmb = QChar::Combining_Below;
            // above
            else if (cmb == 23 || cmb == 27 || cmb == 28 ||
                      cmb == 30 || cmb == 31 || (cmb >= 33 && cmb <= 36))
                cmb = QChar::Combining_Above;
            //below-right
            else if (cmb == 9 || cmb == 103 || cmb == 118)
                cmb = QChar::Combining_BelowRight;
            // above-right
            else if (cmb == 24 || cmb == 107 || cmb == 122)
                cmb = QChar::Combining_AboveRight;
            else if (cmb == 25)
                cmb = QChar::Combining_AboveLeft;
            // fixed:
            //  19 21

        }

        // combining marks of different class don't interact. Reset the rectangle.
        if (cmb != lastCmb) {
            //qDebug("resetting rect");
            attachmentRect = baseRect;
        }

        switch(cmb) {
        case QChar::Combining_DoubleBelow:
                // ### wrong in rtl context!
        case QChar::Combining_BelowLeft:
            p += QPointF(0, offset);
        case QChar::Combining_BelowLeftAttached:
            p += attachmentRect.bottomLeft() - markRect.topLeft();
            break;
        case QChar::Combining_Below:
            p += QPointF(0, offset);
        case QChar::Combining_BelowAttached:
            p += attachmentRect.bottomLeft() - markRect.topLeft();
            p += QPointF((attachmentRect.width() - markRect.width())/2 , 0);
            break;
            case QChar::Combining_BelowRight:
            p += QPointF(0, offset);
        case QChar::Combining_BelowRightAttached:
            p += attachmentRect.bottomRight() - markRect.topRight();
            break;
            case QChar::Combining_Left:
            p += QPointF(-offset, 0);
        case QChar::Combining_LeftAttached:
            break;
            case QChar::Combining_Right:
            p += QPointF(offset, 0);
        case QChar::Combining_RightAttached:
            break;
        case QChar::Combining_DoubleAbove:
            // ### wrong in RTL context!
        case QChar::Combining_AboveLeft:
            p += QPointF(0, -offset);
        case QChar::Combining_AboveLeftAttached:
            p += attachmentRect.topLeft() - markRect.bottomLeft();
            break;
        case QChar::Combining_Above:
            p += QPointF(0, -offset);
        case QChar::Combining_AboveAttached:
            p += attachmentRect.topLeft() - markRect.bottomLeft();
            p += QPointF((attachmentRect.width() - markRect.width())/2 , 0);
            break;
        case QChar::Combining_AboveRight:
            p += QPointF(0, -offset);
        case QChar::Combining_AboveRightAttached:
            p += attachmentRect.topRight() - markRect.bottomRight();
            break;

        case QChar::Combining_IotaSubscript:
            default:
                break;
        }
//          qDebug("char=%x combiningClass = %d offset=%f/%f", mark, cmb, p.x(), p.y());
        markRect.translate(p.x(), p.y());
        attachmentRect |= markRect;
        lastCmb = cmb;
        if (rightToLeft) {
            glyphs[gfrom+i].offset.x = QFixed::fromReal(p.x());
            glyphs[gfrom+i].offset.y = QFixed::fromReal(p.y());
        } else {
            glyphs[gfrom+i].offset.x = QFixed::fromReal(p.x()) - baseInfo.xoff;
            glyphs[gfrom+i].offset.y = QFixed::fromReal(p.y()) - baseInfo.yoff;
        }
        glyphs[gfrom+i].advance = QFixedPoint();
    }
}


void qt_heuristicPosition(QShaperItem *item)
{
    QGlyphLayout *glyphs = item->glyphs;

    int cEnd = -1;
    int i = item->num_glyphs;
    while (i--) {
        if (cEnd == -1 && glyphs[i].attributes.mark) {
            cEnd = i;
        } else if (cEnd != -1 && !glyphs[i].attributes.mark) {
            positionCluster(item, i, cEnd);
            cEnd = -1;
        }
    }
}



// set the glyph attributes heuristically. Assumes a 1 to 1 relationship between chars and glyphs
// and no reordering.
// also computes logClusters heuristically
static void heuristicSetGlyphAttributes(QShaperItem *item, const QChar *uc, int length)
{
    // ### zeroWidth and justification are missing here!!!!!

    Q_ASSERT(item->num_glyphs <= length);

//     qDebug("QScriptEngine::heuristicSetGlyphAttributes, num_glyphs=%d", item->num_glyphs);
    QGlyphLayout *glyphs = item->glyphs;
    unsigned short *logClusters = item->log_clusters;


    // the mac font engine does the setup already in stringToCMap
#ifndef Q_WS_MAC
    int glyph_pos = 0;
    for (int i = 0; i < length; i++) {
        if (uc[i].unicode() >= 0xd800 && uc[i].unicode() < 0xdc00 && i < length-1
            && uc[i+1].unicode() >= 0xdc00 && uc[i+1].unicode() < 0xe000) {
            logClusters[i] = glyph_pos;
            logClusters[++i] = glyph_pos;
        } else {
            logClusters[i] = glyph_pos;
        }
        ++glyph_pos;
    }
    Q_ASSERT(glyph_pos == item->num_glyphs);
#endif

    // first char in a run is never (treated as) a mark
#if !defined(Q_WS_MAC)
    int cStart = 0;
#endif
    const bool symbolFont = item->font->symbol;
    glyphs[0].attributes.mark = false;
    glyphs[0].attributes.clusterStart = true;
    glyphs[0].attributes.dontPrint = (!symbolFont && uc[0].unicode() == 0x00ad) || qIsControlChar(uc[0].unicode());

    int pos = 0;
    int lastCat = QChar::category(uc[0].unicode());
    for (int i = 1; i < length; ++i) {
        if (logClusters[i] == pos)
            // same glyph
            continue;
        ++pos;
        while (pos < logClusters[i]) {
            // the mac engine already has attributes setup properly
#if !defined(Q_WS_MAC)
            glyphs[pos].attributes = glyphs[pos-1].attributes;
#endif
            ++pos;
        }
        // hide soft-hyphens by default
        if ((!symbolFont && uc[i].unicode() == 0x00ad) || qIsControlChar(uc[i].unicode()))
            glyphs[pos].attributes.dontPrint = true;
        const QUnicodeTables::Properties *prop = QUnicodeTables::properties(uc[i].unicode());
        int cat = prop->category;
#if !defined(Q_WS_MAC)
        if (cat != QChar::Mark_NonSpacing) {
            glyphs[pos].attributes.mark = false;
            glyphs[pos].attributes.clusterStart = true;
            glyphs[pos].attributes.combiningClass = 0;
            cStart = logClusters[i];
        } else {
            int cmb = prop->combiningClass;

            if (cmb == 0) {
                // Fix 0 combining classes
                if ((uc[pos].unicode() & 0xff00) == 0x0e00) {
                    // thai or lao
                    unsigned char col = uc[pos].cell();
                    if (col == 0x31 ||
                         col == 0x34 ||
                         col == 0x35 ||
                         col == 0x36 ||
                         col == 0x37 ||
                         col == 0x47 ||
                         col == 0x4c ||
                         col == 0x4d ||
                         col == 0x4e) {
                        cmb = QChar::Combining_AboveRight;
                    } else if (col == 0xb1 ||
                                col == 0xb4 ||
                                col == 0xb5 ||
                                col == 0xb6 ||
                                col == 0xb7 ||
                                col == 0xbb ||
                                col == 0xcc ||
                                col == 0xcd) {
                        cmb = QChar::Combining_Above;
                    } else if (col == 0xbc) {
                        cmb = QChar::Combining_Below;
                    }
                }
            }

            glyphs[pos].attributes.mark = true;
            glyphs[pos].attributes.clusterStart = false;
            glyphs[pos].attributes.combiningClass = cmb;
            logClusters[i] = cStart;
            glyphs[pos].advance = QFixedPoint();
        }
#endif
        // one gets an inter character justification point if the current char is not a non spacing mark.
        // as then the current char belongs to the last one and one gets a space justification point
        // after the space char.
        if (lastCat == QChar::Separator_Space)
            glyphs[pos-1].attributes.justification = QGlyphLayout::Space;
        else if (cat != QChar::Mark_NonSpacing)
            glyphs[pos-1].attributes.justification = QGlyphLayout::Character;
        else
            glyphs[pos-1].attributes.justification = QGlyphLayout::NoJustification;

        lastCat = cat;
    }
    pos = logClusters[length-1];
    if (lastCat == QChar::Separator_Space)
        glyphs[pos].attributes.justification = QGlyphLayout::Space;
    else
        glyphs[pos].attributes.justification = QGlyphLayout::Character;
}

static void heuristicSetGlyphAttributes(QShaperItem *item)
{
    heuristicSetGlyphAttributes(item, item->string->unicode() + item->from, item->length);
}

enum {
    CcmpProperty = 0x1
};

#ifndef QT_NO_OPENTYPE
static const QOpenType::Features basic_features[] = {
    { FT_MAKE_TAG('c', 'c', 'm', 'p'), CcmpProperty },
    { FT_MAKE_TAG('l', 'i', 'g', 'a'), CcmpProperty },
    { FT_MAKE_TAG('c', 'l', 'i', 'g'), CcmpProperty },
    {0, 0}
};
#endif

static bool basic_shape(QShaperItem *item)
{

#ifndef QT_NO_OPENTYPE
    const int availableGlyphs = item->num_glyphs;
#endif

    if (!item->font->stringToCMap(item->string->unicode()+item->from, item->length,
                                  item->glyphs, &item->num_glyphs, QFlag(item->flags)))
        return false;
    heuristicSetGlyphAttributes(item);

#ifndef QT_NO_OPENTYPE
    QOpenType *openType = item->font->openType();
    if (!openType && item->font->type() == QFontEngine::Multi) {
        openType = static_cast<QFontEngineMulti *>(item->font)->engine(0)->openType();
        if (openType) {
            for (int i = 0; i < item->num_glyphs; ++i) {
                if (item->glyphs[i].glyph & 0xff000000) {
                    openType = 0;
                    break;
                }
            }
        }
    }
    if (openType && openType->supportsScript(item->script)) {
        openType->selectScript(item, item->script, basic_features);

        openType->shape(item);
        return openType->positionAndAdd(item, availableGlyphs);
    }

#endif

    qt_heuristicPosition(item);
    return true;
}
#endif

static HB_Bool basic_shape() {}
static HB_Bool hebrew_shape() {}
static HB_Bool arabic_shape() {}
static HB_Bool syriac_shape() {}
static HB_Bool thaana_shape() {}
static HB_Bool indic_shape() {}
static HB_Bool myanmar_shape() {}
static HB_Bool hangul_shape() {}
static HB_Bool khmer_shape() {}
HB_Bool HB_TibetanShape() {}

static HB_AttributeFunction thai_attributes = 0;

const HB_ScriptEngine hb_scriptEngines[] = {
    // Common
    { basic_shape, 0},
    // Greek
    { basic_shape, 0},
    // Cyrillic
    { basic_shape, 0},
    // Armenian
    { basic_shape, 0},
    // Hebrew
    { hebrew_shape, 0 },
    // Arabic
    { arabic_shape, 0},
    // Syriac
    { syriac_shape, 0},
    // Thaana
    { thaana_shape, 0 },
    // Devanagari
    { indic_shape, HB_IndicAttributes },
    // Bengali
    { indic_shape, HB_IndicAttributes },
    // Gurmukhi
    { indic_shape, HB_IndicAttributes },
    // Gujarati
    { indic_shape, HB_IndicAttributes },
    // Oriya
    { indic_shape, HB_IndicAttributes },
    // Tamil
    { indic_shape, HB_IndicAttributes },
    // Telugu
    { indic_shape, HB_IndicAttributes },
    // Kannada
    { indic_shape, HB_IndicAttributes },
    // Malayalam
    { indic_shape, HB_IndicAttributes },
    // Sinhala
    { indic_shape, HB_IndicAttributes },
    // Thai
    { basic_shape, thai_attributes },
    // Lao
    { basic_shape, 0 },
    // Tibetan
    { HB_TibetanShape, HB_TibetanAttributes },
    // Myanmar
    { myanmar_shape, HB_MyanmarAttributes },
    // Georgian
    { basic_shape, 0 },
    // Hangul
    { hangul_shape, 0 },
    // Ogham
    { basic_shape, 0 },
    // Runic
    { basic_shape, 0 },
    // Khmer
    { khmer_shape, HB_KhmerAttributes }
};

void HB_GetCharAttributes(const HB_UChar16 *string, uint32_t stringLength,
                          const HB_ScriptItem *items, uint32_t numItems,
                          HB_CharAttributes *attributes)
{
    calcLineBreaks(string, stringLength, attributes);

    int lastPos = stringLength;
    for (int i = numItems - 1; i >= 0; --i) {
        HB_Script script = items[i].script;
        if (script == HB_Script_Inherited)
            script = HB_Script_Common;
        HB_AttributeFunction attributeFunction = hb_scriptEngines[script].charAttributes;
        if (!attributes)
            continue;
        int len = lastPos - items[i].pos;
        attributeFunction(script, string, items[i].pos, len, attributes);
        lastPos = items[i].pos;
    }
}


