/* Base Types */

typedef uint16_t HB_CodePoint; /* UTF-16 codepoint (not character ) */
typedef char HB_Boolean
typedef uint32_t HB_Fixed; /* 26.6 */
typedef uint32_t HB_Glyph;
typedef uint32_t HB_Unichar;

/* Metrics reported by the font backend for use of the shaper */
struct HB_GlyphMetrics
{
    HB_Fixed advance;
    
    /* Do we need ink/logical extents for the glyph here? */
};

/*
 * HB_Font: Abstract font interface.
 *  First pass of this might just have FT_Face *getFace();
 */
struct HB_FontClass {
    HB_Glyph   charToGlyph(HBFont *font, HB_Unichar char);
    void       getMetrics(HBFont *font, HB_Glyph glyph, HB_GlyphMetrics *metrics);
    HB_Boolean getSFontTable(HBFont *font, void **cookie, char **start, int *len);
    HB_Boolean freeSFontTable(void **cookie);
};

struct HB_Font {
    HB_FontClass *clazz;
};

/*
 * Language tags, of the form en-us; represented as interned, canonicalized
 * strings. hb_language_from_string("en_US"), hb_language_from_string("en-us")
 * both return the same (pointer-comparable) HB_Language).
 */
typedef struct HB_Language_ *HB_Language;

HB_Language hb_language_from_string(const char *str);
const char *hb_language_to_string(HB_Language language);

/* Special treatment for the edges of runs.
 */
enum HB_RunEdge {
    HB_RUN_EDGE_LINE_VISUAL_EDGE    = 1 << 0,
    HB_RUN_EDGE_LINE_LOGICAL_EDGE   = 1 << 1,
    HB_RUN_EDGE_LINE_ADD_HYPHEN     = 1 << 2  /* ???? */
};

/* Defines optional informaiton in HB_ShapeInput; this allows extension
 * of HB_ShapeInput while keeping binary compatibility
 */
enum HB_ShapeFlags {
    HB_SHAPE_START_TYPE = 1 << 0,
    HB_SHAPE_END_TYPE   = 1 << 1
};

/* Attributes types are described by "interned strings"; this is a little
 * annoying if you want to write a switch statement, but keeps things
 * simple.
 */
typedef struct HB_AttributeType_ *HB_AttributeType;

HB_AttributeType hb_attribute_type_from_string(const char *str);
const char *hb_attribute_type_to_string(HB_AttributeType attribute_type);

struct HB_Attribute {
    HB_AttributeType type;
    int start; 
    int end;
};


/**
 * You could handle this like HB_Language, but an enum seems a little nicer;
 * another approach would be to use OpenType script tags.
 */
enum HB_Script {
    HB_SCRIPT_LATIN
    /* ... */
};

/* This is just the subset of direction information needed by the shaper */
enum HB_Direction {
    HB_DIRECTION_LTR,
    HB_DIRECTION_RTL,
    HB_DIRECTION_TTB
};

struct HB_ShapeInput {
    /* Defines what fields the caller has initialized - fields not in
     * the enum are mandatory.
     */
    HB_ShapeFlags flags;
    
    HB_CodePoint *text;
    int length;       /* total length of text to shape */
    int shape_offset; /* start of section to shape */
    int shape_length; /* number of code points to shape */

    HB_Direction direction;
    HB_Script script;
    HB_Language language;

    HB_Attribute *attributes;
    int n_attributes;

    HB_RunEdge start_type;
    HB_RunEdge end_type;
};

struct HB_GlyphItem {
    HB_Glyph glyph;
    
    HB_Fixed x_offset;
    HB_Fixed y_offset;
    HB_Fixed advance;

    /* Add kashida information, etc, here */
};

enum HB_Result {
    HB_RESULT_SUCCESS,
    HB_RESULT_NO_MEMORY,
    HB_SHAPE_RESULT_FAILED
};

/*
 * Buffer for output 
 */
struct HB_GlyphBuffer {
    int glyph_item_size;
    int total_glyphs;
    
    int *log_clusters; /* Uniscribe style */
    int cluster_space;
  
    int glyph_space;
    void *glyph_buffer;
};

/* Making this self-allocating simplifies writing shapers and
 * also keeps things easier for caller. item_size passed in
 * must be at least sizeof(HB_GlyphItem) but can be bigger,
 * to accomodate application structures that extend HB_GlyphItem.
 * The allocated items will be zero-initialized.
 *
 * (Hack: Harfbuzz could choose to use even a *bigger* item size
 * and stick internal information before the public item structure.
 * This hack could possibly be used to unify this with HB_Buffer)
 */
HB_GlyphBuffer *hb_glyph_buffer_new             (size_t item_size);
void            hb_glyph_buffer_clear           ();
HB_Result       hb_glyph_buffer_extend_glyphs   (int    n_items);
HB_Result       hb_glyph_buffer_extend_clusters (int    n_clusters);
void            hb_glyph_buffer_free            (void);


/* Accessor for a particular glyph */
#define HB_GLYPH_BUFFER_ITEM(HB_GlyphBuffer buffer, int index) ..

/*
 * Main shaping function
 */
HB_ShapeResult hb_shape(HB_ShapeInput *input, HB_GlyphBuffer *output);
