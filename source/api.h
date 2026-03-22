#ifndef API_H
#define API_H

#include "flights.h"

#define API_POLL_INTERVAL_S  15  // seconds between fetches
#define API_TIMEOUT_S        10

// Initialize HTTP service
bool api_init(void);

// Fetch flights within bounding box. Returns number of flights parsed, or -1 on error.
int api_fetch_flights(FlightDB *db, float lat_min, float lat_max,
                      float lon_min, float lon_max);

// Fetch all flights (no bounding box filter). Returns count or -1.
int api_fetch_all(FlightDB *db);

// Cleanup
void api_shutdown(void);

#endif
