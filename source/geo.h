#ifndef GEO_H
#define GEO_H

#include <math.h>

#define DEG2RAD (M_PI / 180.0f)
#define RAD2DEG (180.0f / M_PI)
#define EARTH_RADIUS_KM 6371.0f

// Equirectangular projection: lat/lon to screen coordinates
static inline void geo_project(float lat, float lon,
                               float center_lat, float center_lon,
                               float zoom, int screen_w, int screen_h,
                               int *sx, int *sy) {
    float dx = (lon - center_lon) * zoom;
    float dy = (center_lat - lat) * zoom;
    *sx = (int)(dx + screen_w / 2.0f);
    *sy = (int)(dy + screen_h / 2.0f);
}

// Inverse: screen coordinates to lat/lon
static inline void geo_unproject(int sx, int sy,
                                 float center_lat, float center_lon,
                                 float zoom, int screen_w, int screen_h,
                                 float *lat, float *lon) {
    *lon = (sx - screen_w / 2.0f) / zoom + center_lon;
    *lat = center_lat - (sy - screen_h / 2.0f) / zoom;
}

// Haversine distance in km
static inline float geo_distance_km(float lat1, float lon1,
                                     float lat2, float lon2) {
    float dlat = (lat2 - lat1) * DEG2RAD;
    float dlon = (lon2 - lon1) * DEG2RAD;
    float a = sinf(dlat / 2) * sinf(dlat / 2) +
              cosf(lat1 * DEG2RAD) * cosf(lat2 * DEG2RAD) *
              sinf(dlon / 2) * sinf(dlon / 2);
    float c = 2 * atan2f(sqrtf(a), sqrtf(1 - a));
    return EARTH_RADIUS_KM * c;
}

// Bearing from point 1 to point 2 in degrees
static inline float geo_bearing(float lat1, float lon1,
                                float lat2, float lon2) {
    float dlon = (lon2 - lon1) * DEG2RAD;
    float y = sinf(dlon) * cosf(lat2 * DEG2RAD);
    float x = cosf(lat1 * DEG2RAD) * sinf(lat2 * DEG2RAD) -
              sinf(lat1 * DEG2RAD) * cosf(lat2 * DEG2RAD) * cosf(dlon);
    float brng = atan2f(y, x) * RAD2DEG;
    return fmodf(brng + 360.0f, 360.0f);
}

// Meters to feet
static inline float geo_m_to_ft(float m) {
    return m * 3.28084f;
}

// m/s to knots
static inline float geo_ms_to_kts(float ms) {
    return ms * 1.94384f;
}

#endif
