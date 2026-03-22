#include "flights.h"
#include "json.h"
#include "geo.h"
#include <string.h>
#include <math.h>

void flights_init(FlightDB *db) {
    memset(db, 0, sizeof(FlightDB));
}

void flights_clear(FlightDB *db) {
    db->count = 0;
    for (int i = 0; i < MAX_FLIGHTS; i++)
        db->flights[i].valid = false;
}

// Parse OpenSky states array response.
// Format: {"time":N,"states":[[icao24,callsign,country,time_pos,last_contact,
//          lon,lat,baro_alt,on_ground,velocity,heading,vert_rate,sensors,
//          geo_alt,squawk,spi,pos_source,category],...]}
int flights_update_from_json(FlightDB *db, const char *json, int json_len) {
    JsonParser parser;
    json_init(&parser, json, json_len);
    JsonParser *p = &parser;

    flights_clear(db);

    if (!json_expect(p, '{')) return -1;

    // Find "states" key
    bool found_states = false;
    while (json_peek(p) != '}' && json_peek(p) != '\0') {
        char key[32];
        if (json_parse_string(p, key, sizeof(key)) < 0) return -1;
        if (!json_expect(p, ':')) return -1;

        if (strcmp(key, "time") == 0) {
            json_parse_uint32(p, &db->last_update);
        } else if (strcmp(key, "states") == 0) {
            found_states = true;

            if (json_is_null(p)) {
                return 0;
            }

            if (!json_expect(p, '[')) return -1;

            while (json_peek(p) != ']' && json_peek(p) != '\0') {
                if (db->count >= MAX_FLIGHTS) {
                    json_skip_value(p);
                    if (json_peek(p) == ',') json_advance(p);
                    continue;
                }

                Flight *f = &db->flights[db->count];
                memset(f, 0, sizeof(Flight));

                if (!json_expect(p, '[')) return -1;

                // 0: icao24 (string)
                if (json_is_null(p)) f->icao24[0] = '\0';
                else json_parse_string(p, f->icao24, sizeof(f->icao24));
                if (!json_expect(p, ',')) goto skip_state;

                // 1: callsign (string)
                if (json_is_null(p)) f->callsign[0] = '\0';
                else {
                    json_parse_string(p, f->callsign, sizeof(f->callsign));
                    // Trim trailing spaces
                    int len = strlen(f->callsign);
                    while (len > 0 && f->callsign[len - 1] == ' ') f->callsign[--len] = '\0';
                }
                if (!json_expect(p, ',')) goto skip_state;

                // 2: origin_country (string)
                if (json_is_null(p)) f->origin_country[0] = '\0';
                else json_parse_string(p, f->origin_country, sizeof(f->origin_country));
                if (!json_expect(p, ',')) goto skip_state;

                // 3: time_position (int/null) - skip
                json_skip_value(p);
                if (!json_expect(p, ',')) goto skip_state;

                // 4: last_contact (int)
                if (json_is_null(p)) f->last_contact = 0;
                else json_parse_uint32(p, &f->last_contact);
                if (!json_expect(p, ',')) goto skip_state;

                // 5: longitude (float/null)
                if (json_is_null(p)) { f->valid = false; goto finish_state; }
                else json_parse_float(p, &f->longitude);
                if (!json_expect(p, ',')) goto skip_state;

                // 6: latitude (float/null)
                if (json_is_null(p)) { f->valid = false; goto finish_state; }
                else json_parse_float(p, &f->latitude);
                if (!json_expect(p, ',')) goto skip_state;

                // 7: baro_altitude (float/null)
                if (json_is_null(p)) f->altitude = 0;
                else json_parse_float(p, &f->altitude);
                if (!json_expect(p, ',')) goto skip_state;

                // 8: on_ground (bool)
                if (json_peek(p) == 't') {
                    f->on_ground = true;
                    json_skip_value(p);
                } else {
                    f->on_ground = false;
                    json_skip_value(p);
                }
                if (!json_expect(p, ',')) goto skip_state;

                // 9: velocity (float/null)
                if (json_is_null(p)) f->velocity = 0;
                else json_parse_float(p, &f->velocity);
                if (!json_expect(p, ',')) goto skip_state;

                // 10: true_track / heading (float/null)
                if (json_is_null(p)) f->heading = 0;
                else json_parse_float(p, &f->heading);
                if (!json_expect(p, ',')) goto skip_state;

                // 11: vertical_rate (float/null)
                if (json_is_null(p)) f->vertical_rate = 0;
                else json_parse_float(p, &f->vertical_rate);
                if (!json_expect(p, ',')) goto skip_state;

                // 12: sensors (array/null) - skip
                json_skip_value(p);
                if (!json_expect(p, ',')) goto skip_state;

                // 13: geo_altitude (float/null) - prefer over baro if available
                if (!json_is_null(p)) {
                    float geo_alt;
                    json_parse_float(p, &geo_alt);
                    if (geo_alt > 0) f->altitude = geo_alt;
                }

                // Skip remaining fields (squawk, spi, position_source, category)
                finish_state:
                while (json_peek(p) != ']' && json_peek(p) != '\0') {
                    if (json_peek(p) == ',') json_advance(p);
                    json_skip_value(p);
                }
                json_expect(p, ']');

                if (f->latitude != 0 || f->longitude != 0) {
                    f->valid = true;
                    db->count++;
                }

                if (json_peek(p) == ',') json_advance(p);
                continue;

                skip_state:
                while (json_peek(p) != ']' && json_peek(p) != '\0') {
                    json_advance(p);
                }
                json_expect(p, ']');
                if (json_peek(p) == ',') json_advance(p);
            }

            json_expect(p, ']');
        } else {
            json_skip_value(p);
        }

        if (json_peek(p) == ',') json_advance(p);
    }

    if (!found_states) return -1;
    db->online = true;
    return db->count;
}

Flight *flights_nearest(FlightDB *db, float lat, float lon) {
    Flight *best = NULL;
    float best_dist = 1e9f;

    for (int i = 0; i < MAX_FLIGHTS; i++) {
        Flight *f = &db->flights[i];
        if (!f->valid) continue;

        float dist = geo_distance_km(lat, lon, f->latitude, f->longitude);
        if (dist < best_dist) {
            best_dist = dist;
            best = f;
        }
    }
    return best;
}

Flight *flights_get(FlightDB *db, int index) {
    int n = 0;
    for (int i = 0; i < MAX_FLIGHTS; i++) {
        if (!db->flights[i].valid) continue;
        if (n == index) return &db->flights[i];
        n++;
    }
    return NULL;
}

int flights_count_airborne(FlightDB *db) {
    int count = 0;
    for (int i = 0; i < MAX_FLIGHTS; i++) {
        if (db->flights[i].valid && !db->flights[i].on_ground)
            count++;
    }
    return count;
}
