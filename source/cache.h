#ifndef CACHE_H
#define CACHE_H

#include "flights.h"

#define CACHE_DIR    "sdmc:/3ds/flight3ds"
#define CACHE_FILE   "sdmc:/3ds/flight3ds/flights.bin"
#define CACHE_MAGIC  0x464C5433  // "FLT3"

// Save current flight data to SD card
bool cache_save(const FlightDB *db);

// Load cached flight data from SD card. Returns true if loaded.
bool cache_load(FlightDB *db);

// Get age of cache in seconds (0 if no cache)
uint32_t cache_age(void);

#endif
