#ifndef JSON_H
#define JSON_H

#include <stdbool.h>

// Minimal JSON parser for OpenSky API responses.
// Only handles the specific structure: {"time":N,"states":[[...],[...],...]}

typedef struct {
    const char *data;
    int         len;
    int         pos;
} JsonParser;

void json_init(JsonParser *p, const char *data, int len);

// Skip whitespace
void json_skip_ws(JsonParser *p);

// Check current char
char json_peek(JsonParser *p);

// Advance past current char
void json_advance(JsonParser *p);

// Expect and consume a specific char. Returns false on mismatch.
bool json_expect(JsonParser *p, char c);

// Skip a JSON value (string, number, object, array, bool, null)
bool json_skip_value(JsonParser *p);

// Parse a JSON string into buf (without quotes). Returns length, -1 on error.
int json_parse_string(JsonParser *p, char *buf, int buf_size);

// Parse a JSON number as float. Returns false on error.
bool json_parse_float(JsonParser *p, float *out);

// Parse a JSON number as int. Returns false on error.
bool json_parse_int(JsonParser *p, int *out);

// Parse a JSON number as unsigned 32-bit. Returns false on error.
bool json_parse_uint32(JsonParser *p, unsigned int *out);

// Check if current value is null, and skip it if so.
bool json_is_null(JsonParser *p);

#endif
