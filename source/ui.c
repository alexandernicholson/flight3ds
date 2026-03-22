#include "ui.h"
#include "geo.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

// Simple 5x7 font for UI text
static const u8 font5x7[][7] = {
    // Space (32)
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    // A-Z (65-90) - simplified bitmaps
};

// We'll use a direct pixel approach for text rendering on bottom screen
// Simple 4x6 bitmap font covering ASCII 32-127

static void fb_pixel_b(u8 *fb, int fb_w, int fb_h, int x, int y,
                       u8 r, u8 g, u8 b) {
    if (x < 0 || x >= fb_w || y < 0 || y >= fb_h) return;
    int idx = (x * fb_h + (fb_h - 1 - y)) * 3;
    fb[idx]     = b;
    fb[idx + 1] = g;
    fb[idx + 2] = r;
}

static void draw_hline(u8 *fb, int fb_w, int fb_h,
                       int x, int y, int w, u8 r, u8 g, u8 b) {
    for (int i = 0; i < w; i++)
        fb_pixel_b(fb, fb_w, fb_h, x + i, y, r, g, b);
}

static void draw_fill_rect(u8 *fb, int fb_w, int fb_h,
                           int x, int y, int w, int h,
                           u8 r, u8 g, u8 b) {
    for (int dy = 0; dy < h; dy++)
        for (int dx = 0; dx < w; dx++)
            fb_pixel_b(fb, fb_w, fb_h, x + dx, y + dy, r, g, b);
}

// Minimal text rendering using console-style approach
// We render text by writing individual character bitmaps
// For simplicity, use a 3x5 ultra-compact font

static const u8 font3x5[] = {
    // Each char is 3 columns, 5 rows packed into 2 bytes (15 bits)
    // Format: row0[2:0], row1[2:0], row2[2:0], row3[2:0], row4[2:0]
    // We store as 5 bytes per char, each byte is the 3-bit row
    // Space
    0,0,0,0,0,
    // ! (33)
    2,2,2,0,2,
    // " - skip to useful chars
};

// Instead, use a straightforward approach: render chars from a hardcoded table
// covering 0-9, A-Z, a-z, and common punctuation.
// Each char: 5 rows of 4-bit wide patterns

typedef struct { u8 rows[7]; u8 width; } Glyph;

static const Glyph glyphs[128] = {
    // 0-31: control chars (empty)
    [0 ... 31] = {{.rows={0},.width=4}},
    // 32: space
    [' '] = {{0,0,0,0,0,0,0}, 3},
    // 33-47: punctuation
    ['!'] = {{0x4,0x4,0x4,0x4,0x0,0x4,0}, 4},
    ['"'] = {{0xA,0xA,0,0,0,0,0}, 4},
    ['#'] = {{0xA,0xF,0xA,0xF,0xA,0,0}, 4},
    ['%'] = {{0x9,0x2,0x4,0x9,0,0,0}, 4},
    ['('] = {{0x2,0x4,0x4,0x4,0x2,0,0}, 3},
    [')'] = {{0x4,0x2,0x2,0x2,0x4,0,0}, 3},
    ['+'] = {{0,0x4,0xE,0x4,0,0,0}, 4},
    [','] = {{0,0,0,0,0x4,0x4,0x8}, 4},
    ['-'] = {{0,0,0xE,0,0,0,0}, 4},
    ['.'] = {{0,0,0,0,0x4,0,0}, 4},
    ['/'] = {{0x1,0x2,0x4,0x8,0,0,0}, 4},
    // 48-57: digits
    ['0'] = {{0x6,0x9,0x9,0x9,0x6,0,0}, 4},
    ['1'] = {{0x2,0x6,0x2,0x2,0x7,0,0}, 4},
    ['2'] = {{0x6,0x9,0x2,0x4,0xF,0,0}, 4},
    ['3'] = {{0x6,0x9,0x2,0x9,0x6,0,0}, 4},
    ['4'] = {{0x9,0x9,0xF,0x1,0x1,0,0}, 4},
    ['5'] = {{0xF,0x8,0xE,0x1,0xE,0,0}, 4},
    ['6'] = {{0x6,0x8,0xE,0x9,0x6,0,0}, 4},
    ['7'] = {{0xF,0x1,0x2,0x4,0x4,0,0}, 4},
    ['8'] = {{0x6,0x9,0x6,0x9,0x6,0,0}, 4},
    ['9'] = {{0x6,0x9,0x7,0x1,0x6,0,0}, 4},
    [':'] = {{0,0x4,0,0x4,0,0,0}, 4},
    // 65-90: uppercase
    ['A'] = {{0x6,0x9,0xF,0x9,0x9,0,0}, 4},
    ['B'] = {{0xE,0x9,0xE,0x9,0xE,0,0}, 4},
    ['C'] = {{0x6,0x9,0x8,0x9,0x6,0,0}, 4},
    ['D'] = {{0xE,0x9,0x9,0x9,0xE,0,0}, 4},
    ['E'] = {{0xF,0x8,0xE,0x8,0xF,0,0}, 4},
    ['F'] = {{0xF,0x8,0xE,0x8,0x8,0,0}, 4},
    ['G'] = {{0x7,0x8,0xB,0x9,0x7,0,0}, 4},
    ['H'] = {{0x9,0x9,0xF,0x9,0x9,0,0}, 4},
    ['I'] = {{0xE,0x4,0x4,0x4,0xE,0,0}, 4},
    ['J'] = {{0x1,0x1,0x1,0x9,0x6,0,0}, 4},
    ['K'] = {{0x9,0xA,0xC,0xA,0x9,0,0}, 4},
    ['L'] = {{0x8,0x8,0x8,0x8,0xF,0,0}, 4},
    ['M'] = {{0x9,0xF,0xF,0x9,0x9,0,0}, 4},
    ['N'] = {{0x9,0xD,0xB,0x9,0x9,0,0}, 4},
    ['O'] = {{0x6,0x9,0x9,0x9,0x6,0,0}, 4},
    ['P'] = {{0xE,0x9,0xE,0x8,0x8,0,0}, 4},
    ['Q'] = {{0x6,0x9,0x9,0xB,0x7,0,0}, 4},
    ['R'] = {{0xE,0x9,0xE,0xA,0x9,0,0}, 4},
    ['S'] = {{0x7,0x8,0x6,0x1,0xE,0,0}, 4},
    ['T'] = {{0xE,0x4,0x4,0x4,0x4,0,0}, 4},
    ['U'] = {{0x9,0x9,0x9,0x9,0x6,0,0}, 4},
    ['V'] = {{0x9,0x9,0x9,0x6,0x6,0,0}, 4},
    ['W'] = {{0x9,0x9,0xF,0xF,0x9,0,0}, 4},
    ['X'] = {{0x9,0x9,0x6,0x9,0x9,0,0}, 4},
    ['Y'] = {{0x9,0x9,0x7,0x1,0x6,0,0}, 4},
    ['Z'] = {{0xF,0x2,0x4,0x8,0xF,0,0}, 4},
    // 97-122: lowercase (reuse uppercase patterns)
    ['a'] = {{0,0x6,0x9,0x9,0x7,0,0}, 4},
    ['b'] = {{0x8,0xE,0x9,0x9,0xE,0,0}, 4},
    ['c'] = {{0,0x7,0x8,0x8,0x7,0,0}, 4},
    ['d'] = {{0x1,0x7,0x9,0x9,0x7,0,0}, 4},
    ['e'] = {{0,0x6,0xF,0x8,0x7,0,0}, 4},
    ['f'] = {{0x3,0x4,0xE,0x4,0x4,0,0}, 4},
    ['g'] = {{0,0x7,0x9,0x7,0x1,0x6}, 4},
    ['h'] = {{0x8,0xE,0x9,0x9,0x9,0,0}, 4},
    ['i'] = {{0x4,0x0,0x4,0x4,0x4,0,0}, 4},
    ['j'] = {{0x2,0x0,0x2,0x2,0xA,0x4}, 4},
    ['k'] = {{0x8,0xA,0xC,0xA,0x9,0,0}, 4},
    ['l'] = {{0x4,0x4,0x4,0x4,0x2,0,0}, 4},
    ['m'] = {{0,0xF,0xF,0x9,0x9,0,0}, 4},
    ['n'] = {{0,0xE,0x9,0x9,0x9,0,0}, 4},
    ['o'] = {{0,0x6,0x9,0x9,0x6,0,0}, 4},
    ['p'] = {{0,0xE,0x9,0xE,0x8,0x8}, 4},
    ['q'] = {{0,0x7,0x9,0x7,0x1,0x1}, 4},
    ['r'] = {{0,0xB,0xC,0x8,0x8,0,0}, 4},
    ['s'] = {{0,0x7,0xC,0x3,0xE,0,0}, 4},
    ['t'] = {{0x4,0xE,0x4,0x4,0x3,0,0}, 4},
    ['u'] = {{0,0x9,0x9,0x9,0x7,0,0}, 4},
    ['v'] = {{0,0x9,0x9,0x6,0x6,0,0}, 4},
    ['w'] = {{0,0x9,0xF,0xF,0x9,0,0}, 4},
    ['x'] = {{0,0x9,0x6,0x6,0x9,0,0}, 4},
    ['y'] = {{0,0x9,0x9,0x7,0x1,0x6}, 4},
    ['z'] = {{0,0xF,0x2,0x4,0xF,0,0}, 4},
};

static void draw_text(u8 *fb, int fb_w, int fb_h,
                      int x, int y, const char *text,
                      u8 r, u8 g, u8 b) {
    int cx = x;
    for (int i = 0; text[i]; i++) {
        unsigned char c = (unsigned char)text[i];
        if (c > 127) c = '?';
        const Glyph *gl = &glyphs[c];
        for (int row = 0; row < 7; row++) {
            u8 bits = gl->rows[row];
            for (int col = 0; col < 4; col++) {
                if (bits & (0x8 >> col))
                    fb_pixel_b(fb, fb_w, fb_h, cx + col, y + row, r, g, b);
            }
        }
        cx += gl->width + 1;
    }
}

void ui_init(UIState *ui) {
    ui->view = UI_VIEW_MAP;
    ui->list_scroll = 0;
    ui->list_selected = 0;
    ui->show_status = true;
    ui->status_timer = 0;
}

void ui_render_bottom(u8 *fb, const UIState *ui, const FlightDB *db,
                      const MapState *ms, bool online, uint32_t cache_age_s) {
    // Clear bottom screen
    memset(fb, 0, BOTTOM_W * BOTTOM_H * 3);

    if (ui->view == UI_VIEW_HELP) {
        draw_text(fb, BOTTOM_W, BOTTOM_H, 10, 10, "FLIGHT3DS CONTROLS", 255, 255, 100);
        draw_hline(fb, BOTTOM_W, BOTTOM_H, 10, 20, 150, 100, 100, 100);

        draw_text(fb, BOTTOM_W, BOTTOM_H, 10, 30,  "D-Pad   Pan map", 200, 200, 200);
        draw_text(fb, BOTTOM_W, BOTTOM_H, 10, 42,  "L/R     Zoom in/out", 200, 200, 200);
        draw_text(fb, BOTTOM_W, BOTTOM_H, 10, 54,  "A       Select nearest", 200, 200, 200);
        draw_text(fb, BOTTOM_W, BOTTOM_H, 10, 66,  "B       Deselect", 200, 200, 200);
        draw_text(fb, BOTTOM_W, BOTTOM_H, 10, 78,  "X       Toggle cursor", 200, 200, 200);
        draw_text(fb, BOTTOM_W, BOTTOM_H, 10, 90,  "Y       Force refresh", 200, 200, 200);
        draw_text(fb, BOTTOM_W, BOTTOM_H, 10, 102, "START   Help", 200, 200, 200);
        draw_text(fb, BOTTOM_W, BOTTOM_H, 10, 114, "SELECT  Quit", 200, 200, 200);
        draw_text(fb, BOTTOM_W, BOTTOM_H, 10, 134, "Circle  Move cursor", 200, 200, 200);

        draw_text(fb, BOTTOM_W, BOTTOM_H, 10, 160, "COLOR KEY:", 255, 255, 100);
        draw_fill_rect(fb, BOTTOM_W, BOTTOM_H, 10, 174, 6, 6, 255, 200, 50);
        draw_text(fb, BOTTOM_W, BOTTOM_H, 20, 172, "High alt (10000m+)", 200, 200, 200);
        draw_fill_rect(fb, BOTTOM_W, BOTTOM_H, 10, 186, 6, 6, 255, 150, 50);
        draw_text(fb, BOTTOM_W, BOTTOM_H, 20, 184, "Mid alt (3000m+)", 200, 200, 200);
        draw_fill_rect(fb, BOTTOM_W, BOTTOM_H, 10, 198, 6, 6, 255, 80, 50);
        draw_text(fb, BOTTOM_W, BOTTOM_H, 20, 196, "Low alt", 200, 200, 200);
        draw_fill_rect(fb, BOTTOM_W, BOTTOM_H, 10, 210, 6, 6, 80, 80, 80);
        draw_text(fb, BOTTOM_W, BOTTOM_H, 20, 208, "On ground", 200, 200, 200);

        draw_text(fb, BOTTOM_W, BOTTOM_H, 10, 228, "Press START to close", 150, 150, 150);
        return;
    }

    if (ui->view == UI_VIEW_DETAIL && ms->selected >= 0) {
        const Flight *f = flights_get((FlightDB*)db, ms->selected);
        if (f) {
            // Flight detail view
            draw_text(fb, BOTTOM_W, BOTTOM_H, 10, 8, "FLIGHT DETAIL", 255, 255, 100);
            draw_hline(fb, BOTTOM_W, BOTTOM_H, 10, 18, 150, 100, 100, 100);

            char buf[64];

            draw_text(fb, BOTTOM_W, BOTTOM_H, 10, 28, "Callsign:", 150, 150, 150);
            draw_text(fb, BOTTOM_W, BOTTOM_H, 80, 28, f->callsign[0] ? f->callsign : "N/A",
                     255, 255, 255);

            draw_text(fb, BOTTOM_W, BOTTOM_H, 10, 42, "ICAO:", 150, 150, 150);
            draw_text(fb, BOTTOM_W, BOTTOM_H, 80, 42, f->icao24, 255, 255, 255);

            draw_text(fb, BOTTOM_W, BOTTOM_H, 10, 56, "Country:", 150, 150, 150);
            draw_text(fb, BOTTOM_W, BOTTOM_H, 80, 56, f->origin_country, 255, 255, 255);

            snprintf(buf, sizeof(buf), "%.0f ft", geo_m_to_ft(f->altitude));
            draw_text(fb, BOTTOM_W, BOTTOM_H, 10, 76, "Altitude:", 150, 150, 150);
            draw_text(fb, BOTTOM_W, BOTTOM_H, 80, 76, buf, 255, 255, 255);

            snprintf(buf, sizeof(buf), "%.0f kts", geo_ms_to_kts(f->velocity));
            draw_text(fb, BOTTOM_W, BOTTOM_H, 10, 90, "Speed:", 150, 150, 150);
            draw_text(fb, BOTTOM_W, BOTTOM_H, 80, 90, buf, 255, 255, 255);

            snprintf(buf, sizeof(buf), "%.0f deg", f->heading);
            draw_text(fb, BOTTOM_W, BOTTOM_H, 10, 104, "Heading:", 150, 150, 150);
            draw_text(fb, BOTTOM_W, BOTTOM_H, 80, 104, buf, 255, 255, 255);

            snprintf(buf, sizeof(buf), "%.0f ft/m",
                     f->vertical_rate * 196.85f); // m/s to ft/min
            draw_text(fb, BOTTOM_W, BOTTOM_H, 10, 118, "V/S:", 150, 150, 150);
            draw_text(fb, BOTTOM_W, BOTTOM_H, 80, 118, buf, 255, 255, 255);

            snprintf(buf, sizeof(buf), "%.4f", f->latitude);
            draw_text(fb, BOTTOM_W, BOTTOM_H, 10, 138, "Lat:", 150, 150, 150);
            draw_text(fb, BOTTOM_W, BOTTOM_H, 80, 138, buf, 255, 255, 255);

            snprintf(buf, sizeof(buf), "%.4f", f->longitude);
            draw_text(fb, BOTTOM_W, BOTTOM_H, 10, 152, "Lon:", 150, 150, 150);
            draw_text(fb, BOTTOM_W, BOTTOM_H, 80, 152, buf, 255, 255, 255);

            draw_text(fb, BOTTOM_W, BOTTOM_H, 10, 172, "Status:", 150, 150, 150);
            draw_text(fb, BOTTOM_W, BOTTOM_H, 80, 172,
                     f->on_ground ? "On Ground" : "Airborne",
                     f->on_ground ? 150 : 100,
                     f->on_ground ? 150 : 255,
                     f->on_ground ? 150 : 100);

            draw_text(fb, BOTTOM_W, BOTTOM_H, 10, 228, "B: back  D-Pad: browse",
                     120, 120, 120);
            return;
        }
    }

    // Default: flight list view
    draw_text(fb, BOTTOM_W, BOTTOM_H, 10, 4, "FLIGHTS", 255, 255, 100);

    char count_buf[32];
    snprintf(count_buf, sizeof(count_buf), "%d tracked", db->count);
    draw_text(fb, BOTTOM_W, BOTTOM_H, 70, 4, count_buf, 150, 150, 150);

    // Status indicator
    if (online) {
        draw_fill_rect(fb, BOTTOM_W, BOTTOM_H, BOTTOM_W - 30, 4, 6, 6, 50, 255, 50);
        draw_text(fb, BOTTOM_W, BOTTOM_H, BOTTOM_W - 22, 4, "ON", 50, 255, 50);
    } else {
        draw_fill_rect(fb, BOTTOM_W, BOTTOM_H, BOTTOM_W - 40, 4, 6, 6, 255, 100, 50);
        draw_text(fb, BOTTOM_W, BOTTOM_H, BOTTOM_W - 32, 4, "OFF", 255, 100, 50);
    }

    draw_hline(fb, BOTTOM_W, BOTTOM_H, 5, 14, BOTTOM_W - 10, 60, 60, 60);

    // Flight list
    int y = 18;
    int row_h = 14;
    int visible_rows = (BOTTOM_H - 36) / row_h;
    int n = 0;
    int drawn = 0;

    for (int i = 0; i < MAX_FLIGHTS && drawn < visible_rows; i++) {
        const Flight *f = &db->flights[i];
        if (!f->valid) continue;
        if (n < ui->list_scroll) { n++; continue; }

        bool selected = (n == ui->list_selected);

        if (selected) {
            draw_fill_rect(fb, BOTTOM_W, BOTTOM_H, 5, y, BOTTOM_W - 10, row_h,
                          30, 50, 80);
        }

        // Callsign
        u8 tr = selected ? 0 : 200;
        u8 tg = selected ? 255 : 200;
        u8 tb = selected ? 255 : 200;
        draw_text(fb, BOTTOM_W, BOTTOM_H, 10, y + 3,
                 f->callsign[0] ? f->callsign : f->icao24, tr, tg, tb);

        // Country
        draw_text(fb, BOTTOM_W, BOTTOM_H, 70, y + 3,
                 f->origin_country, 150, 150, 150);

        // Altitude
        char alt_buf[16];
        snprintf(alt_buf, sizeof(alt_buf), "%.0fft", geo_m_to_ft(f->altitude));
        draw_text(fb, BOTTOM_W, BOTTOM_H, 200, y + 3, alt_buf, 180, 180, 100);

        // Speed
        char spd_buf[16];
        snprintf(spd_buf, sizeof(spd_buf), "%.0fkt", geo_ms_to_kts(f->velocity));
        draw_text(fb, BOTTOM_W, BOTTOM_H, 260, y + 3, spd_buf, 180, 180, 100);

        y += row_h;
        n++;
        drawn++;
    }

    // Bottom bar
    draw_hline(fb, BOTTOM_W, BOTTOM_H, 5, BOTTOM_H - 14, BOTTOM_W - 10, 60, 60, 60);
    draw_text(fb, BOTTOM_W, BOTTOM_H, 10, BOTTOM_H - 10,
             "A:detail  START:help  SELECT:quit", 120, 120, 120);
}

void ui_draw_status_bar(u8 *fb, int fb_w, int fb_h, const FlightDB *db,
                        bool online, uint32_t cache_age_s) {
    // Semi-transparent status bar on top screen
    int bar_y = fb_h - 12;

    // Darken bar area
    for (int x = 0; x < fb_w; x++) {
        for (int y = bar_y; y < fb_h; y++) {
            int idx = (x * fb_h + (fb_h - 1 - y)) * 3;
            fb[idx]     = fb[idx] / 3;
            fb[idx + 1] = fb[idx + 1] / 3;
            fb[idx + 2] = fb[idx + 2] / 3;
        }
    }

    char buf[64];
    snprintf(buf, sizeof(buf), "%d flights  %s",
             db->count, online ? "LIVE" : "CACHED");

    u8 r = online ? 50 : 255;
    u8 g = online ? 255 : 150;
    u8 b = online ? 50 : 50;

    draw_text(fb, fb_w, fb_h, 4, bar_y + 2, buf, r, g, b);

    if (!online && cache_age_s > 0) {
        snprintf(buf, sizeof(buf), "Age: %us", cache_age_s);
        draw_text(fb, fb_w, fb_h, fb_w - 80, bar_y + 2, buf, 200, 150, 50);
    }
}

void ui_list_scroll_up(UIState *ui) {
    if (ui->list_selected > 0) ui->list_selected--;
    if (ui->list_selected < ui->list_scroll) ui->list_scroll = ui->list_selected;
}

void ui_list_scroll_down(UIState *ui, int max) {
    if (ui->list_selected < max - 1) ui->list_selected++;
    int visible = (BOTTOM_H - 36) / 14;
    if (ui->list_selected >= ui->list_scroll + visible)
        ui->list_scroll = ui->list_selected - visible + 1;
}

void ui_list_select(UIState *ui, int index) {
    ui->list_selected = index;
}
