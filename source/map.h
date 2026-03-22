#ifndef MAP_H
#define MAP_H

#include <3ds.h>
#include "flights.h"

#define MAP_SCREEN_W  400
#define MAP_SCREEN_H  240

#define ZOOM_MIN      1.0f
#define ZOOM_MAX      20.0f
#define ZOOM_DEFAULT  2.2f

typedef struct {
    float center_lat;
    float center_lon;
    float zoom;
    int   selected;      // index into FlightDB, or -1
    int   cursor_x;      // screen cursor position
    int   cursor_y;
    bool  cursor_active; // whether cursor is shown
    bool  show_airports; // render airport markers
} MapState;

void map_init(MapState *ms);
void map_render(u8 *fb, int fb_w, int fb_h, const MapState *ms, const FlightDB *db);
void map_pan(MapState *ms, float dlat, float dlon);
void map_zoom_in(MapState *ms);
void map_zoom_out(MapState *ms);
void map_select_nearest(MapState *ms, const FlightDB *db);
void map_move_cursor(MapState *ms, int dx, int dy);

#endif
