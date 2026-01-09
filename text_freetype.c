/**
 * FreeType Text Renderer Implementation
 * Supports TrueType fonts with full Unicode (including Cyrillic)
 */

#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "text_freetype.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Default font paths to try */
static const char *default_font_paths[] = {
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
    "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
    "/usr/share/fonts/TTF/DejaVuSans.ttf",  /* Arch Linux */
    "/System/Library/Fonts/Helvetica.ttc",   /* macOS */
    NULL
};

/* FreeType context structure */
struct ft_text_context {
    weact_display_t *display;
    FT_Library library;
    FT_Face face;
    int font_size;
    uint16_t color;
    ft_text_align_t align;
};

/**
 * UTF-8 helper: decode next character
 * Returns character code and advances pointer
 */
static uint32_t utf8_decode(const unsigned char **str) {
    const unsigned char *s = *str;
    uint32_t c = *s++;
    
    if (c < 0x80) {
        /* ASCII */
        *str = s;
        return c;
    } else if ((c & 0xE0) == 0xC0) {
        /* 2-byte sequence */
        uint32_t c2 = *s++;
        *str = s;
        return ((c & 0x1F) << 6) | (c2 & 0x3F);
    } else if ((c & 0xF0) == 0xE0) {
        /* 3-byte sequence */
        uint32_t c2 = *s++;
        uint32_t c3 = *s++;
        *str = s;
        return ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
    } else if ((c & 0xF8) == 0xF0) {
        /* 4-byte sequence */
        uint32_t c2 = *s++;
        uint32_t c3 = *s++;
        uint32_t c4 = *s++;
        *str = s;
        return ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | 
               ((c3 & 0x3F) << 6) | (c4 & 0x3F);
    }
    
    /* Invalid UTF-8 */
    *str = s;
    return 0xFFFD;  /* Replacement character */
}

/**
 * Check if FreeType is available
 */
bool ft_text_available(void) {
    FT_Library library;
    if (FT_Init_FreeType(&library) == 0) {
        FT_Done_FreeType(library);
        return true;
    }
    return false;
}

/**
 * Get default font path
 */
const char* ft_text_get_default_font(void) {
    for (int i = 0; default_font_paths[i] != NULL; i++) {
        FILE *f = fopen(default_font_paths[i], "r");
        if (f) {
            fclose(f);
            return default_font_paths[i];
        }
    }
    return NULL;
}

/**
 * Initialize FreeType text renderer
 */
ft_text_context_t* ft_text_init(weact_display_t *display, 
                                 const char *font_path, 
                                 int font_size) {
    if (!display) return NULL;
    
    /* Allocate context */
    ft_text_context_t *ctx = calloc(1, sizeof(ft_text_context_t));
    if (!ctx) return NULL;
    
    ctx->display = display;
    ctx->font_size = font_size;
    ctx->color = 0xFFFF;  /* White */
    ctx->align = FT_TEXT_ALIGN_LEFT;
    
    /* Initialize FreeType */
    if (FT_Init_FreeType(&ctx->library)) {
        fprintf(stderr, "FreeType: Failed to initialize library\n");
        free(ctx);
        return NULL;
    }
    
    /* Determine font path */
    const char *actual_font_path = font_path;
    if (!actual_font_path) {
        actual_font_path = ft_text_get_default_font();
        if (!actual_font_path) {
            fprintf(stderr, "FreeType: No suitable font found\n");
            FT_Done_FreeType(ctx->library);
            free(ctx);
            return NULL;
        }
    }
    
    /* Load font */
    if (FT_New_Face(ctx->library, actual_font_path, 0, &ctx->face)) {
        fprintf(stderr, "FreeType: Failed to load font: %s\n", actual_font_path);
        FT_Done_FreeType(ctx->library);
        free(ctx);
        return NULL;
    }
    
    /* Set pixel size */
    if (FT_Set_Pixel_Sizes(ctx->face, 0, font_size)) {
        fprintf(stderr, "FreeType: Failed to set font size: %d\n", font_size);
        FT_Done_Face(ctx->face);
        FT_Done_FreeType(ctx->library);
        free(ctx);
        return NULL;
    }
    
    return ctx;
}

/**
 * Cleanup FreeType
 */
void ft_text_cleanup(ft_text_context_t *ctx) {
    if (ctx) {
        if (ctx->face) FT_Done_Face(ctx->face);
        if (ctx->library) FT_Done_FreeType(ctx->library);
        free(ctx);
    }
}

/**
 * Set font size
 */
bool ft_text_set_size(ft_text_context_t *ctx, int size) {
    if (!ctx || !ctx->face) return false;
    
    if (FT_Set_Pixel_Sizes(ctx->face, 0, size)) {
        return false;
    }
    
    ctx->font_size = size;
    return true;
}

/**
 * Set text color
 */
void ft_text_set_color(ft_text_context_t *ctx, uint16_t color) {
    if (ctx) {
        ctx->color = color;
    }
}

/**
 * Set text alignment
 */
void ft_text_set_alignment(ft_text_context_t *ctx, ft_text_align_t align) {
    if (ctx) {
        ctx->align = align;
    }
}

/**
 * Get text width (UTF-8 aware)
 */
int ft_text_get_width(ft_text_context_t *ctx, const char *text) {
    if (!ctx || !ctx->face || !text) return 0;
    
    int width = 0;
    const unsigned char *p = (const unsigned char *)text;
    
    while (*p) {
        uint32_t codepoint = utf8_decode(&p);
        if (codepoint == '\n') continue;
        
        /* Load glyph */
        FT_UInt glyph_index = FT_Get_Char_Index(ctx->face, codepoint);
        if (FT_Load_Glyph(ctx->face, glyph_index, FT_LOAD_DEFAULT)) {
            continue;
        }
        
        width += ctx->face->glyph->advance.x >> 6;
    }
    
    return width;
}

/**
 * Get text height (actual font height including ascent and descent)
 */
int ft_text_get_height(ft_text_context_t *ctx) {
    if (!ctx || !ctx->face) return 0;
    
    /* Calculate real height from font metrics */
    int ascent = ctx->face->size->metrics.ascender >> 6;
    int descent = -(ctx->face->size->metrics.descender >> 6);
    
    return ascent + descent;
}

/**
 * Draw a single glyph
 */
static void draw_glyph(ft_text_context_t *ctx, FT_Bitmap *bitmap, 
                       int x, int y, uint16_t color) {
    for (unsigned int row = 0; row < bitmap->rows; row++) {
        for (unsigned int col = 0; col < bitmap->width; col++) {
            int pixel_x = x + col;
            int pixel_y = y + row;
            
            /* Get pixel value (0-255) */
            unsigned char value = bitmap->buffer[row * bitmap->pitch + col];
            
            /* Skip fully transparent pixels */
            if (value < 30) continue;
            
            /* Draw pixel with alpha blending approximation */
            /* For simplicity, we just draw if value > threshold */
            if (value > 128) {
                weact_draw_pixel(ctx->display, pixel_x, pixel_y, color);
            }
        }
    }
}

/**
 * Draw text (UTF-8 with Cyrillic support!)
 */
void ft_text_draw(ft_text_context_t *ctx, int x, int y, const char *text) {
    if (!ctx || !ctx->face || !text) return;
    
    /* Calculate ascent (height above baseline) */
    /* This converts Y from "top of text" to "baseline" */
    int ascent = ctx->face->size->metrics.ascender >> 6;
    
    int pen_x = x;
    int pen_y = y + ascent;  /* Move baseline down by ascent */
    
    /* Apply alignment */
    if (ctx->align == FT_TEXT_ALIGN_CENTER) {
        int text_width = ft_text_get_width(ctx, text);
        pen_x = x - text_width / 2;
    } else if (ctx->align == FT_TEXT_ALIGN_RIGHT) {
        int text_width = ft_text_get_width(ctx, text);
        pen_x = x - text_width;
    }
    
    /* Safety check */
    if (pen_x < 0) pen_x = 0;
    
    /* Draw each character */
    const unsigned char *p = (const unsigned char *)text;
    
    while (*p) {
        uint32_t codepoint = utf8_decode(&p);
        
        /* Handle newline */
        if (codepoint == '\n') {
            pen_x = x;
            pen_y += ctx->font_size;
            continue;
        }
        
        /* Load glyph */
        FT_UInt glyph_index = FT_Get_Char_Index(ctx->face, codepoint);
        if (FT_Load_Glyph(ctx->face, glyph_index, FT_LOAD_DEFAULT)) {
            continue;
        }
        
        /* Render glyph */
        if (FT_Render_Glyph(ctx->face->glyph, FT_RENDER_MODE_NORMAL)) {
            continue;
        }
        
        /* Draw glyph bitmap */
        draw_glyph(ctx, &ctx->face->glyph->bitmap,
                   pen_x + ctx->face->glyph->bitmap_left,
                   pen_y - ctx->face->glyph->bitmap_top,
                   ctx->color);
        
        /* Advance pen */
        pen_x += ctx->face->glyph->advance.x >> 6;
    }
}

/**
 * Draw text with word wrapping (UTF-8 aware)
 */
void ft_text_draw_wrapped(ft_text_context_t *ctx, int x, int y,
                           int max_width, int max_height, const char *text) {
    if (!ctx || !text) return;
    
    int current_y = y;
    
    /* Calculate proper line height from font metrics */
    int ascent = ctx->face->size->metrics.ascender >> 6;
    int descent = -(ctx->face->size->metrics.descender >> 6);
    int line_height = ascent + descent + 2;  /* +2 for line spacing */
    
    /* Simple word wrapping */
    char line_buffer[512];
    int line_pos = 0;
    const char *word_start = text;
    const char *p = text;
    
    while (*p) {
        /* Find word boundary */
        while (*p && *p != ' ' && *p != '\n' && *p != '\t') p++;
        
        int word_len = p - word_start;
        
        /* Build test line */
        char test_line[512];
        snprintf(test_line, sizeof(test_line), "%.*s%s%.*s",
                 line_pos, line_buffer,
                 (line_pos > 0) ? " " : "",
                 word_len, word_start);
        
        int test_width = ft_text_get_width(ctx, test_line);
        
        /* Check if line is too long */
        if (test_width > max_width && line_pos > 0) {
            /* Draw current line */
            line_buffer[line_pos] = '\0';
            ft_text_draw(ctx, x, current_y, line_buffer);
            
            current_y += line_height;
            if (current_y + line_height > y + max_height) {
                break;
            }
            
            /* Start new line with current word */
            snprintf(line_buffer, sizeof(line_buffer), "%.*s", word_len, word_start);
            line_pos = word_len;
        } else {
            /* Add word to current line */
            if (line_pos > 0 && line_pos < (int)sizeof(line_buffer) - 1) {
                line_buffer[line_pos++] = ' ';
            }
            for (int i = 0; i < word_len && line_pos < (int)sizeof(line_buffer) - 1; i++) {
                line_buffer[line_pos++] = word_start[i];
            }
        }
        
        /* Handle newline */
        if (*p == '\n') {
            if (line_pos > 0) {
                line_buffer[line_pos] = '\0';
                ft_text_draw(ctx, x, current_y, line_buffer);
                current_y += line_height;
                line_pos = 0;
            }
            p++;
            word_start = p;
            continue;
        }
        
        /* Skip whitespace */
        while (*p == ' ' || *p == '\t') p++;
        word_start = p;
    }
    
    /* Draw last line */
    if (line_pos > 0) {
        line_buffer[line_pos] = '\0';
        ft_text_draw(ctx, x, current_y, line_buffer);
    }
}
