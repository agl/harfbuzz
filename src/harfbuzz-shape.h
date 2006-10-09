typedef uint16_t HB_CodePoint;
typedef char HB_Boolean
typedef uint32_t HB_Fixed; /* 26.6 */
typedef uint32_t HB_Glyph;
typedef uint32_t HB_Unichar;

struct HB_Metrics
{
    HB_Fixed advance;
};

/*
 * HB_Font: Abstract font interface.
 *  First pass of this might just have FT_Face *getFace();
 */
struct HB_FontClass {
    HB_Glyph   charToGlyph(HBFont *font, HB_Unichar char);
    void       getMetrics(HBFont *font, HB_Glyph glyph, HB_Metrics *metrics);
    HB_Boolean getSFontTable(HBFont *font, void **cookie, char **start, int *len);
    HB_Boolean freeSFontTable(void **cookie);
};

struct HB_Font {
    HB_FontClass *clazz;
};

/*
 * Language tags, of the form en-us
 */
typedef void *HB_Language;

HB_Language hb_language_from_string(const char *str);
const char *hb_language_to_string(HB_Language language);

/*
 * Input to shaping process
 */
enum HB_RunEdge {
    HB_RUN_EDGE_START_END,
    HB_RUN_EDGE_HYPHEN,
    HB_RUN_EDGE_MIDDLE,
};

/*
 * Just do a diff at the end of one run
 */
enum HB_DiffType {
    HB_DIFF_START = 1 << 0,
    HB_DIFF_END = 1 << 1
};

/* Defines optional informaiton in HB_ShapeInput => binary compat */
enum HB_ShapeFlags {
    HB_SHAPE_START_TYPE = 1 << 0,
    HB_SHAPE_END_TYPE   = 1 << 1,
    HB_SHAPE_DIFF   = 1 << 2,
};

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

HB_AttributeType hb_attribute_type_from_string(const char *str);
const char *hb_attribute_type_to_string(HB_AttributeType attribute_type);

struct HB_ShapeInput {
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
    HB_DiffType diff_type;
};

struct HB_GlyphItem {
    HB_Glyph glyph;
    
    HB_Fixed x_offset;
    HB_Fixed y_offset;
    HB_Fixed advance;

    /* Add kashida information, etc, here */
};

struct HB_GlyphOutput {
    int totalGlyphs;
    int glyph_item_size;
    void *buffer;
    int *log_clusters; /* Uniscribe style */
    int codepoints_shaped; /* When diffing */
};

/* Accessor for a particular glyph;
#define HB_GLYPH_OUTPUT_ITEM(HB_GlyphItem item, int index) ...

/*
 * Main shaping function
 */

enum HB_ShapeResult {
    HB_SHAPE_RESULT_SUCCESS
    HB_SHAPE_RESULT_NOT_ENOUGH_SPACE,
    HB_SHAPE_RESULT_FAILED,
};

HB_ShapeResult hb_shape(HB_ShapeInput *input, HB_ShapeFlags flags, HBGlyphOutput *output);

