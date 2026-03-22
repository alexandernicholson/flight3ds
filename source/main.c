#include <3ds.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "flights.h"
#include "map.h"
#include "ui.h"
#include "api.h"
#include "cache.h"
#include "datapack.h"
#include "geo.h"

static FlightDB flight_db;
static MapState map_state;
static UIState  ui_state;
static PackIndex pack_index;

static u64 last_fetch_tick = 0;
static bool fetch_pending = false;
static bool first_fetch = true;

static void fetch_flights_for_view(void) {
    // Calculate visible bounding box with some margin
    float lat_min, lat_max, lon_min, lon_max;
    float dummy_lat, dummy_lon;

    geo_unproject(0, MAP_SCREEN_H, map_state.center_lat, map_state.center_lon,
                 map_state.zoom, MAP_SCREEN_W, MAP_SCREEN_H, &lat_min, &dummy_lon);
    geo_unproject(MAP_SCREEN_W, 0, map_state.center_lat, map_state.center_lon,
                 map_state.zoom, MAP_SCREEN_W, MAP_SCREEN_H, &lat_max, &dummy_lon);
    geo_unproject(0, 0, map_state.center_lat, map_state.center_lon,
                 map_state.zoom, MAP_SCREEN_W, MAP_SCREEN_H, &dummy_lat, &lon_min);
    geo_unproject(MAP_SCREEN_W, 0, map_state.center_lat, map_state.center_lon,
                 map_state.zoom, MAP_SCREEN_W, MAP_SCREEN_H, &dummy_lat, &lon_max);

    // Add margin
    float lat_margin = (lat_max - lat_min) * 0.2f;
    float lon_margin = (lon_max - lon_min) * 0.2f;
    lat_min -= lat_margin;
    lat_max += lat_margin;
    lon_min -= lon_margin;
    lon_max += lon_margin;

    // Clamp
    if (lat_min < -90) lat_min = -90;
    if (lat_max > 90) lat_max = 90;

    int result;
    // Use bbox query if zoomed in enough, otherwise fetch all
    if (map_state.zoom > 3.0f) {
        result = api_fetch_flights(&flight_db, lat_min, lat_max, lon_min, lon_max);
    } else {
        result = api_fetch_all(&flight_db);
    }

    if (result >= 0) {
        cache_save(&flight_db);
    }
}

int main(int argc, char *argv[]) {
    gfxInitDefault();
    gfxSetDoubleBuffering(GFX_TOP, true);
    gfxSetDoubleBuffering(GFX_BOTTOM, true);

    // Initialize subsystems
    flights_init(&flight_db);
    map_init(&map_state);
    ui_init(&ui_state);

    bool api_ok = api_init();

    // Scan existing data packs
    pack_scan(&pack_index);

    // Try to load cached flights first
    if (cache_load(&flight_db)) {
        // Got cached data, will refresh from API shortly
    }

    last_fetch_tick = osGetTime();

    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();

        // Exit
        if (kDown & KEY_SELECT) break;

        // Help toggle
        if (kDown & KEY_START) {
            if (ui_state.view == UI_VIEW_HELP)
                ui_state.view = UI_VIEW_MAP;
            else if (ui_state.view == UI_VIEW_PACKS)
                ui_state.view = UI_VIEW_MAP;
            else
                ui_state.view = UI_VIEW_HELP;
        }

        // Circle pad for cursor movement
        circlePosition pos;
        hidCircleRead(&pos);
        if (abs(pos.dx) > 20 || abs(pos.dy) > 20) {
            map_move_cursor(&map_state, pos.dx / 25, -pos.dy / 25);
        }

        if (ui_state.view == UI_VIEW_MAP || ui_state.view == UI_VIEW_DETAIL) {
            // D-pad: pan map
            float pan_speed = 5.0f;
            if (kHeld & KEY_DUP)    map_pan(&map_state, pan_speed, 0);
            if (kHeld & KEY_DDOWN)  map_pan(&map_state, -pan_speed, 0);
            if (kHeld & KEY_DLEFT)  map_pan(&map_state, 0, -pan_speed);
            if (kHeld & KEY_DRIGHT) map_pan(&map_state, 0, pan_speed);

            // L/R: zoom (L+R together: open data packs)
            if ((kDown & KEY_L) && (kHeld & KEY_R)) {
                pack_scan(&pack_index);
                ui_state.view = UI_VIEW_PACKS;
            } else if ((kDown & KEY_R) && (kHeld & KEY_L)) {
                pack_scan(&pack_index);
                ui_state.view = UI_VIEW_PACKS;
            } else if (kDown & KEY_L) {
                map_zoom_out(&map_state);
            } else if (kDown & KEY_R) {
                map_zoom_in(&map_state);
            }

            // A: select nearest flight / show detail
            if (kDown & KEY_A) {
                if (ui_state.view == UI_VIEW_MAP) {
                    if (map_state.cursor_active) {
                        map_select_nearest(&map_state, &flight_db);
                    }
                    if (map_state.selected >= 0) {
                        ui_state.view = UI_VIEW_DETAIL;
                        ui_state.list_selected = map_state.selected;

                        // Center map on selected flight
                        const Flight *f = flights_get(&flight_db, map_state.selected);
                        if (f) {
                            map_state.center_lat = f->latitude;
                            map_state.center_lon = f->longitude;
                        }
                    }
                } else {
                    // In detail view, A browses to next flight
                    int max = flight_db.count;
                    if (max > 0) {
                        map_state.selected = (map_state.selected + 1) % max;
                        ui_state.list_selected = map_state.selected;
                        const Flight *f = flights_get(&flight_db, map_state.selected);
                        if (f) {
                            map_state.center_lat = f->latitude;
                            map_state.center_lon = f->longitude;
                        }
                    }
                }
            }

            // B: deselect / back
            if (kDown & KEY_B) {
                if (ui_state.view == UI_VIEW_DETAIL) {
                    ui_state.view = UI_VIEW_MAP;
                } else {
                    map_state.selected = -1;
                }
            }

            // X: toggle cursor
            if (kDown & KEY_X) {
                map_state.cursor_active = !map_state.cursor_active;
                if (map_state.cursor_active) {
                    map_state.cursor_x = MAP_SCREEN_W / 2;
                    map_state.cursor_y = MAP_SCREEN_H / 2;
                }
            }

            // Y: save data pack (long press could be force refresh)
            if (kDown & KEY_Y) {
                if (flight_db.count > 0) {
                    char pack_name[PACK_NAME_LEN];
                    pack_auto_name(pack_name, sizeof(pack_name),
                                   map_state.center_lat, map_state.center_lon);
                    pack_save(pack_name, &flight_db,
                              map_state.center_lat, map_state.center_lon,
                              map_state.zoom);
                    pack_scan(&pack_index);
                    // Switch to pack view to confirm
                    ui_state.view = UI_VIEW_PACKS;
                    ui_state.pack_selected = pack_index.count - 1;
                } else {
                    fetch_pending = true;
                }
            }
        }

        // Pack browser controls
        if (ui_state.view == UI_VIEW_PACKS) {
            if (kDown & KEY_DUP) {
                if (ui_state.pack_selected > 0) ui_state.pack_selected--;
                if (ui_state.pack_selected < ui_state.pack_scroll)
                    ui_state.pack_scroll = ui_state.pack_selected;
            }
            if (kDown & KEY_DDOWN) {
                if (ui_state.pack_selected < pack_index.count - 1)
                    ui_state.pack_selected++;
                int visible = (BOTTOM_H - 40) / 18;
                if (ui_state.pack_selected >= ui_state.pack_scroll + visible)
                    ui_state.pack_scroll = ui_state.pack_selected - visible + 1;
            }
            // A: load selected pack
            if (kDown & KEY_A) {
                if (pack_index.count > 0) {
                    float lat, lon, zoom;
                    if (pack_load(&pack_index, ui_state.pack_selected,
                                  &flight_db, &lat, &lon, &zoom)) {
                        map_state.center_lat = lat;
                        map_state.center_lon = lon;
                        map_state.zoom = zoom;
                        ui_state.view = UI_VIEW_MAP;
                    }
                }
            }
            // X: delete selected pack
            if (kDown & KEY_X) {
                if (pack_index.count > 0) {
                    pack_delete(&pack_index, ui_state.pack_selected);
                    pack_scan(&pack_index);
                    if (ui_state.pack_selected >= pack_index.count && pack_index.count > 0)
                        ui_state.pack_selected = pack_index.count - 1;
                }
            }
            // B: back to map
            if (kDown & KEY_B) {
                ui_state.view = UI_VIEW_MAP;
            }
            // Y: save current as new pack
            if (kDown & KEY_Y) {
                if (flight_db.count > 0) {
                    char pack_name[PACK_NAME_LEN];
                    pack_auto_name(pack_name, sizeof(pack_name),
                                   map_state.center_lat, map_state.center_lon);
                    pack_save(pack_name, &flight_db,
                              map_state.center_lat, map_state.center_lon,
                              map_state.zoom);
                    pack_scan(&pack_index);
                    ui_state.pack_selected = pack_index.count - 1;
                }
            }
        }

        // In list view, D-pad scrolls list
        if (ui_state.view == UI_VIEW_MAP) {
            if (kDown & KEY_CPAD_UP)    ui_list_scroll_up(&ui_state);
            if (kDown & KEY_CPAD_DOWN)  ui_list_scroll_down(&ui_state, flight_db.count);
        }

        // Periodic flight data fetch
        u64 now = osGetTime();
        u64 elapsed_ms = now - last_fetch_tick;
        if (api_ok && (fetch_pending || first_fetch || elapsed_ms > API_POLL_INTERVAL_S * 1000)) {
            fetch_flights_for_view();
            last_fetch_tick = now;
            fetch_pending = false;
            first_fetch = false;
        }

        // Render top screen (map)
        u16 top_w, top_h;
        u8 *top_fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, &top_h, &top_w);
        // Note: gfxGetFramebuffer returns (height, width) due to rotation
        map_render(top_fb, top_w, top_h, &map_state, &flight_db);
        ui_draw_status_bar(top_fb, top_w, top_h, &flight_db,
                          flight_db.online, cache_age());

        // Render bottom screen
        u16 bot_w, bot_h;
        u8 *bot_fb = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, &bot_h, &bot_w);
        ui_render_bottom(bot_fb, &ui_state, &flight_db, &map_state,
                        &pack_index, flight_db.online, cache_age());

        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    api_shutdown();
    gfxExit();
    return 0;
}
