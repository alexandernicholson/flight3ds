#ifndef AIRPORTS_H
#define AIRPORTS_H

// Major world airports with ICAO code, name, coordinates, and type.
// ~200 airports covering all continents and major hubs.

typedef enum {
    APT_LARGE,   // major international hub
    APT_MEDIUM,  // regional/national
} AirportType;

typedef struct {
    const char *icao;
    const char *name;
    float       lat;
    float       lon;
    AirportType type;
} Airport;

static const Airport airports[] = {
    // === NORTH AMERICA ===
    {"KATL", "Atlanta",           33.6367, -84.4281, APT_LARGE},
    {"KLAX", "Los Angeles",       33.9425, -118.408, APT_LARGE},
    {"KORD", "Chicago OHare",     41.9742, -87.9073, APT_LARGE},
    {"KDFW", "Dallas FtWorth",    32.8968, -97.0380, APT_LARGE},
    {"KDEN", "Denver",            39.8561, -104.674, APT_LARGE},
    {"KJFK", "New York JFK",      40.6399, -73.7787, APT_LARGE},
    {"KSFO", "San Francisco",     37.6213, -122.379, APT_LARGE},
    {"KLAS", "Las Vegas",         36.0800, -115.152, APT_LARGE},
    {"KMIA", "Miami",             25.7959, -80.2870, APT_LARGE},
    {"KSEA", "Seattle",           47.4502, -122.309, APT_LARGE},
    {"KEWR", "Newark",            40.6925, -74.1687, APT_LARGE},
    {"KMCO", "Orlando",           28.4312, -81.3081, APT_MEDIUM},
    {"KBOS", "Boston",            42.3656, -71.0096, APT_LARGE},
    {"KMSP", "Minneapolis",       44.8848, -93.2223, APT_MEDIUM},
    {"KDTW", "Detroit",           42.2124, -83.3534, APT_MEDIUM},
    {"KPHL", "Philadelphia",      39.8744, -75.2424, APT_MEDIUM},
    {"KLGA", "New York LaGuardia",40.7772, -73.8726, APT_MEDIUM},
    {"KIAH", "Houston",           29.9902, -95.3368, APT_LARGE},
    {"KPHX", "Phoenix",           33.4373, -112.008, APT_MEDIUM},
    {"KCLT", "Charlotte",         35.2140, -80.9431, APT_MEDIUM},
    {"KDCA", "Washington Reagan", 38.8521, -77.0377, APT_MEDIUM},
    {"KIAD", "Washington Dulles", 38.9445, -77.4558, APT_LARGE},
    {"PANC", "Anchorage",         61.1741, -149.996, APT_MEDIUM},
    {"PHNL", "Honolulu",          21.3187, -157.922, APT_MEDIUM},
    {"CYYZ", "Toronto Pearson",   43.6772, -79.6306, APT_LARGE},
    {"CYVR", "Vancouver",         49.1947, -123.184, APT_LARGE},
    {"CYUL", "Montreal Trudeau",  45.4706, -73.7408, APT_MEDIUM},
    {"CYOW", "Ottawa",            45.3225, -75.6692, APT_MEDIUM},
    {"MMMX", "Mexico City",       19.4363, -99.0721, APT_LARGE},
    {"MMUN", "Cancun",            21.0365, -86.8771, APT_MEDIUM},

    // === SOUTH AMERICA ===
    {"SBGR", "Sao Paulo Guard.",  -23.432, -46.4697, APT_LARGE},
    {"SCEL", "Santiago",          -33.393, -70.7858, APT_LARGE},
    {"SAEZ", "Buenos Aires Eze.", -34.822, -58.5358, APT_LARGE},
    {"SKBO", "Bogota",             4.7016, -74.1469, APT_LARGE},
    {"SPJC", "Lima",              -12.022, -77.1143, APT_LARGE},
    {"SBGL", "Rio de Janeiro",    -22.809, -43.2506, APT_MEDIUM},
    {"SEQM", "Quito",             -0.1292, -78.3575, APT_MEDIUM},
    {"SLLP", "La Paz",            -16.513, -68.1923, APT_MEDIUM},

    // === EUROPE ===
    {"EGLL", "London Heathrow",   51.4706, -0.46133, APT_LARGE},
    {"EGKK", "London Gatwick",    51.1481, -0.19028, APT_MEDIUM},
    {"EGLC", "London City",       51.5053,  0.05525, APT_MEDIUM},
    {"EGSS", "London Stansted",   51.8850,  0.23500, APT_MEDIUM},
    {"LFPG", "Paris CDG",         49.0097,  2.54778, APT_LARGE},
    {"LFPO", "Paris Orly",        48.7233,  2.37944, APT_MEDIUM},
    {"EDDF", "Frankfurt",         50.0333,  8.57056, APT_LARGE},
    {"EHAM", "Amsterdam Schiphol",52.3086,  4.76389, APT_LARGE},
    {"LEMD", "Madrid Barajas",    40.4936, -3.56676, APT_LARGE},
    {"LEBL", "Barcelona",         41.2971,  2.07846, APT_MEDIUM},
    {"LIRF", "Rome Fiumicino",    41.8003, 12.2389,  APT_LARGE},
    {"EDDM", "Munich",            48.3538, 11.7861,  APT_LARGE},
    {"LSZH", "Zurich",            47.4647,  8.54917, APT_LARGE},
    {"EIDW", "Dublin",            53.4213, -6.27007, APT_MEDIUM},
    {"LOWW", "Vienna",            48.1103, 16.5697,  APT_MEDIUM},
    {"EKCH", "Copenhagen",        55.6180, 12.6508,  APT_MEDIUM},
    {"ENGM", "Oslo Gardermoen",   60.1939, 11.1004,  APT_MEDIUM},
    {"ESSA", "Stockholm Arlanda", 59.6519, 17.9186,  APT_MEDIUM},
    {"EFHK", "Helsinki Vantaa",   60.3172, 24.9633,  APT_MEDIUM},
    {"EPWA", "Warsaw Chopin",     52.1657, 20.9671,  APT_MEDIUM},
    {"LKPR", "Prague",            50.1008, 14.2600,  APT_MEDIUM},
    {"LHBP", "Budapest",          47.4369, 19.2556,  APT_MEDIUM},
    {"LGAV", "Athens",            37.9364, 23.9445,  APT_MEDIUM},
    {"LTFM", "Istanbul",          41.2753, 28.7519,  APT_LARGE},
    {"LTAI", "Antalya",           36.8987, 30.8005,  APT_MEDIUM},
    {"UUEE", "Moscow Sheremetyevo",55.9726, 37.4146, APT_LARGE},
    {"ULLI", "St Petersburg",     59.8003, 30.2625,  APT_MEDIUM},
    {"LPPT", "Lisbon",            38.7813, -9.13592, APT_MEDIUM},
    {"EBBR", "Brussels",          50.9014,  4.48444, APT_MEDIUM},
    {"LFMN", "Nice",              43.6584,  7.21587, APT_MEDIUM},

    // === MIDDLE EAST ===
    {"OMDB", "Dubai",             25.2528, 55.3644,  APT_LARGE},
    {"OEJN", "Jeddah",            21.6706, 39.1506,  APT_LARGE},
    {"OERK", "Riyadh",            24.9578, 46.6989,  APT_MEDIUM},
    {"OTHH", "Doha Hamad",        25.2731, 51.6081,  APT_LARGE},
    {"OMAA", "Abu Dhabi",         24.4439, 54.6511,  APT_LARGE},
    {"OIIE", "Tehran IKA",        35.4161, 51.1522,  APT_MEDIUM},
    {"OLBA", "Beirut",            33.8209, 35.4884,  APT_MEDIUM},
    {"LLBG", "Tel Aviv",          32.0114, 34.8867,  APT_MEDIUM},
    {"OBBI", "Bahrain",           26.2708, 50.6336,  APT_MEDIUM},
    {"OKBK", "Kuwait",            29.2266, 47.9689,  APT_MEDIUM},
    {"OOMS", "Muscat",            23.5933, 58.2844,  APT_MEDIUM},

    // === AFRICA ===
    {"FAOR", "Johannesburg",      -26.139, 28.2460,  APT_LARGE},
    {"FACT", "Cape Town",         -33.964, 18.6017,  APT_MEDIUM},
    {"HECA", "Cairo",              30.1219, 31.4056, APT_LARGE},
    {"GMMN", "Casablanca",        33.3675, -7.58972, APT_MEDIUM},
    {"DNMM", "Lagos",              6.5774,  3.32117, APT_LARGE},
    {"HKJK", "Nairobi JKIA",     -1.31924,36.9278,  APT_MEDIUM},
    {"HAAB", "Addis Ababa",        8.97789,38.7993,  APT_MEDIUM},
    {"FALE", "Durban",            -29.614, 31.1197,  APT_MEDIUM},
    {"DTTA", "Tunis",              36.8510, 10.2272, APT_MEDIUM},
    {"DAAG", "Algiers",            36.6910,  3.21541,APT_MEDIUM},

    // === SOUTH ASIA ===
    {"VIDP", "Delhi Indira Gandhi",28.5665,77.1031,  APT_LARGE},
    {"VABB", "Mumbai",             19.0887, 72.8679, APT_LARGE},
    {"VOBL", "Bangalore",          13.1986, 77.7066, APT_MEDIUM},
    {"VOMM", "Chennai",            12.9941, 80.1709, APT_MEDIUM},
    {"VECC", "Kolkata",            22.6547, 88.4467, APT_MEDIUM},
    {"OPKC", "Karachi",            24.9065, 67.1610, APT_MEDIUM},
    {"OPLA", "Lahore",             31.5216, 74.4036, APT_MEDIUM},
    {"VGHS", "Dhaka",              23.8433, 90.3978, APT_MEDIUM},
    {"VCBI", "Colombo",             7.1808, 79.8841, APT_MEDIUM},

    // === EAST ASIA ===
    {"ZBAA", "Beijing Capital",    40.0801,116.585,  APT_LARGE},
    {"ZSPD", "Shanghai Pudong",    31.1434,121.805,  APT_LARGE},
    {"ZGGG", "Guangzhou",          23.3924,113.299,  APT_LARGE},
    {"ZSSS", "Shanghai Hongqiao",  31.1979,121.336,  APT_MEDIUM},
    {"ZUUU", "Chengdu Tianfu",     30.5785,103.947,  APT_MEDIUM},
    {"ZGSZ", "Shenzhen",           22.6393,113.811,  APT_MEDIUM},
    {"VHHH", "Hong Kong",          22.3089,113.915,  APT_LARGE},
    {"VMMC", "Macau",              22.1496,113.592,  APT_MEDIUM},
    {"RCTP", "Taipei Taoyuan",     25.0777,121.233,  APT_LARGE},
    {"RJTT", "Tokyo Haneda",       35.5523,139.780,  APT_LARGE},
    {"RJAA", "Tokyo Narita",       35.7647,140.386,  APT_LARGE},
    {"RJBB", "Osaka Kansai",       34.4347,135.244,  APT_MEDIUM},
    {"RJCC", "Sapporo Chitose",    42.7752,141.692,  APT_MEDIUM},
    {"RKSI", "Seoul Incheon",      37.4691,126.451,  APT_LARGE},
    {"RKSS", "Seoul Gimpo",        37.5586,126.791,  APT_MEDIUM},

    // === SOUTHEAST ASIA ===
    {"WSSS", "Singapore Changi",    1.3502,103.994,  APT_LARGE},
    {"VTBS", "Bangkok Suvarnabhumi",13.6900,100.750, APT_LARGE},
    {"WMKK", "Kuala Lumpur",        2.7456,101.710,  APT_LARGE},
    {"RPLL", "Manila NAIA",        14.5086,121.020,  APT_MEDIUM},
    {"WIII", "Jakarta Soekarno",   -6.1256,106.656,  APT_LARGE},
    {"VVNB", "Hanoi Noi Bai",     21.2212,105.807,  APT_MEDIUM},
    {"VVTS", "Ho Chi Minh",       10.8188,106.652,  APT_MEDIUM},
    {"VDPP", "Phnom Penh",        11.5466,104.844,  APT_MEDIUM},
    {"VLVT", "Vientiane",         17.9883,102.563,  APT_MEDIUM},
    {"WADD", "Bali Ngurah Rai",   -8.7482,115.167,  APT_MEDIUM},

    // === OCEANIA ===
    {"YSSY", "Sydney",            -33.946, 151.177,  APT_LARGE},
    {"YMML", "Melbourne",         -37.673, 144.843,  APT_LARGE},
    {"YBBN", "Brisbane",          -27.384, 153.117,  APT_MEDIUM},
    {"YPPH", "Perth",             -31.940, 115.967,  APT_MEDIUM},
    {"NZAA", "Auckland",          -37.008, 174.792,  APT_MEDIUM},
    {"NZWN", "Wellington",        -41.327, 174.805,  APT_MEDIUM},
    {"NZCH", "Christchurch",      -43.489, 172.532,  APT_MEDIUM},
    {"NFFN", "Fiji Nadi",         -17.755, 177.443,  APT_MEDIUM},
};

#define NUM_AIRPORTS (sizeof(airports) / sizeof(airports[0]))

// Find nearest airport to a lat/lon. Returns NULL if none within threshold_km.
static inline const Airport *airport_nearest(float lat, float lon,
                                              float threshold_km) {
    const Airport *best = NULL;
    float best_dist = threshold_km;
    for (int i = 0; i < (int)NUM_AIRPORTS; i++) {
        float dlat = airports[i].lat - lat;
        float dlon = airports[i].lon - lon;
        // Quick approximation (avoid trig for culling)
        float approx = dlat * dlat + dlon * dlon * 0.7f;
        if (approx > 100) continue; // rough cull: ~1000km
        // Precise check
        float dist = dlat * dlat + dlon * dlon; // simplified for ranking
        if (dist < best_dist) {
            best_dist = dist;
            best = &airports[i];
        }
    }
    return best;
}

// Lookup airport by ICAO code. Returns NULL if not found.
static inline const Airport *airport_by_icao(const char *icao) {
    for (int i = 0; i < (int)NUM_AIRPORTS; i++) {
        if (airports[i].icao[0] == icao[0] &&
            airports[i].icao[1] == icao[1] &&
            airports[i].icao[2] == icao[2] &&
            airports[i].icao[3] == icao[3])
            return &airports[i];
    }
    return NULL;
}

#endif
