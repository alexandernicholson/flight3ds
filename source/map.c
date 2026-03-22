#include "map.h"
#include "geo.h"
#include "coastlines.h"
#include "airports.h"
#include <string.h>
#include <math.h>

// 3DS framebuffer is column-major: pixel at (x,y) = fb[(x * fb_h + (fb_h - 1 - y)) * 3]
// Format: BGR8

static inline void fb_pixel(u8 *fb, int fb_w, int fb_h, int x, int y,
                            u8 r, u8 g, u8 b) {
    if (x < 0 || x >= fb_w || y < 0 || y >= fb_h) return;
    int idx = (x * fb_h + (fb_h - 1 - y)) * 3;
    fb[idx]     = b;
    fb[idx + 1] = g;
    fb[idx + 2] = r;
}

static void draw_line(u8 *fb, int fb_w, int fb_h,
                      int x0, int y0, int x1, int y1,
                      u8 r, u8 g, u8 b) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    for (int i = 0; i < 1000; i++) { // safety limit
        fb_pixel(fb, fb_w, fb_h, x0, y0, r, g, b);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx)  { err += dx; y0 += sy; }
    }
}

static void draw_rect(u8 *fb, int fb_w, int fb_h,
                      int x, int y, int w, int h,
                      u8 r, u8 g, u8 b) {
    for (int dy = 0; dy < h; dy++)
        for (int dx = 0; dx < w; dx++)
            fb_pixel(fb, fb_w, fb_h, x + dx, y + dy, r, g, b);
}

// Simple 4x6 digit font for map labels
static const u8 font4x6_digits[][6] = {
    {0x6,0x9,0x9,0x9,0x9,0x6}, // 0
    {0x2,0x6,0x2,0x2,0x2,0x7}, // 1
    {0x6,0x9,0x2,0x4,0x8,0xF}, // 2
    {0x6,0x9,0x2,0x1,0x9,0x6}, // 3
    {0x1,0x5,0x9,0xF,0x1,0x1}, // 4
    {0xF,0x8,0xE,0x1,0x9,0x6}, // 5
    {0x6,0x8,0xE,0x9,0x9,0x6}, // 6
    {0xF,0x1,0x2,0x4,0x4,0x4}, // 7
    {0x6,0x9,0x6,0x9,0x9,0x6}, // 8
    {0x6,0x9,0x7,0x1,0x1,0x6}, // 9
};

static void draw_char(u8 *fb, int fb_w, int fb_h, int cx, int cy,
                      char c, u8 r, u8 g, u8 b) {
    int idx = -1;
    if (c >= '0' && c <= '9') idx = c - '0';
    else if (c == '-') {
        // Draw a dash
        for (int dx = 0; dx < 4; dx++)
            fb_pixel(fb, fb_w, fb_h, cx + dx, cy + 3, r, g, b);
        return;
    }
    else return;

    if (idx < 0) return;
    for (int row = 0; row < 6; row++) {
        u8 bits = font4x6_digits[idx][row];
        for (int col = 0; col < 4; col++) {
            if (bits & (0x8 >> col))
                fb_pixel(fb, fb_w, fb_h, cx + col, cy + row, r, g, b);
        }
    }
}

static void draw_number(u8 *fb, int fb_w, int fb_h, int x, int y,
                        int num, u8 r, u8 g, u8 b) {
    char buf[12];
    int len = 0;
    if (num < 0) { buf[len++] = '-'; num = -num; }
    if (num == 0) { buf[len++] = '0'; }
    else {
        char tmp[10]; int tl = 0;
        while (num > 0) { tmp[tl++] = '0' + (num % 10); num /= 10; }
        for (int i = tl - 1; i >= 0; i--) buf[len++] = tmp[i];
    }
    for (int i = 0; i < len; i++)
        draw_char(fb, fb_w, fb_h, x + i * 5, y, buf[i], r, g, b);
}

void map_init(MapState *ms) {
    ms->center_lat = 20.0f;
    ms->center_lon = 0.0f;
    ms->zoom = ZOOM_DEFAULT;
    ms->selected = -1;
    ms->cursor_x = MAP_SCREEN_W / 2;
    ms->cursor_y = MAP_SCREEN_H / 2;
    ms->cursor_active = false;
    ms->show_airports = true;
}

void map_render(u8 *fb, int fb_w, int fb_h, const MapState *ms, const FlightDB *db) {
    // Clear to dark blue/black
    for (int x = 0; x < fb_w; x++) {
        for (int y = 0; y < fb_h; y++) {
            int idx = (x * fb_h + (fb_h - 1 - y)) * 3;
            fb[idx]     = 30;  // B
            fb[idx + 1] = 15;  // G
            fb[idx + 2] = 8;   // R
        }
    }

    // Draw grid lines
    for (float lat = -90; lat <= 90; lat += GRID_LAT_STEP) {
        int sy_prev = -1;
        for (int sx = 0; sx < fb_w; sx += 2) {
            float lon;
            float dummy_lat;
            geo_unproject(sx, 0, ms->center_lat, ms->center_lon,
                         ms->zoom, fb_w, fb_h, &dummy_lat, &lon);
            int screen_x, screen_y;
            geo_project(lat, lon, ms->center_lat, ms->center_lon,
                       ms->zoom, fb_w, fb_h, &screen_x, &screen_y);
            if (screen_y >= 0 && screen_y < fb_h)
                fb_pixel(fb, fb_w, fb_h, sx, screen_y, 20, 30, 50);
        }
    }
    for (float lon = -180; lon <= 180; lon += GRID_LON_STEP) {
        int sx, sy;
        geo_project(0, lon, ms->center_lat, ms->center_lon,
                   ms->zoom, fb_w, fb_h, &sx, &sy);
        if (sx >= 0 && sx < fb_w) {
            for (int y = 0; y < fb_h; y += 2)
                fb_pixel(fb, fb_w, fb_h, sx, y, 20, 30, 50);
        }
    }

    // Draw equator brighter
    {
        int sx, sy;
        geo_project(0, 0, ms->center_lat, ms->center_lon,
                   ms->zoom, fb_w, fb_h, &sx, &sy);
        if (sy >= 0 && sy < fb_h)
            for (int x = 0; x < fb_w; x++)
                fb_pixel(fb, fb_w, fb_h, x, sy, 30, 40, 60);
    }

    // Draw coastlines
    int prev_sx = -9999, prev_sy = -9999;
    bool segment_started = false;

    for (int i = 0; ; i++) {
        const CoastPoint *pt = &coastline_data[i];

        if (pt->lat >= 998.0f) {
            // End of segment or end of data
            segment_started = false;
            prev_sx = -9999;

            // Check if next is also end (= end of all data)
            if (coastline_data[i + 1].lat >= 998.0f) break;
            continue;
        }

        int sx, sy;
        geo_project(pt->lat, pt->lon, ms->center_lat, ms->center_lon,
                   ms->zoom, fb_w, fb_h, &sx, &sy);

        if (segment_started && prev_sx != -9999) {
            // Only draw if both points are reasonably near screen
            if ((sx > -200 && sx < fb_w + 200) || (prev_sx > -200 && prev_sx < fb_w + 200)) {
                draw_line(fb, fb_w, fb_h, prev_sx, prev_sy, sx, sy,
                         60, 120, 60); // dark green coastlines
            }
        }

        prev_sx = sx;
        prev_sy = sy;
        segment_started = true;
    }

    // Draw airports (only when zoomed in enough and enabled)
    if (ms->show_airports && ms->zoom >= 3.0f) {
        for (int i = 0; i < (int)NUM_AIRPORTS; i++) {
            int sx, sy;
            geo_project(airports[i].lat, airports[i].lon,
                       ms->center_lat, ms->center_lon,
                       ms->zoom, fb_w, fb_h, &sx, &sy);

            if (sx < -5 || sx >= fb_w + 5 || sy < -5 || sy >= fb_h + 5) continue;

            // Only show large airports at medium zoom, all at high zoom
            if (ms->zoom < 6.0f && airports[i].type != APT_LARGE) continue;

            if (airports[i].type == APT_LARGE) {
                // Diamond shape for major hubs
                fb_pixel(fb, fb_w, fb_h, sx, sy - 2, 100, 180, 255);
                fb_pixel(fb, fb_w, fb_h, sx - 1, sy - 1, 100, 180, 255);
                fb_pixel(fb, fb_w, fb_h, sx + 1, sy - 1, 100, 180, 255);
                fb_pixel(fb, fb_w, fb_h, sx - 2, sy, 100, 180, 255);
                fb_pixel(fb, fb_w, fb_h, sx + 2, sy, 100, 180, 255);
                fb_pixel(fb, fb_w, fb_h, sx - 1, sy + 1, 100, 180, 255);
                fb_pixel(fb, fb_w, fb_h, sx + 1, sy + 1, 100, 180, 255);
                fb_pixel(fb, fb_w, fb_h, sx, sy + 2, 100, 180, 255);
            } else {
                // Small square for regional
                fb_pixel(fb, fb_w, fb_h, sx, sy, 70, 130, 200);
                fb_pixel(fb, fb_w, fb_h, sx + 1, sy, 70, 130, 200);
                fb_pixel(fb, fb_w, fb_h, sx, sy + 1, 70, 130, 200);
                fb_pixel(fb, fb_w, fb_h, sx + 1, sy + 1, 70, 130, 200);
            }

            // Label at high zoom
            if (ms->zoom >= 8.0f) {
                const char *label = airports[i].icao;
                int lx = sx + 4;
                for (int c = 0; c < 4 && label[c]; c++)
                    draw_char(fb, fb_w, fb_h, lx + c * 5, sy - 3,
                             label[c], 80, 140, 200);
            }
        }
    }

    // Draw flights
    for (int i = 0; i < MAX_FLIGHTS; i++) {
        const Flight *f = &db->flights[i];
        if (!f->valid) continue;

        int sx, sy;
        geo_project(f->latitude, f->longitude, ms->center_lat, ms->center_lon,
                   ms->zoom, fb_w, fb_h, &sx, &sy);

        if (sx < -5 || sx >= fb_w + 5 || sy < -5 || sy >= fb_h + 5) continue;

        bool is_selected = (ms->selected >= 0 && flights_get((FlightDB*)db, ms->selected) == f);

        if (f->on_ground) {
            // Ground: dim gray dot
            fb_pixel(fb, fb_w, fb_h, sx, sy, 80, 80, 80);
        } else if (is_selected) {
            // Selected: bright cyan, larger
            draw_rect(fb, fb_w, fb_h, sx - 2, sy - 2, 5, 5, 0, 255, 255);

            // Heading indicator
            float rad = f->heading * DEG2RAD;
            int hx = sx + (int)(sinf(rad) * 8);
            int hy = sy - (int)(cosf(rad) * 8);
            draw_line(fb, fb_w, fb_h, sx, sy, hx, hy, 0, 255, 255);

            // Draw callsign near selected flight
            int tx = sx + 6;
            int ty = sy - 3;
            for (int c = 0; c < 8 && f->callsign[c]; c++) {
                char ch = f->callsign[c];
                if (ch >= '0' && ch <= '9')
                    draw_char(fb, fb_w, fb_h, tx + c * 5, ty, ch, 0, 255, 255);
            }
        } else {
            // Airborne: yellow/orange dot
            u8 r, g, b_col;
            if (f->altitude > 10000) { r = 255; g = 200; b_col = 50; }       // high: yellow
            else if (f->altitude > 3000) { r = 255; g = 150; b_col = 50; }   // mid: orange
            else { r = 255; g = 80; b_col = 50; }                             // low: red-orange

            fb_pixel(fb, fb_w, fb_h, sx, sy, r, g, b_col);
            fb_pixel(fb, fb_w, fb_h, sx + 1, sy, r, g, b_col);
            fb_pixel(fb, fb_w, fb_h, sx, sy + 1, r, g, b_col);
            fb_pixel(fb, fb_w, fb_h, sx + 1, sy + 1, r, g, b_col);

            // Small heading tick for airborne
            if (f->velocity > 10) {
                float rad = f->heading * DEG2RAD;
                int hx = sx + (int)(sinf(rad) * 4);
                int hy = sy - (int)(cosf(rad) * 4);
                draw_line(fb, fb_w, fb_h, sx, sy, hx, hy, r, g, b_col);
            }
        }
    }

    // Draw cursor crosshair
    if (ms->cursor_active) {
        for (int d = -8; d <= 8; d++) {
            if (d >= -2 && d <= 2) continue; // gap in center
            fb_pixel(fb, fb_w, fb_h, ms->cursor_x + d, ms->cursor_y, 255, 255, 255);
            fb_pixel(fb, fb_w, fb_h, ms->cursor_x, ms->cursor_y + d, 255, 255, 255);
        }
    }

    // Draw coordinate label at cursor or center
    {
        float lat, lon;
        geo_unproject(ms->cursor_active ? ms->cursor_x : fb_w / 2,
                     ms->cursor_active ? ms->cursor_y : fb_h / 2,
                     ms->center_lat, ms->center_lon,
                     ms->zoom, fb_w, fb_h, &lat, &lon);
        draw_number(fb, fb_w, fb_h, 2, 2, (int)lat, 100, 100, 100);
        draw_number(fb, fb_w, fb_h, 35, 2, (int)lon, 100, 100, 100);
    }

    // Zoom indicator
    draw_number(fb, fb_w, fb_h, fb_w - 30, 2, (int)(ms->zoom * 10), 100, 100, 100);
}

void map_pan(MapState *ms, float dlat, float dlon) {
    ms->center_lat += dlat / ms->zoom;
    ms->center_lon += dlon / ms->zoom;
    if (ms->center_lat > 85) ms->center_lat = 85;
    if (ms->center_lat < -85) ms->center_lat = -85;
    if (ms->center_lon > 180) ms->center_lon -= 360;
    if (ms->center_lon < -180) ms->center_lon += 360;
}

void map_zoom_in(MapState *ms) {
    ms->zoom *= 1.3f;
    if (ms->zoom > ZOOM_MAX) ms->zoom = ZOOM_MAX;
}

void map_zoom_out(MapState *ms) {
    ms->zoom /= 1.3f;
    if (ms->zoom < ZOOM_MIN) ms->zoom = ZOOM_MIN;
}

void map_select_nearest(MapState *ms, const FlightDB *db) {
    float lat, lon;
    geo_unproject(ms->cursor_x, ms->cursor_y,
                 ms->center_lat, ms->center_lon,
                 ms->zoom, MAP_SCREEN_W, MAP_SCREEN_H, &lat, &lon);

    float best_dist = 1e9f;
    int best_idx = -1;
    int n = 0;

    for (int i = 0; i < MAX_FLIGHTS; i++) {
        if (!db->flights[i].valid) continue;
        float dist = geo_distance_km(lat, lon,
                                      db->flights[i].latitude,
                                      db->flights[i].longitude);
        if (dist < best_dist) {
            best_dist = dist;
            best_idx = n;
        }
        n++;
    }

    ms->selected = best_idx;
}

void map_move_cursor(MapState *ms, int dx, int dy) {
    ms->cursor_active = true;
    ms->cursor_x += dx;
    ms->cursor_y += dy;
    if (ms->cursor_x < 0) ms->cursor_x = 0;
    if (ms->cursor_x >= MAP_SCREEN_W) ms->cursor_x = MAP_SCREEN_W - 1;
    if (ms->cursor_y < 0) ms->cursor_y = 0;
    if (ms->cursor_y >= MAP_SCREEN_H) ms->cursor_y = MAP_SCREEN_H - 1;
}
