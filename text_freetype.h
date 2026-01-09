/**
 * FreeType Text Renderer for WeAct Display
 * Supports TrueType fonts with full Unicode (including Cyrillic)
 */

#ifndef TEXT_FREETYPE_H
#define TEXT_FREETYPE_H

#include "weact_display.h"
#include <stdbool.h>

/* Font size presets (can use any size 6-32) */
#define FT_FONT_SIZE_SMALL  8
#define FT_FONT_SIZE_MEDIUM 12
#define FT_FONT_SIZE_LARGE  16

/* Text alignment */
typedef enum {
    FT_TEXT_ALIGN_LEFT = 0,
    FT_TEXT_ALIGN_CENTER,
    FT_TEXT_ALIGN_RIGHT
} ft_text_align_t;

/* FreeType text context */
typedef struct ft_text_context ft_text_context_t;

/**
 * Initialize FreeType text renderer
 * @param display WeAct display handle
 * @param font_path Path to TTF font file (NULL for default DejaVu Sans)
 * @param font_size Font size in points (6-32)
 * @return Text context or NULL on error
 */
ft_text_context_t* ft_text_init(weact_display_t *display, 
                                 const char *font_path, 
                                 int font_size);

/**
 * Cleanup FreeType text renderer
 */
void ft_text_cleanup(ft_text_context_t *ctx);

/**
 * Set font size
 */
bool ft_text_set_size(ft_text_context_t *ctx, int size);

/**
 * Set text color
 */
void ft_text_set_color(ft_text_context_t *ctx, uint16_t color);

/**
 * Set text alignment
 */
void ft_text_set_alignment(ft_text_context_t *ctx, ft_text_align_t align);

/**
 * Get text width in pixels (supports UTF-8)
 */
int ft_text_get_width(ft_text_context_t *ctx, const char *text);

/**
 * Get text height in pixels
 */
int ft_text_get_height(ft_text_context_t *ctx);

/**
 * Draw text at position (supports UTF-8 including Cyrillic)
 * @param ctx Text context
 * @param x X coordinate
 * @param y Y coordinate (baseline)
 * @param text UTF-8 text string
 */
void ft_text_draw(ft_text_context_t *ctx, int x, int y, const char *text);

/**
 * Draw text with word wrapping
 * @param ctx Text context
 * @param x X coordinate
 * @param y Y coordinate
 * @param max_width Maximum width in pixels
 * @param max_height Maximum height in pixels
 * @param text UTF-8 text string
 */
void ft_text_draw_wrapped(ft_text_context_t *ctx, int x, int y,
                           int max_width, int max_height, const char *text);

/**
 * Check if FreeType is available
 * @return true if FreeType library is available
 */
bool ft_text_available(void);

/**
 * Get default font path
 * @return Path to DejaVu Sans font
 */
const char* ft_text_get_default_font(void);

#endif /* TEXT_FREETYPE_H */
