/**
 * WeAct Display FS Library for Linux/C
 * Version 1.0
 * 
 * Support for WeAct Display FS 0.96-inch (160x80) via serial port
 * Protocol: v1.1
 * Compatibility: Linux (Debian 11-13+), GCC
 * 
 * Based on PureBasic version by CheshirCa
 * C/Linux port with enhanced features
 */

#ifndef WEACT_DISPLAY_H
#define WEACT_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Display Constants */
#define WEACT_DISPLAY_WIDTH  160
#define WEACT_DISPLAY_HEIGHT 80
#define WEACT_BAUDRATE       B115200
#define WEACT_MAX_BUFFER_SIZE (WEACT_DISPLAY_WIDTH * WEACT_DISPLAY_HEIGHT * 2)

/* Orientation Constants (Protocol v1.1) */
typedef enum {
    WEACT_PORTRAIT = 0,
    WEACT_REVERSE_PORTRAIT = 1,
    WEACT_LANDSCAPE = 2,
    WEACT_REVERSE_LANDSCAPE = 3,
    WEACT_ROTATE = 5  /* Auto-rotation mode */
} weact_orientation_t;

/* Scrolling Direction Constants */
typedef enum {
    SCROLL_LEFT = 0,
    SCROLL_RIGHT,
    SCROLL_UP,
    SCROLL_DOWN
} weact_scroll_dir_t;

/* Predefined Colors in BRG565 Format */
/* Note: Display uses BRG565 (Blue-Red-Green) not RGB565 */
#define WEACT_RED     0x07C0  /* BRG: 00000 11111 000000 */
#define WEACT_GREEN   0x001F  /* BRG: 00000 00000 011111 */
#define WEACT_BLUE    0xF800  /* BRG: 11111 00000 000000 */
#define WEACT_WHITE   0xFFFF  /* BRG: 11111 11111 111111 */
#define WEACT_BLACK   0x0000  /* BRG: 00000 00000 000000 */
#define WEACT_YELLOW  0x07FF  /* BRG: 00000 11111 111111 */
#define WEACT_CYAN    0xF81F  /* BRG: 11111 00000 111111 */
#define WEACT_MAGENTA 0xFFE0  /* BRG: 11111 11111 000000 */

/* Display Structure */
typedef struct {
    int fd;                      /* Serial port file descriptor */
    char port_name[256];         /* Port name (e.g., "/dev/ttyUSB0") */
    bool is_connected;           /* Connection status flag */
    weact_orientation_t orientation; /* Current orientation */
    uint8_t brightness;          /* Current brightness (0-255) */
    int display_width;           /* Current display width */
    int display_height;          /* Current display height */
    uint8_t *frame_buffer;       /* Frame buffer */
    uint8_t *back_buffer;        /* Back buffer (double buffering) */
    char last_error[512];        /* Last error message */
} weact_display_t;

/* Color Conversion */
uint16_t weact_rgb_to_brg565(uint8_t r, uint8_t g, uint8_t b);

/* Initialization and Cleanup */
bool weact_init(weact_display_t *display, const char *port_name);
void weact_close(weact_display_t *display);
void weact_cleanup(weact_display_t *display);

/* Buffer Operations */
void weact_clear_buffer(weact_display_t *display, uint16_t color);
void weact_swap_buffers(weact_display_t *display);
bool weact_flush_buffer(weact_display_t *display);
bool weact_update_display(weact_display_t *display);

/* Drawing Functions */
void weact_draw_pixel(weact_display_t *display, int x, int y, uint16_t color);
void weact_draw_line(weact_display_t *display, int x1, int y1, int x2, int y2, uint16_t color);
void weact_draw_rect(weact_display_t *display, int x, int y, int width, int height, uint16_t color, bool filled);
void weact_draw_circle(weact_display_t *display, int cx, int cy, int radius, uint16_t color, bool filled);

/* Display Control */
bool weact_set_orientation(weact_display_t *display, weact_orientation_t orientation);
bool weact_set_brightness(weact_display_t *display, uint8_t brightness, uint16_t time_ms);
bool weact_fill_screen(weact_display_t *display, uint16_t color);
bool weact_system_reset(weact_display_t *display);

/* Information Functions */
bool weact_is_connected(const weact_display_t *display);
int weact_get_display_width(const weact_display_t *display);
int weact_get_display_height(const weact_display_t *display);
weact_orientation_t weact_get_orientation(const weact_display_t *display);
uint8_t weact_get_brightness(const weact_display_t *display);
const char *weact_get_last_error(const weact_display_t *display);
void weact_get_info(const weact_display_t *display, char *buffer, size_t size);

#endif /* WEACT_DISPLAY_H */
