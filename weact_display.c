/**
 * WeAct Display FS Library Implementation
 */

#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "weact_display.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <time.h>

/* Private helper function to send command */
static bool send_command(weact_display_t *display, const uint8_t *data, size_t length) {
    if (!display->is_connected) {
        snprintf(display->last_error, sizeof(display->last_error), 
                 "Display not connected");
        return false;
    }
    
    ssize_t written = write(display->fd, data, length);
    if (written < 0) {
        snprintf(display->last_error, sizeof(display->last_error), 
                 "Write error: %s", strerror(errno));
        return false;
    }
    
    if ((size_t)written != length) {
        snprintf(display->last_error, sizeof(display->last_error), 
                 "Incomplete write: %zd of %zu bytes", written, length);
        return false;
    }
    
    usleep(5000); /* 5ms delay */
    return true;
}

/* Color Conversion: RGB888 to BRG565 */
uint16_t weact_rgb_to_brg565(uint8_t r, uint8_t g, uint8_t b) {
    uint8_t r5 = (r >> 3) & 0x1F;  /* 5 bits red */
    uint8_t g6 = (g >> 2) & 0x3F;  /* 6 bits green */
    uint8_t b5 = (b >> 3) & 0x1F;  /* 5 bits blue */
    
    /* BRG565 format: BBBBB RRRRR GGGGGG */
    return (b5 << 11) | (r5 << 6) | g6;
}

/* Initialize display */
bool weact_init(weact_display_t *display, const char *port_name) {
    if (!display || !port_name) {
        return false;
    }
    
    memset(display, 0, sizeof(weact_display_t));
    strncpy(display->port_name, port_name, sizeof(display->port_name) - 1);
    
    /* Open serial port */
    display->fd = open(port_name, O_RDWR | O_NOCTTY | O_SYNC);
    if (display->fd < 0) {
        snprintf(display->last_error, sizeof(display->last_error),
                 "Failed to open port %s: %s", port_name, strerror(errno));
        return false;
    }
    
    /* Configure serial port */
    struct termios tty;
    if (tcgetattr(display->fd, &tty) != 0) {
        snprintf(display->last_error, sizeof(display->last_error),
                 "Error getting terminal attributes: %s", strerror(errno));
        close(display->fd);
        return false;
    }
    
    /* Set baud rate */
    cfsetospeed(&tty, WEACT_BAUDRATE);
    cfsetispeed(&tty, WEACT_BAUDRATE);
    
    /* 8N1 mode */
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;  /* 8 data bits */
    tty.c_cflag &= ~(PARENB | PARODD);            /* No parity */
    tty.c_cflag &= ~CSTOPB;                        /* 1 stop bit */
#ifdef CRTSCTS
    tty.c_cflag &= ~CRTSCTS;                       /* No hardware flow control */
#endif
    tty.c_cflag |= (CLOCAL | CREAD);               /* Enable receiver, ignore modem lines */
    
    /* Raw mode */
    tty.c_lflag = 0;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);       /* No software flow control */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
    tty.c_oflag = 0;
    
    /* Timeouts */
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 10;  /* 1 second timeout */
    
    if (tcsetattr(display->fd, TCSANOW, &tty) != 0) {
        snprintf(display->last_error, sizeof(display->last_error),
                 "Error setting terminal attributes: %s", strerror(errno));
        close(display->fd);
        return false;
    }
    
    /* Allocate buffers */
    display->frame_buffer = (uint8_t *)malloc(WEACT_MAX_BUFFER_SIZE);
    display->back_buffer = (uint8_t *)malloc(WEACT_MAX_BUFFER_SIZE);
    
    if (!display->frame_buffer || !display->back_buffer) {
        snprintf(display->last_error, sizeof(display->last_error),
                 "Failed to allocate memory buffers");
        if (display->frame_buffer) free(display->frame_buffer);
        if (display->back_buffer) free(display->back_buffer);
        close(display->fd);
        return false;
    }
    
    memset(display->frame_buffer, 0, WEACT_MAX_BUFFER_SIZE);
    memset(display->back_buffer, 0, WEACT_MAX_BUFFER_SIZE);
    
    /* Initialize display state */
    display->is_connected = true;
    display->orientation = WEACT_LANDSCAPE;
    display->brightness = 255;
    display->display_width = WEACT_DISPLAY_WIDTH;
    display->display_height = WEACT_DISPLAY_HEIGHT;
    display->last_error[0] = '\0';
    
    /* Set initial orientation */
    weact_set_orientation(display, WEACT_LANDSCAPE);
    usleep(500000); /* 500ms delay */
    
    return true;
}

/* Close display connection */
void weact_close(weact_display_t *display) {
    if (!display) return;
    
    if (display->is_connected && display->fd >= 0) {
        close(display->fd);
        display->fd = -1;
        display->is_connected = false;
    }
    
    if (display->frame_buffer) {
        free(display->frame_buffer);
        display->frame_buffer = NULL;
    }
    
    if (display->back_buffer) {
        free(display->back_buffer);
        display->back_buffer = NULL;
    }
}

/* Cleanup all resources */
void weact_cleanup(weact_display_t *display) {
    weact_close(display);
}

/* Clear buffer with color */
void weact_clear_buffer(weact_display_t *display, uint16_t color) {
    if (!display || !display->back_buffer) return;
    
    uint8_t color_l = color >> 8;
    uint8_t color_h = color & 0xFF;
    
    int pixel_count = display->display_width * display->display_height;
    for (int i = 0; i < pixel_count; i++) {
        display->back_buffer[i * 2] = color_l;
        display->back_buffer[i * 2 + 1] = color_h;
    }
}

/* Swap front and back buffers */
void weact_swap_buffers(weact_display_t *display) {
    if (!display) return;
    
    uint8_t *temp = display->frame_buffer;
    display->frame_buffer = display->back_buffer;
    display->back_buffer = temp;
}

/* Flush buffer to display */
bool weact_flush_buffer(weact_display_t *display) {
    if (!display || !display->back_buffer || !display->is_connected) {
        return false;
    }
    
    /* Prepare SET_BITMAP command (0x05) */
    uint8_t cmd[10];
    cmd[0] = 0x05;  /* SET_BITMAP command */
    
    /* Start coordinates (0, 0) */
    cmd[1] = 0;
    cmd[2] = 0;
    cmd[3] = 0;
    cmd[4] = 0;
    
    /* End coordinates (width-1, height-1) */
    int x_end = display->display_width - 1;
    cmd[5] = x_end & 0xFF;
    cmd[6] = (x_end >> 8) & 0xFF;
    
    int y_end = display->display_height - 1;
    cmd[7] = y_end & 0xFF;
    cmd[8] = (y_end >> 8) & 0xFF;
    
    cmd[9] = 0x0A;  /* Terminator */
    
    /* Send command */
    if (!send_command(display, cmd, 10)) {
        return false;
    }
    
    usleep(10000); /* 10ms delay */
    
    /* Send image data */
    size_t bytes_to_send = display->display_width * display->display_height * 2;
    ssize_t written = write(display->fd, display->back_buffer, bytes_to_send);
    
    if (written < 0) {
        snprintf(display->last_error, sizeof(display->last_error),
                 "Failed to send image data: %s", strerror(errno));
        return false;
    }
    
    if ((size_t)written != bytes_to_send) {
        snprintf(display->last_error, sizeof(display->last_error),
                 "Incomplete image data write: %zd of %zu bytes", written, bytes_to_send);
        return false;
    }
    
    usleep(10000); /* 10ms delay */
    return true;
}

/* Update display (flush and swap) */
bool weact_update_display(weact_display_t *display) {
    if (weact_flush_buffer(display)) {
        weact_swap_buffers(display);
        return true;
    }
    return false;
}

/* Draw pixel in back buffer */
void weact_draw_pixel(weact_display_t *display, int x, int y, uint16_t color) {
    if (!display || !display->back_buffer) return;
    
    if (x < 0 || x >= display->display_width || y < 0 || y >= display->display_height) {
        return;
    }
    
    int offset = (y * display->display_width + x) * 2;
    display->back_buffer[offset] = color >> 8;
    display->back_buffer[offset + 1] = color & 0xFF;
}

/* Draw line (Bresenham's algorithm) */
void weact_draw_line(weact_display_t *display, int x1, int y1, int x2, int y2, uint16_t color) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    while (true) {
        weact_draw_pixel(display, x1, y1, color);
        
        if (x1 == x2 && y1 == y2) break;
        
        int e2 = err * 2;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

/* Draw rectangle */
void weact_draw_rect(weact_display_t *display, int x, int y, int width, int height, 
                     uint16_t color, bool filled) {
    if (filled) {
        for (int yy = y; yy < y + height; yy++) {
            if (yy >= 0 && yy < display->display_height) {
                for (int xx = x; xx < x + width; xx++) {
                    if (xx >= 0 && xx < display->display_width) {
                        weact_draw_pixel(display, xx, yy, color);
                    }
                }
            }
        }
    } else {
        /* Top and bottom edges */
        for (int xx = x; xx < x + width; xx++) {
            weact_draw_pixel(display, xx, y, color);
            weact_draw_pixel(display, xx, y + height - 1, color);
        }
        /* Left and right edges */
        for (int yy = y; yy < y + height; yy++) {
            weact_draw_pixel(display, x, yy, color);
            weact_draw_pixel(display, x + width - 1, yy, color);
        }
    }
}

/* Draw circle (Bresenham's algorithm) */
void weact_draw_circle(weact_display_t *display, int cx, int cy, int radius, 
                       uint16_t color, bool filled) {
    if (filled) {
        for (int y = -radius; y <= radius; y++) {
            for (int x = -radius; x <= radius; x++) {
                if (x * x + y * y <= radius * radius) {
                    weact_draw_pixel(display, cx + x, cy + y, color);
                }
            }
        }
    } else {
        int x = 0;
        int y = radius;
        int d = 3 - 2 * radius;
        
        while (x <= y) {
            weact_draw_pixel(display, cx + x, cy + y, color);
            weact_draw_pixel(display, cx - x, cy + y, color);
            weact_draw_pixel(display, cx + x, cy - y, color);
            weact_draw_pixel(display, cx - x, cy - y, color);
            weact_draw_pixel(display, cx + y, cy + x, color);
            weact_draw_pixel(display, cx - y, cy + x, color);
            weact_draw_pixel(display, cx + y, cy - x, color);
            weact_draw_pixel(display, cx - y, cy - x, color);
            
            if (d < 0) {
                d = d + 4 * x + 6;
            } else {
                d = d + 4 * (x - y) + 10;
                y--;
            }
            x++;
        }
    }
}

/* Set display orientation */
bool weact_set_orientation(weact_display_t *display, weact_orientation_t orientation) {
    if (!display) return false;
    
    if (orientation > 3 && orientation != 5) {
        snprintf(display->last_error, sizeof(display->last_error),
                 "Invalid orientation value. Use 0-3 or 5 for ROTATE");
        return false;
    }
    
    /* Determine new dimensions */
    int new_width, new_height;
    
    switch (orientation) {
        case WEACT_PORTRAIT:
        case WEACT_REVERSE_PORTRAIT:
            new_width = 80;
            new_height = 160;
            break;
        case WEACT_LANDSCAPE:
        case WEACT_REVERSE_LANDSCAPE:
            new_width = 160;
            new_height = 80;
            break;
        case WEACT_ROTATE:
            /* Auto-rotation - keep current dimensions */
            new_width = display->display_width;
            new_height = display->display_height;
            break;
        default:
            return false;
    }
    
    /* Send orientation command (0x02) */
    uint8_t cmd[3];
    cmd[0] = 0x02;
    cmd[1] = orientation;
    cmd[2] = 0x0A;
    
    if (!send_command(display, cmd, 3)) {
        return false;
    }
    
    display->orientation = orientation;
    display->display_width = new_width;
    display->display_height = new_height;
    
    usleep(100000); /* 100ms delay */
    
    /* Clear buffers after orientation change */
    if (display->frame_buffer && display->back_buffer) {
        memset(display->frame_buffer, 0, WEACT_MAX_BUFFER_SIZE);
        memset(display->back_buffer, 0, WEACT_MAX_BUFFER_SIZE);
    }
    
    return true;
}

/* Set brightness */
bool weact_set_brightness(weact_display_t *display, uint8_t brightness, uint16_t time_ms) {
    if (!display) return false;
    
    if (time_ms > 5000) time_ms = 5000;
    
    /* Send brightness command (0x03) */
    uint8_t cmd[5];
    cmd[0] = 0x03;
    cmd[1] = brightness;
    cmd[2] = time_ms & 0xFF;
    cmd[3] = (time_ms >> 8) & 0xFF;
    cmd[4] = 0x0A;
    
    if (send_command(display, cmd, 5)) {
        display->brightness = brightness;
        return true;
    }
    
    return false;
}

/* Fill entire screen with color (FULL command) */
bool weact_fill_screen(weact_display_t *display, uint16_t color) {
    if (!display || !display->is_connected) {
        if (display) {
            snprintf(display->last_error, sizeof(display->last_error),
                     "Display not connected");
        }
        return false;
    }
    
    /* Send FULL command (0x04) */
    uint8_t cmd[12];
    cmd[0] = 0x04;  /* FULL command */
    
    /* Start coordinates (0, 0) */
    cmd[1] = 0;
    cmd[2] = 0;
    cmd[3] = 0;
    cmd[4] = 0;
    
    /* End coordinates (width-1, height-1) */
    int x_end = display->display_width - 1;
    cmd[5] = x_end & 0xFF;
    cmd[6] = (x_end >> 8) & 0xFF;
    
    int y_end = display->display_height - 1;
    cmd[7] = y_end & 0xFF;
    cmd[8] = (y_end >> 8) & 0xFF;
    
    /* Color in RGB565 format */
    cmd[9] = color & 0xFF;
    cmd[10] = (color >> 8) & 0xFF;
    
    cmd[11] = 0x0A;  /* Terminator */
    
    if (send_command(display, cmd, 12)) {
        usleep(50000); /* 50ms delay */
        return true;
    }
    
    return false;
}

/* System reset */
bool weact_system_reset(weact_display_t *display) {
    if (!display) return false;
    
    /* Send reset command (0x40) */
    uint8_t cmd[2];
    cmd[0] = 0x40;
    cmd[1] = 0x0A;
    
    if (send_command(display, cmd, 2)) {
        usleep(1000000); /* 1 second delay for reset */
        return true;
    }
    
    return false;
}

/* Information functions */
bool weact_is_connected(const weact_display_t *display) {
    return display && display->is_connected;
}

int weact_get_display_width(const weact_display_t *display) {
    return display ? display->display_width : 0;
}

int weact_get_display_height(const weact_display_t *display) {
    return display ? display->display_height : 0;
}

weact_orientation_t weact_get_orientation(const weact_display_t *display) {
    return display ? display->orientation : WEACT_LANDSCAPE;
}

uint8_t weact_get_brightness(const weact_display_t *display) {
    return display ? display->brightness : 0;
}

const char *weact_get_last_error(const weact_display_t *display) {
    return display ? display->last_error : "Invalid display pointer";
}

void weact_get_info(const weact_display_t *display, char *buffer, size_t size) {
    if (!buffer || size == 0) return;
    
    if (display && display->is_connected) {
        snprintf(buffer, size, "WeAct Display FS 0.96-inch (%s) %dx%d",
                 display->port_name, display->display_width, display->display_height);
    } else {
        snprintf(buffer, size, "Not connected");
    }
}
