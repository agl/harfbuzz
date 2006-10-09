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
    HB_RUN_EDGE_LINE_VISUAL_MIDDLE  = 1 << 1,
    HB_RUN_EDGE_LINE_LOGICAL_EDGE   = 1 << 2,
    HB_RUN_EDGE_LINE_LOGICAL_MIDDLE = 1 << 3,
    HB_RUN_EDGE_LINE_ADD_HYPHEN     = 1 << 4  /* ???? */
};

/* Defines optional informaiton in HB_ShapeInput => binary compat */
enum HB_ShapeFlags {
    HB_SHAPE_START_TYPE = 1 << 0,
    HB_SHAPE_END_TYPE   = 1 << 1
};

/* Attributes types are described by "interned strings"; this is a little
 * annoying if you want to right a switch statement, but keeps things
 * simple.
 */
typedef struct HB_AttributeType_ HB_AttributeType;

HB_AttributeType hb_attribute_type_from_string(const char *str);
const char *hb_attribute_type_to_string(HB_AttributeType attribute_type);

struct HB_Attribute {
    HB_AttributeType type;
    int start; 
    int end;
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

    HB_Gravity gravity;
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

struct HB_GlyphOutput {
    int glyph_item_size;
    int buffer_space;
    int total_glyphs;
    int *log_clusters; /* Uniscribe style */
  
    void *buffer;
};

/* Accessor for a particular glyph, really only for internal use */
#define HB_GLYPH_OUTPUT_ITEM(HB_GlyphOutput output, int index) ..

/*
 * Main shaping function
 */
enum HB_ShapeResult {
    HB_SHAPE_RESULT_SUCCESS,
    HB_SHAPE_RESULT_NOT_ENOUGH_SPACE, /* HB_GlyphOutput.total_glyphs is updated */
    HB_SHAPE_RESULT_FAILED,
};

HB_ShapeResult hb_shape(HB_ShapeInput *input, HBGlyphOutput *output);
