#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdio.h>
#include <harfbuzz-shape.h>
#include <harfbuzz.h>

static FT_Library freetype;

static FT_Face
setup()
{
    FcPattern *pattern;
    FcPattern *match;
    FcFontSet *fontset;
    FcResult  result;
    FcChar8   *file;
    int       index;
    FT_Face   face;

    FT_Init_FreeType (&freetype);
    FcInit ();

    pattern = FcNameParse ("DejaVu Sans");
    FcConfigSubstitute (0, pattern, FcMatchPattern);
    FcDefaultSubstitute (pattern);

    match = FcFontMatch (0, pattern, &result);
    if (!match) {
        fprintf(stderr, "Cannot find DejaVu Sans\n");
        exit(1);
    }

    fontset = FcFontSetCreate ();
    FcFontSetAdd (fontset, match);

    if (fontset->nfont < 1) {
        fprintf(stderr, "Fontset is empty?!\n");
        exit(1);
    }

    if (FcPatternGetString (fontset->fonts[0], FC_FILE, 0, &file) != FcResultMatch) {
        fprintf(stderr, "Cannot get font filename\n");
        exit(1);
    }

    if (FcPatternGetInteger (fontset->fonts[0], FC_INDEX, 0, &index) != FcResultMatch) {
        fprintf(stderr, "Cannot get index in font\n");
        exit(1);
    }

    if (FT_New_Face (freetype, file, index, &face)) {
        fprintf(stderr, "Cannot open font\n");
        exit(1);
    }

    FcPatternDestroy (pattern);
    FcFontSetDestroy (fontset);

    return face;
}

typedef enum {
    CcmpProperty = 0x1,
    InitProperty = 0x2,
    IsolProperty = 0x4,
    FinaProperty = 0x8,
    MediProperty = 0x10,
    RligProperty = 0x20,
    CaltProperty = 0x40,
    LigaProperty = 0x80,
    DligProperty = 0x100,
    CswhProperty = 0x200,
    MsetProperty = 0x400
} ArabicProperty;

static struct {
    FT_UInt tag;
    FT_UInt property;
} arabicGSubFeatures[] = {
    { FT_MAKE_TAG('c', 'c', 'm', 'p'), CcmpProperty },
    { FT_MAKE_TAG('i', 's', 'o', 'l'), IsolProperty },
    { FT_MAKE_TAG('f', 'i', 'n', 'a'), FinaProperty },
    { FT_MAKE_TAG('m', 'e', 'd', 'i'), MediProperty },
    { FT_MAKE_TAG('i', 'n', 'i', 't'), InitProperty },
    { FT_MAKE_TAG('r', 'l', 'i', 'g'), RligProperty },
    { FT_MAKE_TAG('c', 'a', 'l', 't'), CaltProperty },
    { FT_MAKE_TAG('l', 'i', 'g', 'a'), LigaProperty },
    { FT_MAKE_TAG('d', 'l', 'i', 'g'), DligProperty },
    { FT_MAKE_TAG('c', 's', 'w', 'h'), CswhProperty }
};
static int numArabicGSubFeatures = sizeof(arabicGSubFeatures) / sizeof(arabicGSubFeatures[0]);

static void
shapetest(FT_Face face)
{
    HB_Buffer buffer;
    HB_GDEF gdef = 0;
    HB_GSUB gsub = 0;
    FT_UShort scriptIndex = 0;
    FT_UShort featureIndex = 0;
    FT_UInt error = 0;
    int i;

    HB_Load_GDEF_Table (face, &gdef);
    HB_Load_GSUB_Table (face, &gsub, gdef);

    if (HB_GSUB_Select_Script (gsub, FT_MAKE_TAG ('a', 'r', 'a', 'b'), &scriptIndex)) {
        fprintf (stderr, "could not select script\n");
        return;
    }

    for (i = 0; i < numArabicGSubFeatures; ++i) {
        if (!HB_GSUB_Select_Feature (gsub, arabicGSubFeatures[i].tag, scriptIndex, 0xffff, &featureIndex)) {
            HB_GSUB_Add_Feature (gsub, featureIndex, arabicGSubFeatures[i].property);
            /*
            printf ("selected %c%c%c%c\n",
                    (arabicGSubFeatures[i].tag >> 24) & 0xff,
                    (arabicGSubFeatures[i].tag >> 16) & 0xff,
                    (arabicGSubFeatures[i].tag >> 8) & 0xff,
                    arabicGSubFeatures[i].tag & 0xff);
                    */
        }
    }

    hb_buffer_new (face->memory, &buffer);

    /* lam */
    if (hb_buffer_add_glyph (buffer,
                             FT_Get_Char_Index(face, 0x0644),
                             /*properties*/ IsolProperty|MediProperty|FinaProperty,
                             /*cluster*/ 0)) {
        fprintf (stderr, "add_glyph failed?!\n");
        return;
    }

    /* alef */
    if (hb_buffer_add_glyph (buffer,
                             FT_Get_Char_Index(face, 0x0627),
                             /*properties*/ IsolProperty|MediProperty|InitProperty,
                             /*cluster*/ 1)) {
        fprintf (stderr, "add_glyph[2] failed?!\n");
        return;
    }

    printf ("input:\n");
    for (i = 0; i < buffer->in_length; ++i)
        printf ("i %d glyph %4x\n", i, buffer->in_string[i].gindex);

    error = HB_GSUB_Apply_String (gsub, buffer);
    if (error && error != HB_Err_Not_Covered) {
        fprintf(stderr, "harfbuzz gsub error %x\n", error);
        return;
    }

    printf ("output:\n");
    for (i = 0; i < buffer->in_length; ++i)
        printf ("i %d glyph %4x\n", i, buffer->in_string[i].gindex);

    if (gsub)
        HB_Done_GSUB_Table (gsub);
    if (gdef)
        HB_Done_GDEF_Table (gdef);
    hb_buffer_free(buffer);
}

int
main()
{
    FT_Face face;
    face = setup ();

    shapetest (face);

    FT_Done_Face (face);
    FT_Done_FreeType (freetype);
    FcFini ();
    return 0;
}
