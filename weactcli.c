/**
 * WeActCLI - Console Text Display Utility for Linux
 * 
 * Usage: weactcli -p /dev/ttyUSB0 [options] "text"
 * 
 * Port of Windows/PureBasic version to Linux/C
 * Now with full Cyrillic support via FreeType2!
 */

#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "weact_display.h"
#include "text_freetype.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <sys/time.h>

/* Configuration structure */
typedef struct {
    char port[256];
    char text[8192];
    char file_path[512];
    uint16_t color;
    int font_size;
    bool center;
    bool clear_only;
    bool verbose;
    bool scroll;
    float scroll_speed;
    weact_scroll_dir_t scroll_direction;
    bool read_stdin;
} cli_config_t;

/* Global config */
static cli_config_t config = {
    .port = "/dev/ttyUSB0",
    .text = "",
    .file_path = "",
    .color = WEACT_WHITE,
    .font_size = FT_FONT_SIZE_MEDIUM,
    .center = false,
    .clear_only = false,
    .verbose = false,
    .scroll = false,
    .scroll_speed = 30.0f,
    .scroll_direction = SCROLL_UP,
    .read_stdin = false
};

/* Color name to BRG565 */
static uint16_t parse_color(const char *color_name) {
    if (!color_name) return WEACT_WHITE;
    
    if (strcasecmp(color_name, "red") == 0) return WEACT_RED;
    if (strcasecmp(color_name, "green") == 0) return WEACT_GREEN;
    if (strcasecmp(color_name, "blue") == 0) return WEACT_BLUE;
    if (strcasecmp(color_name, "white") == 0) return WEACT_WHITE;
    if (strcasecmp(color_name, "black") == 0) return WEACT_BLACK;
    if (strcasecmp(color_name, "yellow") == 0) return WEACT_YELLOW;
    if (strcasecmp(color_name, "cyan") == 0) return WEACT_CYAN;
    if (strcasecmp(color_name, "magenta") == 0) return WEACT_MAGENTA;
    
    fprintf(stderr, "Warning: Unknown color '%s', using white\n", color_name);
    return WEACT_WHITE;
}

/* Parse font size */
static int parse_font_size(const char *size_str) {
    if (!size_str) return FT_FONT_SIZE_MEDIUM;
    
    int size = atoi(size_str);
    
    if (size <= 8) return FT_FONT_SIZE_SMALL;
    if (size <= 12) return FT_FONT_SIZE_MEDIUM;
    return FT_FONT_SIZE_LARGE;
}

/* Parse scroll parameter */
static bool parse_scroll(const char *scroll_str) {
    if (!scroll_str) return false;
    
    char *colon = strchr(scroll_str, ':');
    
    /* Parse speed */
    config.scroll_speed = atof(scroll_str);
    if (config.scroll_speed <= 0 || config.scroll_speed > 100) {
        fprintf(stderr, "Warning: Scroll speed %.1f out of range, using 30.0\n", 
                config.scroll_speed);
        config.scroll_speed = 30.0f;
    }
    
    /* Parse direction */
    if (colon) {
        char dir = tolower(*(colon + 1));
        if (dir == 'u') {
            config.scroll_direction = SCROLL_UP;
        } else if (dir == 'd') {
            config.scroll_direction = SCROLL_DOWN;
        } else {
            fprintf(stderr, "Warning: Unknown direction '%c', using up\n", dir);
            config.scroll_direction = SCROLL_UP;
        }
    }
    
    return true;
}

/* Read text from stdin */
static bool read_from_stdin(char *buffer, size_t max_size) {
    if (!buffer || max_size == 0) return false;
    
    size_t total_read = 0;
    char line[1024];
    
    while (fgets(line, sizeof(line), stdin)) {
        size_t line_len = strlen(line);
        if (total_read + line_len < max_size - 1) {
            strcat(buffer, line);
            total_read += line_len;
        } else {
            break;
        }
    }
    
    return total_read > 0;
}

/* Load text from file */
static bool load_from_file(const char *path, char *buffer, size_t max_size) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", path);
        return false;
    }
    
    size_t total_read = 0;
    char line[1024];
    
    while (fgets(line, sizeof(line), fp) && total_read < max_size - 1) {
        size_t line_len = strlen(line);
        if (total_read + line_len < max_size - 1) {
            strcat(buffer, line);
            total_read += line_len;
        } else {
            break;
        }
    }
    
    fclose(fp);
    
    if (config.verbose) {
        printf("Loaded %zu bytes from file: %s\n", total_read, path);
    }
    
    return total_read > 0;
}

/* Display help */
static void show_help(const char *prog_name) {
    printf("WeActCLI - Console Text Display Utility for WeAct Display FS\n");
    printf("\n");
    printf("USAGE MODES:\n");
    printf("  1. Command line:  %s -p /dev/ttyUSB0 \"Hello World\"\n", prog_name);
    printf("  2. File input:    %s -p /dev/ttyUSB0 -f log.txt\n", prog_name);
    printf("  3. Pipe:          echo \"Hello\" | %s -p /dev/ttyUSB0\n", prog_name);
    printf("  4. Redirection:   %s -p /dev/ttyUSB0 < log.txt\n", prog_name);
    printf("  5. Clear screen:  %s -p /dev/ttyUSB0 --cls\n", prog_name);
    printf("\n");
    printf("OPTIONS:\n");
    printf("  -p, --port PORT       Serial port (required, e.g., /dev/ttyUSB0)\n");
    printf("  -c, --color COLOR     Text color: red, green, blue, white, black,\n");
    printf("                        yellow, cyan, magenta (default: white)\n");
    printf("  -s, --scroll SPEED[:DIR]\n");
    printf("                        Scroll speed (px/s) and direction (u=up, d=down)\n");
    printf("                        Example: -s 25.5:u or -s 40:d\n");
    printf("  -z, --size SIZE       Font size: 8 (small), 12 (medium), 16 (large)\n");
    printf("                        Default: 12\n");
    printf("  -f, --file FILE       Load text from file\n");
    printf("  -i, --stdin           Read from stdin (auto-detected with pipes)\n");
    printf("  --center              Center text horizontally\n");
    printf("  --cls                 Clear screen only\n");
    printf("  -v, --verbose         Verbose output\n");
    printf("  -h, --help            Show this help\n");
    printf("\n");
    printf("EXAMPLES:\n");
    printf("  %s -p /dev/ttyUSB0 -c green --center \"Status: OK\"\n", prog_name);
    printf("  %s -p /dev/ttyUSB0 -s 25.5:u \"Scrolling text...\"\n", prog_name);
    printf("  dir | %s -p /dev/ttyUSB0 -c yellow\n", prog_name);
    printf("  %s -p /dev/ttyUSB0 --cls\n", prog_name);
    printf("\n");
    printf("NOTES:\n");
    printf("  - Port is always required\n");
    printf("  - File input has priority over stdin\n");
    printf("  - Stdin has priority over command line text\n");
    printf("  - Common ports: /dev/ttyUSB0, /dev/ttyACM0, /dev/ttyS0\n");
    printf("\n");
}

/* Simple scrolling animation */
static void display_scrolling_text(weact_display_t *display, ft_text_context_t *text_ctx, 
                                   const char *text) {
    int display_width = weact_get_display_width(display);
    int display_height = weact_get_display_height(display);
    
    int text_width = ft_text_get_width(text_ctx, text);
    int text_height = ft_text_get_height(text_ctx);
    
    /* Starting positions based on direction */
    float position;
    float start_pos, end_pos;
    
    if (config.scroll_direction == SCROLL_UP) {
        start_pos = display_height;
        end_pos = -text_height;
    } else {
        start_pos = -text_height;
        end_pos = display_height;
    }
    
    position = start_pos;
    
    /* Frame timing */
    struct timeval last_time, current_time;
    gettimeofday(&last_time, NULL);
    
    float accumulated_pixels = 0.0f;
    bool keep_scrolling = true;
    
    while (keep_scrolling) {
        /* Calculate delta time */
        gettimeofday(&current_time, NULL);
        float delta_time = (current_time.tv_sec - last_time.tv_sec) + 
                          (current_time.tv_usec - last_time.tv_usec) / 1000000.0f;
        last_time = current_time;
        
        /* Calculate pixels to move */
        float pixels_to_move = config.scroll_speed * delta_time;
        accumulated_pixels += pixels_to_move;
        
        /* Move only whole pixels */
        int int_pixels = (int)accumulated_pixels;
        if (int_pixels >= 1) {
            accumulated_pixels -= int_pixels;
            
            if (config.scroll_direction == SCROLL_UP) {
                position -= int_pixels;
                if (position <= end_pos) keep_scrolling = false;
            } else {
                position += int_pixels;
                if (position >= end_pos) keep_scrolling = false;
            }
        }
        
        /* Render frame */
        weact_clear_buffer(display, WEACT_BLACK);
        
        int y = (int)position;
        int x = config.center ? (display_width - text_width) / 2 : 5;
        
        ft_text_draw(text_ctx, x, y, text);
        
        weact_update_display(display);
        
        usleep(33000); /* ~30 FPS */
    }
    
    /* Clear at end */
    weact_clear_buffer(display, WEACT_BLACK);
    weact_update_display(display);
}

/* Display static text */
static void display_static_text(weact_display_t *display, ft_text_context_t *text_ctx, 
                                const char *text) {
    int display_width = weact_get_display_width(display);
    int display_height = weact_get_display_height(display);
    
    weact_clear_buffer(display, WEACT_BLACK);
    
    /* Calculate text dimensions */
    int text_width = ft_text_get_width(text_ctx, text);
    int text_height = ft_text_get_height(text_ctx);
    
    if (config.verbose) {
        printf("Text dimensions: %dx%d pixels\n", text_width, text_height);
        printf("Display dimensions: %dx%d pixels\n", display_width, display_height);
    }
    
    /* Always use LEFT alignment and calculate X manually */
    ft_text_set_alignment(text_ctx, FT_TEXT_ALIGN_LEFT);
    
    if (config.center) {
        if (text_width <= display_width - 10) {
            /* Text fits on one line - calculate center position */
            int x = (display_width - text_width) / 2;
            int y = (display_height - text_height) / 2;
            
            /* Ensure X is not negative */
            if (x < 0) x = 0;
            
            if (config.verbose) {
                printf("Single line centered at: x=%d, y=%d\n", x, y);
            }
            
            ft_text_draw(text_ctx, x, y, text);
        } else {
            /* Text too long - need word wrapping with manual centering */
            if (config.verbose) {
                printf("Text too long (%d > %d), using word wrap\n", 
                       text_width, display_width - 10);
            }
            
            /* For now, just use left-aligned word wrap */
            /* TODO: Implement per-line centering in ft_text_draw_wrapped */
            ft_text_draw_wrapped(text_ctx, 5, 5, display_width - 10, 
                             display_height - 10, text);
        }
    } else {
        /* Normal left-aligned text */
        if (config.verbose) {
            printf("Drawing left-aligned text\n");
        }
        
        /* Simple case - just draw at 5,5 */
        if (text_width <= display_width - 10) {
            ft_text_draw(text_ctx, 5, 5, text);
        } else {
            ft_text_draw_wrapped(text_ctx, 5, 5, display_width - 10, 
                             display_height - 10, text);
        }
    }
    
    weact_update_display(display);
    
    if (config.verbose) {
        printf("Display updated\n");
    }
    
    sleep(2); /* Show for 2 seconds */
}

/* Main program */
int main(int argc, char *argv[]) {
    /* Check if stdin is a pipe/redirect */
    if (!isatty(STDIN_FILENO)) {
        config.read_stdin = true;
    }
    
    /* Parse command line options */
    static struct option long_options[] = {
        {"port",    required_argument, 0, 'p'},
        {"color",   required_argument, 0, 'c'},
        {"scroll",  required_argument, 0, 's'},
        {"size",    required_argument, 0, 'z'},
        {"file",    required_argument, 0, 'f'},
        {"stdin",   no_argument,       0, 'i'},
        {"center",  no_argument,       0, 'C'},
        {"cls",     no_argument,       0, 'L'},
        {"verbose", no_argument,       0, 'v'},
        {"help",    no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    bool port_specified = false;
    
    while ((opt = getopt_long(argc, argv, "p:c:s:z:f:iCLvh", 
                              long_options, &option_index)) != -1) {
        switch (opt) {
            case 'p':
                strncpy(config.port, optarg, sizeof(config.port) - 1);
                port_specified = true;
                break;
            case 'c':
                config.color = parse_color(optarg);
                break;
            case 's':
                config.scroll = parse_scroll(optarg);
                break;
            case 'z':
                config.font_size = parse_font_size(optarg);
                break;
            case 'f':
                strncpy(config.file_path, optarg, sizeof(config.file_path) - 1);
                break;
            case 'i':
                config.read_stdin = true;
                break;
            case 'C':
                config.center = true;
                break;
            case 'L':
                config.clear_only = true;
                break;
            case 'v':
                config.verbose = true;
                break;
            case 'h':
                show_help(argv[0]);
                return 0;
            default:
                show_help(argv[0]);
                return 1;
        }
    }
    
    /* Check port specified */
    if (!port_specified) {
        fprintf(stderr, "Error: Port not specified. Use -p /dev/ttyUSB0\n\n");
        show_help(argv[0]);
        return 1;
    }
    
    /* Get text from remaining arguments */
    if (optind < argc && !config.read_stdin && config.file_path[0] == '\0') {
        for (int i = optind; i < argc; i++) {
            if (strlen(config.text) + strlen(argv[i]) + 2 < sizeof(config.text)) {
                if (config.text[0] != '\0') strcat(config.text, " ");
                strcat(config.text, argv[i]);
            }
        }
    }
    
    /* Priority: file > stdin > command line */
    if (config.file_path[0] != '\0') {
        if (!load_from_file(config.file_path, config.text, sizeof(config.text))) {
            return 1;
        }
    } else if (config.read_stdin) {
        if (!read_from_stdin(config.text, sizeof(config.text))) {
            if (!config.clear_only) {
                fprintf(stderr, "Error: No input from stdin\n");
                return 1;
            }
        }
    } else if (config.text[0] == '\0' && !config.clear_only) {
        fprintf(stderr, "Error: No text specified\n\n");
        show_help(argv[0]);
        return 1;
    }
    
    /* Verbose output */
    if (config.verbose) {
        printf("=== WeActCLI Configuration ===\n");
        printf("Port: %s\n", config.port);
        printf("Color: 0x%04X\n", config.color);
        printf("Font size: %d\n", config.font_size);
        printf("Center: %s\n", config.center ? "yes" : "no");
        printf("Clear only: %s\n", config.clear_only ? "yes" : "no");
        printf("Scroll: %s\n", config.scroll ? "yes" : "no");
        if (config.scroll) {
            printf("Scroll speed: %.1f px/s\n", config.scroll_speed);
            printf("Scroll direction: %s\n", 
                   config.scroll_direction == SCROLL_UP ? "up" : "down");
        }
        if (config.text[0] != '\0') {
            printf("Text length: %zu characters\n", strlen(config.text));
            if (strlen(config.text) > 100) {
                printf("Text preview: %.100s...\n", config.text);
            } else {
                printf("Text: %s\n", config.text);
            }
        }
        printf("==============================\n\n");
    }
    
    /* Initialize display */
    weact_display_t display;
    
    if (!weact_init(&display, config.port)) {
        fprintf(stderr, "Error: Failed to initialize display\n");
        fprintf(stderr, "Details: %s\n", weact_get_last_error(&display));
        fprintf(stderr, "\nPlease check:\n");
        fprintf(stderr, "  1. Device exists: ls -l %s\n", config.port);
        fprintf(stderr, "  2. Permissions: sudo chmod 666 %s\n", config.port);
        fprintf(stderr, "  3. User in dialout group: sudo usermod -a -G dialout $USER\n");
        return 1;
    }
    
    if (config.verbose) {
        char info[256];
        weact_get_info(&display, info, sizeof(info));
        printf("Display initialized: %s\n", info);
    }
    
    /* Clear screen only mode */
    if (config.clear_only) {
        weact_clear_buffer(&display, WEACT_BLACK);
        weact_update_display(&display);
        
        if (config.verbose) {
            printf("Screen cleared\n");
        }
        
        weact_cleanup(&display);
        return 0;
    }
    
    /* Initialize FreeType text renderer with Cyrillic support */
    ft_text_context_t *text_ctx = ft_text_init(&display, NULL, config.font_size);
    if (!text_ctx) {
        fprintf(stderr, "Error: Failed to initialize FreeType text renderer\n");
        fprintf(stderr, "Make sure libfreetype6 is installed:\n");
        fprintf(stderr, "  sudo apt-get install libfreetype6-dev\n");
        weact_close(&display);
        return 1;
    }
    
    ft_text_set_color(text_ctx, config.color);
    
    if (config.center) {
        ft_text_set_alignment(text_ctx, FT_TEXT_ALIGN_CENTER);
    }
    
    /* Display text */
    if (config.scroll) {
        display_scrolling_text(&display, text_ctx, config.text);
    } else {
        display_static_text(&display, text_ctx, config.text);
    }
    
    /* Cleanup */
    ft_text_cleanup(text_ctx);
    weact_cleanup(&display);
    
    if (config.verbose) {
        printf("\nOperation completed successfully\n");
    }
    
    return 0;
}
