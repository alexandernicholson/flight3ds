#ifndef UI_H
#define UI_H

#include <3ds.h>
#include "flights.h"
#include "map.h"
#include "datapack.h"

#define BOTTOM_W  320
#define BOTTOM_H  240

typedef enum {
    UI_VIEW_MAP,       // top: map, bottom: flight list
    UI_VIEW_DETAIL,    // top: map centered on flight, bottom: flight info
    UI_VIEW_HELP,      // bottom: controls help
    UI_VIEW_PACKS,     // bottom: data pack browser
} UIView;

typedef struct {
    UIView   view;
    int      list_scroll;    // scroll offset in flight list
    int      list_selected;  // selected index in list
    bool     show_status;    // show status bar overlay on map
    uint32_t status_timer;   // auto-hide status bar
    int      pack_selected;  // selected index in pack list
    int      pack_scroll;    // scroll offset in pack list
} UIState;

void ui_init(UIState *ui);
void ui_render_bottom(u8 *fb, const UIState *ui, const FlightDB *db,
                      const MapState *ms, const PackIndex *packs,
                      bool online, uint32_t cache_age_s);
void ui_draw_status_bar(u8 *fb, int fb_w, int fb_h, const FlightDB *db,
                        bool online, uint32_t cache_age_s);
void ui_list_scroll_up(UIState *ui);
void ui_list_scroll_down(UIState *ui, int max);
void ui_list_select(UIState *ui, int index);

#endif
