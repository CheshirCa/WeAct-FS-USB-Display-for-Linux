/**
 * WeActTerm - Terminal Emulator for WeAct Display
 * 
 * Provides a simple TTY terminal on WeAct Display for headless SBC debugging
 * Perfect for quick diagnostics on Raspberry Pi, Orange Pi, etc.
 * 
 * Usage: weactterm -p /dev/ttyACM0 [options]
 * 
 * Features:
 * - PTY-based terminal emulation
 * - Scrollback buffer
 * - Cyrillic support
 * - Basic ANSI escape sequence support
 */

#define _XOPEN_SOURCE 600
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "weact_display.h"
#include "text_freetype.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <pty.h>

/* Terminal configuration */
#define SCROLLBACK_LINES 100
#define INPUT_BUFFER_SIZE 1024
#define DISPLAY_WIDTH 160
#define DISPLAY_HEIGHT 80

/* Font size presets */
#define FONT_SMALL 8
#define FONT_MEDIUM 10
#define FONT_LARGE 12

/* Font types */
typedef enum {
    FONT_MONO = 0,      /* Monospace (DejaVu Sans Mono) - best for terminal */
    FONT_SANS,          /* Sans (DejaVu Sans) - proportional */
    FONT_SERIF          /* Serif (DejaVu Serif) - proportional */
} font_type_t;

/* Default font paths */
static const char *font_paths[][3] = {
    /* FONT_MONO */
    {
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf"
    },
    /* FONT_SANS */
    {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf"
    },
    /* FONT_SERIF */
    {
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",
        "/usr/share/fonts/TTF/DejaVuSerif.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf"
    }
};

/* Global state */
typedef struct {
    weact_display_t display;
    ft_text_context_t *text_ctx;
    
    /* PTY */
    int master_fd;
    int slave_fd;
    pid_t child_pid;
    
    /* Terminal state */
    char screen_buffer[SCROLLBACK_LINES][81];  /* Max 80 chars + null */
    int current_line;
    int total_lines;
    int scroll_offset;
    int cursor_x;
    int cursor_y;
    
    /* Scroll mode */
    bool scroll_mode;       /* ScrollLock enabled */
    int view_offset;        /* How many lines scrolled back */
    
    /* Display settings - dynamic based on font size */
    int font_size;
    font_type_t font_type;
    char font_path[512];
    int term_cols;
    int term_rows;
    int char_width;   /* Width of one character in pixels */
    int char_height;  /* Height of one character in pixels */
    bool flip_mode;   /* Flip landscape 180 degrees */
    
    char port[256];
    bool verbose;
    bool running;
    
    /* Original terminal settings */
    struct termios orig_termios;
} terminal_state_t;

static terminal_state_t term_state = {0};

/* Find font file for given type */
static const char* find_font_path(font_type_t type) {
    for (int i = 0; i < 3; i++) {
        const char *path = font_paths[type][i];
        FILE *f = fopen(path, "r");
        if (f) {
            fclose(f);
            return path;
        }
    }
    return NULL;
}

/* Calculate terminal dimensions based on font size */
static void calculate_terminal_dimensions(void) {
    /* Character dimensions based on font size and type */
    /* Monospace fonts have fixed width, proportional vary */
    
    if (term_state.font_type == FONT_MONO) {
        /* Monospace - all characters same width */
        switch (term_state.font_size) {
            case FONT_SMALL:   /* 8pt */
                term_state.char_width = 5;
                term_state.char_height = 10;
                break;
            case FONT_MEDIUM:  /* 10pt */
                term_state.char_width = 6;
                term_state.char_height = 12;
                break;
            case FONT_LARGE:   /* 12pt */
                term_state.char_width = 7;
                term_state.char_height = 14;
                break;
            default:
                term_state.char_width = 6;
                term_state.char_height = 12;
        }
    } else {
        /* Proportional fonts - use average width */
        switch (term_state.font_size) {
            case FONT_SMALL:
                term_state.char_width = 5;
                term_state.char_height = 9;
                break;
            case FONT_MEDIUM:
                term_state.char_width = 6;
                term_state.char_height = 11;
                break;
            case FONT_LARGE:
                term_state.char_width = 7;
                term_state.char_height = 13;
                break;
            default:
                term_state.char_width = 6;
                term_state.char_height = 11;
        }
    }
    
    /* Calculate how many columns and rows fit */
    /* Leave 4px margins (2px on each side) */
    term_state.term_cols = (DISPLAY_WIDTH - 4) / term_state.char_width;
    term_state.term_rows = (DISPLAY_HEIGHT - 4) / term_state.char_height;
    
    /* Sanity limits */
    if (term_state.term_cols < 10) term_state.term_cols = 10;
    if (term_state.term_cols > 80) term_state.term_cols = 80;
    if (term_state.term_rows < 4) term_state.term_rows = 4;
    if (term_state.term_rows > 20) term_state.term_rows = 20;
    
    if (term_state.verbose) {
        fprintf(stderr, "Terminal dimensions: %dx%d chars (%dx%d px per char)\n",
                term_state.term_cols, term_state.term_rows,
                term_state.char_width, term_state.char_height);
    }
}

/* Signal handler for cleanup */
static void signal_handler(int signo) {
    (void)signo;
    term_state.running = false;
}

/* Restore terminal settings */
static void restore_terminal(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term_state.orig_termios);
}

/* Set terminal to raw mode */
static bool set_raw_mode(void) {
    if (tcgetattr(STDIN_FILENO, &term_state.orig_termios) < 0) {
        perror("tcgetattr");
        return false;
    }
    
    atexit(restore_terminal);
    
    struct termios raw = term_state.orig_termios;
    
    /* Input modes - disable break, CR to NL, parity check, strip, start/stop */
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    
    /* Output modes - disable processing */
    raw.c_oflag &= ~(OPOST);
    
    /* Control modes - set 8 bit chars */
    raw.c_cflag |= (CS8);
    
    /* Local modes - disable echo, canonical mode, extended input, signals */
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    
    /* Control chars - read returns after 1 char or 100ms */
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) < 0) {
        perror("tcsetattr");
        return false;
    }
    
    return true;
}

/* Initialize screen buffer */
static void init_screen_buffer(void) {
    for (int i = 0; i < SCROLLBACK_LINES; i++) {
        memset(term_state.screen_buffer[i], 0, 81);
    }
    term_state.current_line = 0;
    term_state.total_lines = 1;  /* Start with 1 line available */
    term_state.scroll_offset = 0;
    term_state.cursor_x = 0;
    term_state.cursor_y = 0;
}

/* Add character to screen buffer */
static void add_char_to_buffer(char c) {
    if (c == '\n' || c == '\r') {
        /* Move to next line */
        term_state.cursor_x = 0;
        term_state.cursor_y++;
        
        /* Make sure we have a line to write to */
        if (term_state.cursor_y >= SCROLLBACK_LINES) {
            /* Shift buffer up */
            memmove(term_state.screen_buffer[0], 
                   term_state.screen_buffer[1],
                   (SCROLLBACK_LINES - 1) * 81);
            term_state.cursor_y = SCROLLBACK_LINES - 1;
            memset(term_state.screen_buffer[term_state.cursor_y], 0, 81);
        } else {
            /* Clear new line */
            memset(term_state.screen_buffer[term_state.cursor_y], 0, 81);
        }
        
    } else if (c == '\b' || c == 127) {
        /* Backspace */
        if (term_state.cursor_x > 0) {
            term_state.cursor_x--;
            term_state.screen_buffer[term_state.cursor_y][term_state.cursor_x] = ' ';
        }
        
    } else if (c >= 32 && c < 127) {
        /* Printable ASCII */
        if (term_state.cursor_x < term_state.term_cols && 
            term_state.cursor_x < 80) {  /* Hard limit for buffer */
            term_state.screen_buffer[term_state.cursor_y][term_state.cursor_x] = c;
            term_state.cursor_x++;
            
            if (term_state.cursor_x >= term_state.term_cols) {
                /* Auto wrap to next line */
                term_state.cursor_x = 0;
                term_state.cursor_y++;
                
                if (term_state.cursor_y >= SCROLLBACK_LINES) {
                    /* Shift buffer up */
                    memmove(term_state.screen_buffer[0], 
                           term_state.screen_buffer[1],
                           (SCROLLBACK_LINES - 1) * 81);
                    term_state.cursor_y = SCROLLBACK_LINES - 1;
                    memset(term_state.screen_buffer[term_state.cursor_y], 0, 81);
                }
            }
        }
    }
}

/* Simple ANSI escape sequence parser */
static void process_ansi_sequence(const char *seq, int len) {
    (void)seq;
    (void)len;
    /* Basic implementation - just ignore for now */
    /* Can be extended to support colors, cursor movement, etc. */
}

/* Process output from PTY */
static void process_pty_output(const char *data, ssize_t len) {
    static char escape_buffer[64];
    static int escape_len = 0;
    static bool in_escape = false;
    
    for (ssize_t i = 0; i < len; i++) {
        char c = data[i];
        
        if (in_escape) {
            escape_buffer[escape_len++] = c;
            
            /* Check for end of escape sequence */
            if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || escape_len >= 63) {
                process_ansi_sequence(escape_buffer, escape_len);
                in_escape = false;
                escape_len = 0;
            }
        } else if (c == 27) { /* ESC */
            in_escape = true;
            escape_buffer[0] = c;
            escape_len = 1;
        } else {
            add_char_to_buffer(c);
        }
    }
}

/* Render screen to display */
static void render_screen(void) {
    weact_clear_buffer(&term_state.display, WEACT_BLACK);
    
    /* Calculate which lines to show based on scroll mode */
    int display_start_line;
    
    if (term_state.scroll_mode) {
        /* Scroll mode - show from view_offset */
        display_start_line = term_state.cursor_y - term_state.term_rows + 1 - term_state.view_offset;
        if (display_start_line < 0) display_start_line = 0;
    } else {
        /* Normal mode - show last lines */
        if (term_state.cursor_y < term_state.term_rows) {
            display_start_line = 0;
        } else {
            display_start_line = term_state.cursor_y - term_state.term_rows + 1;
        }
    }
    
    int visible_lines = term_state.term_rows;
    int y = 2;  /* Small top margin */
    
    for (int row = 0; row < visible_lines; row++) {
        int line_idx = display_start_line + row;
        
        if (line_idx >= 0 && line_idx <= term_state.cursor_y && line_idx < SCROLLBACK_LINES) {
            char *line = term_state.screen_buffer[line_idx];
            if (line[0] != '\0') {
                if (term_state.verbose && !term_state.scroll_mode) {
                    fprintf(stderr, "Drawing line %d at y=%d: '%s'\n", line_idx, y, line);
                }
                ft_text_draw(term_state.text_ctx, 2, y, line);
            }
        }
        
        y += term_state.char_height;
    }
    
    /* Draw cursor only in normal mode (not when scrolling) */
    if (!term_state.scroll_mode) {
        int cursor_screen_row = (term_state.cursor_y < term_state.term_rows) ? 
                                term_state.cursor_y : term_state.term_rows - 1;
        
        if (cursor_screen_row >= 0 && cursor_screen_row < term_state.term_rows) {
            int cursor_pixel_x = 2 + (term_state.cursor_x * term_state.char_width);
            int cursor_pixel_y = 2 + (cursor_screen_row * term_state.char_height);
            
            /* Draw cursor as filled rectangle */
            weact_draw_rect(&term_state.display, cursor_pixel_x, cursor_pixel_y, 
                           term_state.char_width, term_state.char_height, 
                           WEACT_GREEN, true);
        }
    } else {
        /* In scroll mode - show indicator in top-right corner */
        /* Draw "SCROLL" text or indicator */
        weact_draw_rect(&term_state.display, DISPLAY_WIDTH - 10, 2, 8, 6, 
                       WEACT_YELLOW, true);
    }
    
    weact_update_display(&term_state.display);
}

/* Create PTY and spawn shell */
static bool create_pty_shell(void) {
    struct winsize ws = {
        .ws_row = term_state.term_rows,
        .ws_col = term_state.term_cols,
        .ws_xpixel = DISPLAY_WIDTH,
        .ws_ypixel = DISPLAY_HEIGHT
    };
    
    term_state.child_pid = forkpty(&term_state.master_fd, NULL, NULL, &ws);
    
    if (term_state.child_pid < 0) {
        perror("forkpty");
        return false;
    }
    
    if (term_state.child_pid == 0) {
        /* Child process - execute shell */
        setenv("TERM", "vt100", 1);
        setenv("PS1", "\\$ ", 1);
        
        char *shell = getenv("SHELL");
        if (!shell) shell = "/bin/bash";
        
        execlp(shell, shell, NULL);
        perror("execlp");
        exit(1);
    }
    
    /* Parent process */
    if (term_state.verbose) {
        fprintf(stderr, "PTY shell started (PID: %d)\n", term_state.child_pid);
    }
    
    return true;
}

/* Main terminal loop */
static void terminal_loop(void) {
    char pty_buffer[4096];
    char kbd_buffer[256];
    
    fd_set readfds;
    struct timeval timeout;
    
    term_state.running = true;
    
    /* Initial render */
    render_screen();
    
    while (term_state.running) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(term_state.master_fd, &readfds);
        
        int max_fd = (term_state.master_fd > STDIN_FILENO) ? 
                     term_state.master_fd : STDIN_FILENO;
        
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; /* 100ms */
        
        int ret = select(max_fd + 1, &readfds, NULL, NULL, &timeout);
        
        if (ret < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }
        
        /* Read from keyboard */
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            ssize_t n = read(STDIN_FILENO, kbd_buffer, sizeof(kbd_buffer));
            if (n > 0) {
                /* Check for special keys */
                bool handled = false;
                
                /* Check for Ctrl+C */
                if (kbd_buffer[0] == 3) { /* Ctrl+C */
                    term_state.running = false;
                    break;
                }
                
                /* Check for escape sequences (arrow keys, Page Up/Down, etc) */
                if (n >= 3 && kbd_buffer[0] == 27 && kbd_buffer[1] == '[') {
                    /* ESC [ sequences */
                    
                    /* Page Up: ESC[5~ */
                    if (n >= 4 && kbd_buffer[2] == '5' && kbd_buffer[3] == '~') {
                        if (!term_state.scroll_mode) {
                            term_state.scroll_mode = true;
                            term_state.view_offset = 0;
                        }
                        /* Scroll up one page */
                        term_state.view_offset += term_state.term_rows;
                        if (term_state.view_offset > term_state.cursor_y) {
                            term_state.view_offset = term_state.cursor_y;
                        }
                        render_screen();
                        handled = true;
                    }
                    /* Page Down: ESC[6~ */
                    else if (n >= 4 && kbd_buffer[2] == '6' && kbd_buffer[3] == '~') {
                        if (term_state.scroll_mode) {
                            /* Scroll down one page */
                            term_state.view_offset -= term_state.term_rows;
                            if (term_state.view_offset < 0) {
                                term_state.view_offset = 0;
                                term_state.scroll_mode = false;  /* Exit scroll mode at bottom */
                            }
                            render_screen();
                            handled = true;
                        }
                    }
                }
                /* ScrollLock (sent as Ctrl+Q or special sequence depending on terminal) */
                /* Also handle 'q' in scroll mode to exit */
                else if (kbd_buffer[0] == 'q' && term_state.scroll_mode) {
                    /* Exit scroll mode */
                    term_state.scroll_mode = false;
                    term_state.view_offset = 0;
                    render_screen();
                    handled = true;
                }
                
                /* If not in scroll mode, send to PTY */
                if (!handled && !term_state.scroll_mode) {
                    write(term_state.master_fd, kbd_buffer, n);
                }
            }
        }
        
        /* Read from PTY */
        if (FD_ISSET(term_state.master_fd, &readfds)) {
            ssize_t n = read(term_state.master_fd, pty_buffer, sizeof(pty_buffer));
            if (n > 0) {
                process_pty_output(pty_buffer, n);
                render_screen();
            } else if (n == 0) {
                /* PTY closed */
                if (term_state.verbose) {
                    fprintf(stderr, "\nShell exited\n");
                }
                break;
            }
        }
    }
}

/* Show help */
static void show_help(const char *prog_name) {
    printf("WeActTerm - Terminal Emulator for WeAct Display\n");
    printf("\n");
    printf("USAGE:\n");
    printf("  %s -p /dev/ttyACM0 [options]\n", prog_name);
    printf("\n");
    printf("OPTIONS:\n");
    printf("  -p, --port PORT    Serial port for display (required)\n");
    printf("  -f, --font TYPE    Font type: mono, sans, serif (default: mono)\n");
    printf("                     mono  = Monospace (best for terminal, fixed width)\n");
    printf("                     sans  = Sans-serif (proportional width)\n");
    printf("                     serif = Serif (proportional width)\n");
    printf("  -z, --size SIZE    Font size: 8, 10, 12 (default: 10)\n");
    printf("                     8  = ~26 cols × 8 rows  (small, more text)\n");
    printf("                     10 = ~25 cols × 7 rows  (medium, readable)\n");
    printf("                     12 = ~22 cols × 6 rows  (large, very readable)\n");
    printf("  -l, --flip         Flip display 180° (reverse landscape)\n");
    printf("                     Useful if display is mounted upside-down\n");
    printf("  -v, --verbose      Verbose output\n");
    printf("  -h, --help         Show this help\n");
    printf("\n");
    printf("CONTROLS:\n");
    printf("  Ctrl+C             Exit weactterm\n");
    printf("  Page Up            Scroll back (enter scroll mode)\n");
    printf("  Page Down          Scroll forward\n");
    printf("  q (in scroll mode) Exit scroll mode\n");
    printf("\n");
    printf("FEATURES:\n");
    printf("  - Monospace font for perfect alignment (default)\n");
    printf("  - Scroll back through output with Page Up/Down\n");
    printf("  - Yellow indicator when in scroll mode\n");
    printf("  - Up to 100 lines of scrollback buffer\n");
    printf("  - Flip mode for upside-down mounting\n");
    printf("\n");
    printf("RECOMMENDED:\n");
    printf("  %s -p /dev/ttyACM0 -f mono -z 12\n", prog_name);
    printf("  (Monospace 12pt - best readability and alignment)\n");
    printf("\n");
    printf("EXAMPLES:\n");
    printf("  %s -p /dev/ttyACM0              # Default: mono 10pt\n", prog_name);
    printf("  %s -p /dev/ttyACM0 -z 12        # Larger font\n", prog_name);
    printf("  %s -p /dev/ttyACM0 -f mono -z 12  # Best settings\n", prog_name);
    printf("  %s -p /dev/ttyACM0 --flip       # Upside-down mount\n", prog_name);
    printf("  %s -p /dev/ttyUSB0 -v           # Verbose mode\n", prog_name);
    printf("\n");
    printf("NOTES:\n");
    printf("  - Use monospace font for proper cursor alignment\n");
    printf("  - Best for simple commands: ip, ls, ps, systemctl status\n");
    printf("  - Not suitable for: vim, nano, complex TUI apps\n");
    printf("  - Press Page Up to scroll back through output\n");
    printf("  - Use --flip if your display is mounted upside-down\n");
    printf("\n");
}

/* Cleanup */
static void cleanup(void) {
    if (term_state.text_ctx) {
        ft_text_cleanup(term_state.text_ctx);
    }
    
    if (term_state.master_fd > 0) {
        close(term_state.master_fd);
    }
    
    weact_cleanup(&term_state.display);
}

/* Main */
int main(int argc, char *argv[]) {
    bool port_specified = false;
    
    /* Set defaults */
    term_state.font_size = FONT_MEDIUM;  /* 10pt by default */
    term_state.font_type = FONT_MONO;    /* Monospace by default - best for terminal! */
    term_state.scroll_mode = false;
    term_state.view_offset = 0;
    term_state.flip_mode = false;        /* Normal orientation by default */
    
    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0) {
            if (i + 1 < argc) {
                strncpy(term_state.port, argv[++i], sizeof(term_state.port) - 1);
                port_specified = true;
            }
        } else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--font") == 0) {
            if (i + 1 < argc) {
                i++;
                if (strcasecmp(argv[i], "mono") == 0 || strcasecmp(argv[i], "monospace") == 0) {
                    term_state.font_type = FONT_MONO;
                } else if (strcasecmp(argv[i], "sans") == 0) {
                    term_state.font_type = FONT_SANS;
                } else if (strcasecmp(argv[i], "serif") == 0) {
                    term_state.font_type = FONT_SERIF;
                } else {
                    fprintf(stderr, "Warning: Unknown font type '%s', using mono\n", argv[i]);
                    term_state.font_type = FONT_MONO;
                }
            }
        } else if (strcmp(argv[i], "-z") == 0 || strcmp(argv[i], "--size") == 0) {
            if (i + 1 < argc) {
                int size = atoi(argv[++i]);
                if (size == 8) {
                    term_state.font_size = FONT_SMALL;
                } else if (size == 10) {
                    term_state.font_size = FONT_MEDIUM;
                } else if (size == 12) {
                    term_state.font_size = FONT_LARGE;
                } else {
                    fprintf(stderr, "Warning: Invalid font size %d, using default (10)\n", size);
                    term_state.font_size = FONT_MEDIUM;
                }
            }
        } else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--flip") == 0) {
            term_state.flip_mode = true;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            term_state.verbose = true;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            show_help(argv[0]);
            return 0;
        }
    }
    
    if (!port_specified) {
        fprintf(stderr, "Error: Port not specified\n\n");
        show_help(argv[0]);
        return 1;
    }
    
    /* Find font file */
    const char *font_path = find_font_path(term_state.font_type);
    if (!font_path) {
        fprintf(stderr, "Error: Could not find suitable font\n");
        fprintf(stderr, "Please install fonts-dejavu or fonts-liberation:\n");
        fprintf(stderr, "  sudo apt-get install fonts-dejavu fonts-liberation\n");
        return 1;
    }
    strncpy(term_state.font_path, font_path, sizeof(term_state.font_path) - 1);
    
    /* Calculate terminal dimensions based on font size and type */
    calculate_terminal_dimensions();
    
    /* Setup signal handlers */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    if (term_state.verbose) {
        fprintf(stderr, "WeActTerm starting...\n");
        fprintf(stderr, "Font: %s (%dpt)\n", 
                term_state.font_type == FONT_MONO ? "Monospace" :
                term_state.font_type == FONT_SANS ? "Sans" : "Serif",
                term_state.font_size);
        fprintf(stderr, "Font file: %s\n", term_state.font_path);
        fprintf(stderr, "Display: %dx%d chars on %s\n", 
                term_state.term_cols, term_state.term_rows, term_state.port);
        if (term_state.flip_mode) {
            fprintf(stderr, "Flip mode: enabled (reverse landscape)\n");
        }
    }
    
    /* Initialize display */
    if (!weact_init(&term_state.display, term_state.port)) {
        fprintf(stderr, "Error: Failed to initialize display\n");
        fprintf(stderr, "Details: %s\n", weact_get_last_error(&term_state.display));
        return 1;
    }
    
    if (term_state.verbose) {
        fprintf(stderr, "Display initialized: %s\n", term_state.port);
    }
    
    /* Apply flip mode if requested */
    if (term_state.flip_mode) {
        if (term_state.verbose) {
            fprintf(stderr, "Setting reverse landscape orientation (flip mode)\n");
        }
        if (!weact_set_orientation(&term_state.display, WEACT_REVERSE_LANDSCAPE)) {
            fprintf(stderr, "Warning: Failed to set flip orientation\n");
        }
    }
    
    /* Initialize text renderer with selected font */
    term_state.text_ctx = ft_text_init(&term_state.display, term_state.font_path, 
                                       term_state.font_size);
    if (!term_state.text_ctx) {
        fprintf(stderr, "Error: Failed to initialize text renderer\n");
        fprintf(stderr, "Font: %s\n", term_state.font_path);
        weact_close(&term_state.display);
        return 1;
    }
    
    ft_text_set_color(term_state.text_ctx, WEACT_WHITE);
    
    /* Initialize screen buffer */
    init_screen_buffer();
    
    /* Set terminal to raw mode */
    if (!set_raw_mode()) {
        fprintf(stderr, "Error: Failed to set raw mode\n");
        cleanup();
        return 1;
    }
    
    /* Create PTY and shell */
    if (!create_pty_shell()) {
        fprintf(stderr, "Error: Failed to create PTY shell\n");
        cleanup();
        return 1;
    }
    
    /* Show initial message on display */
    weact_clear_buffer(&term_state.display, WEACT_BLACK);
    
    char welcome[64];
    const char *font_name = term_state.font_type == FONT_MONO ? "Mono" :
                            term_state.font_type == FONT_SANS ? "Sans" : "Serif";
    snprintf(welcome, sizeof(welcome), "%s %dpt%s", 
             font_name, term_state.font_size,
             term_state.flip_mode ? " flip" : "");
    ft_text_draw(term_state.text_ctx, 2, 5, welcome);
    
    snprintf(welcome, sizeof(welcome), "%dx%d chars", 
             term_state.term_cols, term_state.term_rows);
    ft_text_draw(term_state.text_ctx, 2, 5 + term_state.char_height, welcome);
    
    ft_text_draw(term_state.text_ctx, 2, 5 + term_state.char_height * 3, "PgUp=Scroll");
    ft_text_draw(term_state.text_ctx, 2, 5 + term_state.char_height * 4, "Ctrl+C=Exit");
    weact_update_display(&term_state.display);
    sleep(1);
    
    /* Run terminal loop */
    terminal_loop();
    
    /* Cleanup */
    cleanup();
    
    return 0;
}
