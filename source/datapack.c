#include "datapack.h"
#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

// Region detection by lat/lon bounding boxes
typedef struct {
    const char *name;
    float lat_min, lat_max, lon_min, lon_max;
} Region;

static const Region regions[] = {
    {"N.America",   15,  72, -170, -50},
    {"C.America",    5,  25, -120, -60},
    {"S.America",  -56,  15,  -85, -30},
    {"Europe",      35,  72,  -12,  40},
    {"UK/Ireland",  49,  61,  -11,   2},
    {"Scandinavia", 55,  72,    4,  32},
    {"Mid East",    12,  42,   25,  63},
    {"N.Africa",    15,  38,  -18,  40},
    {"S.Africa",   -35,  15,   10,  55},
    {"W.Africa",     0,  20,  -20,  15},
    {"S.Asia",       5,  38,   60,  98},
    {"E.Asia",      18,  55,   98, 145},
    {"Japan",       24,  46,  125, 150},
    {"SE.Asia",    -10,  25,   95, 145},
    {"Australia",  -45, -10,  110, 160},
    {"NZ/Pacific", -48, -30,  165, 180},
    {"Caribbean",   10,  28,  -90, -58},
    {"N.Atlantic",  40,  65,  -50, -10},
    {"N.Pacific",   20,  55,  140, 180},
};
#define NUM_REGIONS (sizeof(regions) / sizeof(regions[0]))

const char *pack_region_name(float lat, float lon) {
    for (int i = 0; i < (int)NUM_REGIONS; i++) {
        if (lat >= regions[i].lat_min && lat <= regions[i].lat_max &&
            lon >= regions[i].lon_min && lon <= regions[i].lon_max)
            return regions[i].name;
    }
    return "Global";
}

void pack_auto_name(char *buf, int buf_size, float lat, float lon) {
    const char *region = pack_region_name(lat, lon);
    // Use osGetTime as a simple counter for unique names
    u32 tick = (u32)(osGetTime() / 1000) & 0xFFFF;
    snprintf(buf, buf_size, "%s_%04X", region, tick);
    // Sanitize: replace dots and spaces
    for (int i = 0; buf[i]; i++) {
        if (buf[i] == '.' || buf[i] == ' ') buf[i] = '_';
    }
}

void pack_scan(PackIndex *idx) {
    idx->count = 0;
    mkdir(PACK_DIR, 0777);

    DIR *dir = opendir(PACK_DIR);
    if (!dir) return;

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL && idx->count < MAX_PACKS) {
        int len = strlen(ent->d_name);
        if (len < 5) continue;
        if (strcmp(ent->d_name + len - 4, ".f3d") != 0) continue;

        char path[256];
        snprintf(path, sizeof(path), "%s/%s", PACK_DIR, ent->d_name);

        FILE *f = fopen(path, "rb");
        if (!f) continue;

        uint32_t magic, version;
        if (fread(&magic, 4, 1, f) != 1 || magic != PACK_MAGIC) {
            fclose(f); continue;
        }
        if (fread(&version, 4, 1, f) != 1 || version != PACK_VERSION) {
            fclose(f); continue;
        }

        PackInfo *pi = &idx->packs[idx->count];
        fread(&pi->timestamp, 4, 1, f);
        fread(&pi->flight_count, 4, 1, f);
        fread(pi->name, PACK_NAME_LEN, 1, f);
        pi->name[PACK_NAME_LEN - 1] = '\0';
        fread(pi->region, PACK_REGION_LEN, 1, f);
        pi->region[PACK_REGION_LEN - 1] = '\0';
        fread(&pi->center_lat, 4, 1, f);
        fread(&pi->center_lon, 4, 1, f);
        fread(&pi->zoom, 4, 1, f);

        fclose(f);
        idx->count++;
    }

    closedir(dir);
}

bool pack_save(const char *name, const FlightDB *db,
               float center_lat, float center_lon, float zoom) {
    mkdir(PACK_DIR, 0777);

    char path[256];
    snprintf(path, sizeof(path), "%s/%s.f3d", PACK_DIR, name);

    FILE *f = fopen(path, "wb");
    if (!f) return false;

    uint32_t magic = PACK_MAGIC;
    uint32_t version = PACK_VERSION;
    fwrite(&magic, 4, 1, f);
    fwrite(&version, 4, 1, f);
    fwrite(&db->last_update, 4, 1, f);
    fwrite(&db->count, 4, 1, f);

    // Pack name (padded to fixed length)
    char name_buf[PACK_NAME_LEN] = {0};
    strncpy(name_buf, name, PACK_NAME_LEN - 1);
    fwrite(name_buf, PACK_NAME_LEN, 1, f);

    // Region
    char region_buf[PACK_REGION_LEN] = {0};
    const char *region = pack_region_name(center_lat, center_lon);
    strncpy(region_buf, region, PACK_REGION_LEN - 1);
    fwrite(region_buf, PACK_REGION_LEN, 1, f);

    // View state
    fwrite(&center_lat, 4, 1, f);
    fwrite(&center_lon, 4, 1, f);
    fwrite(&zoom, 4, 1, f);

    // Flight data
    for (int i = 0; i < MAX_FLIGHTS; i++) {
        if (!db->flights[i].valid) continue;
        fwrite(&db->flights[i], sizeof(Flight), 1, f);
    }

    fclose(f);
    return true;
}

bool pack_load(const PackIndex *idx, int pack_idx, FlightDB *db,
               float *out_lat, float *out_lon, float *out_zoom) {
    if (pack_idx < 0 || pack_idx >= idx->count) return false;

    const PackInfo *pi = &idx->packs[pack_idx];

    // Reconstruct filename from name
    char path[256];
    snprintf(path, sizeof(path), "%s/%s.f3d", PACK_DIR, pi->name);

    FILE *f = fopen(path, "rb");
    if (!f) return false;

    // Skip header
    uint32_t magic, version, timestamp;
    int count;
    fread(&magic, 4, 1, f);
    fread(&version, 4, 1, f);
    fread(&timestamp, 4, 1, f);
    fread(&count, 4, 1, f);

    if (magic != PACK_MAGIC || version != PACK_VERSION) {
        fclose(f); return false;
    }

    // Skip name + region + view (already in PackInfo)
    char skip[PACK_NAME_LEN + PACK_REGION_LEN];
    fread(skip, PACK_NAME_LEN + PACK_REGION_LEN, 1, f);

    float lat, lon, zoom;
    fread(&lat, 4, 1, f);
    fread(&lon, 4, 1, f);
    fread(&zoom, 4, 1, f);

    // Load flights
    flights_clear(db);
    db->last_update = timestamp;
    db->online = false;

    for (int i = 0; i < count && i < MAX_FLIGHTS; i++) {
        Flight flight;
        if (fread(&flight, sizeof(Flight), 1, f) != 1) break;
        flight.valid = true;
        db->flights[db->count++] = flight;
    }

    fclose(f);

    if (out_lat) *out_lat = lat;
    if (out_lon) *out_lon = lon;
    if (out_zoom) *out_zoom = zoom;

    return db->count > 0;
}

bool pack_delete(const PackIndex *idx, int pack_idx) {
    if (pack_idx < 0 || pack_idx >= idx->count) return false;
    char path[256];
    snprintf(path, sizeof(path), "%s/%s.f3d", PACK_DIR, idx->packs[pack_idx].name);
    return remove(path) == 0;
}
