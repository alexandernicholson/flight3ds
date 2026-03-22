#include "cache.h"
#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static uint32_t cached_time = 0;

bool cache_save(const FlightDB *db) {
    mkdir(CACHE_DIR, 0777);

    FILE *f = fopen(CACHE_FILE, "wb");
    if (!f) return false;

    uint32_t magic = CACHE_MAGIC;
    fwrite(&magic, 4, 1, f);
    fwrite(&db->last_update, 4, 1, f);
    fwrite(&db->count, 4, 1, f);

    for (int i = 0; i < MAX_FLIGHTS; i++) {
        if (!db->flights[i].valid) continue;
        fwrite(&db->flights[i], sizeof(Flight), 1, f);
    }

    fclose(f);
    cached_time = db->last_update;
    return true;
}

bool cache_load(FlightDB *db) {
    FILE *f = fopen(CACHE_FILE, "rb");
    if (!f) return false;

    uint32_t magic;
    if (fread(&magic, 4, 1, f) != 1 || magic != CACHE_MAGIC) {
        fclose(f);
        return false;
    }

    uint32_t update_time;
    int count;
    if (fread(&update_time, 4, 1, f) != 1) { fclose(f); return false; }
    if (fread(&count, 4, 1, f) != 1) { fclose(f); return false; }

    flights_clear(db);
    db->last_update = update_time;
    cached_time = update_time;

    for (int i = 0; i < count && i < MAX_FLIGHTS; i++) {
        Flight flight;
        if (fread(&flight, sizeof(Flight), 1, f) != 1) break;
        flight.valid = true;
        db->flights[db->count++] = flight;
    }

    fclose(f);
    db->online = false;
    return db->count > 0;
}

uint32_t cache_age(void) {
    if (cached_time == 0) return 0;
    // Use osGetTime which returns ms since boot - not ideal but works for relative age
    return (uint32_t)(osGetTime() / 1000);
}
