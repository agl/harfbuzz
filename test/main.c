#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdio.h>
#include <harfbuzz-shape.h>

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

int main()
{
    FT_Face face;
    face = setup ();

    FT_Done_Face (face);
    FT_Done_FreeType (freetype);
    FcFini ();
    return 0;
}
