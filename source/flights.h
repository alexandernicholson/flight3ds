#ifndef FLIGHTS_H
#define FLIGHTS_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_FLIGHTS    512
#define CALLSIGN_LEN   9
#define COUNTRY_LEN    32

typedef struct {
    char     icao24[7];
    char     callsign[CALLSIGN_LEN];
    char     origin_country[COUNTRY_LEN];
    float    latitude;
    float    longitude;
    float    altitude;       // meters (geometric)
    float    velocity;       // m/s
    float    heading;        // degrees from north
    float    vertical_rate;  // m/s
    bool     on_ground;
    uint32_t last_contact;   // unix timestamp
    bool     valid;          // slot in use
} Flight;

typedef struct {
    Flight   flights[MAX_FLIGHTS];
    int      count;
    uint32_t last_update;    // unix timestamp of last API fetch
    bool     online;         // whether last fetch succeeded
} FlightDB;

void     flights_init(FlightDB *db);
void     flights_clear(FlightDB *db);
int      flights_update_from_json(FlightDB *db, const char *json, int json_len);
Flight  *flights_nearest(FlightDB *db, float lat, float lon);
Flight  *flights_get(FlightDB *db, int index);
int      flights_count_airborne(FlightDB *db);

#endif
