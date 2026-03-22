#ifndef DATAPACK_H
#define DATAPACK_H

#include "flights.h"
#include <stdbool.h>

// Data packs are named snapshots of flight data saved to SD card.
// They allow recording and replaying flight states offline.
//
// Format on disk:
//   sdmc:/3ds/flight3ds/packs/<name>.f3d
//
// File format:
//   [4] magic "F3DP"
//   [4] version (1)
//   [4] timestamp (unix)
//   [4] flight count
//   [32] pack name (null-terminated)
//   [16] region description (null-terminated)
//   [4] center_lat (float)
//   [4] center_lon (float)
//   [4] zoom (float)
//   [N * sizeof(Flight)] flight data

#define PACK_DIR       "sdmc:/3ds/flight3ds/packs"
#define PACK_MAGIC     0x46334450  // "F3DP"
#define PACK_VERSION   1
#define PACK_NAME_LEN  32
#define PACK_REGION_LEN 16
#define MAX_PACKS      32

typedef struct {
    char     name[PACK_NAME_LEN];
    char     region[PACK_REGION_LEN];
    uint32_t timestamp;
    int      flight_count;
    float    center_lat;
    float    center_lon;
    float    zoom;
} PackInfo;

typedef struct {
    PackInfo packs[MAX_PACKS];
    int      count;
} PackIndex;

// Scan the packs directory and populate index.
void pack_scan(PackIndex *idx);

// Save current state as a named pack. Region is auto-detected from center coords.
bool pack_save(const char *name, const FlightDB *db,
               float center_lat, float center_lon, float zoom);

// Load a pack by index into the flight database. Returns true on success.
// Also sets *out_lat, *out_lon, *out_zoom to the saved view.
bool pack_load(const PackIndex *idx, int pack_idx, FlightDB *db,
               float *out_lat, float *out_lon, float *out_zoom);

// Delete a pack by index.
bool pack_delete(const PackIndex *idx, int pack_idx);

// Auto-detect region name from lat/lon
const char *pack_region_name(float lat, float lon);

// Generate a default pack name from region + timestamp
void pack_auto_name(char *buf, int buf_size, float lat, float lon);

#endif
