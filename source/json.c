#include "json.h"
#include <string.h>
#include <stdlib.h>

void json_init(JsonParser *p, const char *data, int len) {
    p->data = data;
    p->len = len;
    p->pos = 0;
}

void json_skip_ws(JsonParser *p) {
    while (p->pos < p->len) {
        char c = p->data[p->pos];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
            p->pos++;
        else
            break;
    }
}

char json_peek(JsonParser *p) {
    json_skip_ws(p);
    if (p->pos >= p->len) return '\0';
    return p->data[p->pos];
}

void json_advance(JsonParser *p) {
    if (p->pos < p->len) p->pos++;
}

bool json_expect(JsonParser *p, char c) {
    json_skip_ws(p);
    if (p->pos >= p->len || p->data[p->pos] != c) return false;
    p->pos++;
    return true;
}

bool json_skip_value(JsonParser *p) {
    json_skip_ws(p);
    if (p->pos >= p->len) return false;

    char c = p->data[p->pos];

    if (c == '"') {
        p->pos++;
        while (p->pos < p->len) {
            if (p->data[p->pos] == '\\') {
                p->pos += 2;
            } else if (p->data[p->pos] == '"') {
                p->pos++;
                return true;
            } else {
                p->pos++;
            }
        }
        return false;
    }

    if (c == '{') {
        p->pos++;
        json_skip_ws(p);
        if (json_peek(p) == '}') { p->pos++; return true; }
        while (1) {
            if (!json_skip_value(p)) return false; // key
            if (!json_expect(p, ':')) return false;
            if (!json_skip_value(p)) return false; // value
            json_skip_ws(p);
            if (json_peek(p) == '}') { p->pos++; return true; }
            if (!json_expect(p, ',')) return false;
        }
    }

    if (c == '[') {
        p->pos++;
        json_skip_ws(p);
        if (json_peek(p) == ']') { p->pos++; return true; }
        while (1) {
            if (!json_skip_value(p)) return false;
            json_skip_ws(p);
            if (json_peek(p) == ']') { p->pos++; return true; }
            if (!json_expect(p, ',')) return false;
        }
    }

    if (c == 't') { // true
        if (p->pos + 4 <= p->len && memcmp(p->data + p->pos, "true", 4) == 0) {
            p->pos += 4; return true;
        }
        return false;
    }
    if (c == 'f') { // false
        if (p->pos + 5 <= p->len && memcmp(p->data + p->pos, "false", 5) == 0) {
            p->pos += 5; return true;
        }
        return false;
    }
    if (c == 'n') { // null
        if (p->pos + 4 <= p->len && memcmp(p->data + p->pos, "null", 4) == 0) {
            p->pos += 4; return true;
        }
        return false;
    }

    // number
    if (c == '-' || (c >= '0' && c <= '9')) {
        if (c == '-') p->pos++;
        while (p->pos < p->len && p->data[p->pos] >= '0' && p->data[p->pos] <= '9') p->pos++;
        if (p->pos < p->len && p->data[p->pos] == '.') {
            p->pos++;
            while (p->pos < p->len && p->data[p->pos] >= '0' && p->data[p->pos] <= '9') p->pos++;
        }
        if (p->pos < p->len && (p->data[p->pos] == 'e' || p->data[p->pos] == 'E')) {
            p->pos++;
            if (p->pos < p->len && (p->data[p->pos] == '+' || p->data[p->pos] == '-')) p->pos++;
            while (p->pos < p->len && p->data[p->pos] >= '0' && p->data[p->pos] <= '9') p->pos++;
        }
        return true;
    }

    return false;
}

int json_parse_string(JsonParser *p, char *buf, int buf_size) {
    json_skip_ws(p);
    if (p->pos >= p->len || p->data[p->pos] != '"') return -1;
    p->pos++;

    int out = 0;
    while (p->pos < p->len) {
        char c = p->data[p->pos];
        if (c == '\\') {
            p->pos++;
            if (p->pos >= p->len) return -1;
            c = p->data[p->pos];
            if (out < buf_size - 1) buf[out++] = c;
            p->pos++;
        } else if (c == '"') {
            p->pos++;
            if (out < buf_size) buf[out] = '\0';
            return out;
        } else {
            if (out < buf_size - 1) buf[out++] = c;
            p->pos++;
        }
    }
    return -1;
}

bool json_parse_float(JsonParser *p, float *out) {
    json_skip_ws(p);
    if (p->pos >= p->len) return false;

    char buf[32];
    int i = 0;
    while (p->pos < p->len && i < 31) {
        char c = p->data[p->pos];
        if ((c >= '0' && c <= '9') || c == '.' || c == '-' || c == '+' || c == 'e' || c == 'E') {
            buf[i++] = c;
            p->pos++;
        } else {
            break;
        }
    }
    if (i == 0) return false;
    buf[i] = '\0';
    *out = strtof(buf, NULL);
    return true;
}

bool json_parse_int(JsonParser *p, int *out) {
    float f;
    if (!json_parse_float(p, &f)) return false;
    *out = (int)f;
    return true;
}

bool json_parse_uint32(JsonParser *p, unsigned int *out) {
    float f;
    if (!json_parse_float(p, &f)) return false;
    *out = (unsigned int)f;
    return true;
}

bool json_is_null(JsonParser *p) {
    json_skip_ws(p);
    if (p->pos + 4 <= p->len && memcmp(p->data + p->pos, "null", 4) == 0) {
        p->pos += 4;
        return true;
    }
    return false;
}
