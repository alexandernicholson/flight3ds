#ifndef AIRLINES_H
#define AIRLINES_H

// ICAO airline designators to names.
// Callsigns typically start with a 3-letter airline code followed by flight number.
// e.g. "BAW123" = British Airways flight 123

typedef struct {
    const char *icao;    // 3-letter ICAO designator
    const char *name;    // short airline name
} Airline;

static const Airline airlines[] = {
    // === NORTH AMERICA ===
    {"AAL", "American"},
    {"DAL", "Delta"},
    {"UAL", "United"},
    {"SWA", "Southwest"},
    {"JBU", "JetBlue"},
    {"NKS", "Spirit"},
    {"FFT", "Frontier"},
    {"ASA", "Alaska"},
    {"HAL", "Hawaiian"},
    {"SKW", "SkyWest"},
    {"RPA", "Republic"},
    {"ENY", "Envoy Air"},
    {"PDT", "Piedmont"},
    {"JIA", "PSA"},
    {"CPZ", "Compass"},
    {"ACA", "Air Canada"},
    {"WJA", "WestJet"},
    {"TSC", "Air Transat"},
    {"AMX", "Aeromexico"},
    {"VIV", "VivaAerobus"},
    {"VOI", "Volaris"},

    // === EUROPE ===
    {"BAW", "British Airways"},
    {"EZY", "easyJet"},
    {"RYR", "Ryanair"},
    {"DLH", "Lufthansa"},
    {"AFR", "Air France"},
    {"KLM", "KLM"},
    {"IBE", "Iberia"},
    {"VLG", "Vueling"},
    {"AZA", "ITA Airways"},
    {"SAS", "SAS"},
    {"FIN", "Finnair"},
    {"NAX", "Norwegian"},
    {"WZZ", "Wizz Air"},
    {"EWG", "Eurowings"},
    {"SWR", "Swiss"},
    {"AUA", "Austrian"},
    {"BEL", "Brussels"},
    {"TAP", "TAP Portugal"},
    {"AEE", "Aegean"},
    {"LOT", "LOT Polish"},
    {"CSA", "Czech Airlines"},
    {"THY", "Turkish"},
    {"AFL", "Aeroflot"},
    {"EIN", "Aer Lingus"},
    {"VIR", "Virgin Atlantic"},
    {"TOM", "TUI"},
    {"EJU", "easyJet Europe"},
    {"BER", "French Bee"},
    {"TVF", "Transavia France"},
    {"VOE", "Volotea"},

    // === MIDDLE EAST ===
    {"UAE", "Emirates"},
    {"ETD", "Etihad"},
    {"QTR", "Qatar Airways"},
    {"SVA", "Saudia"},
    {"MEA", "Middle East Air"},
    {"GFA", "Gulf Air"},
    {"OMA", "Oman Air"},
    {"KAC", "Kuwait Airways"},
    {"RJA", "Royal Jordanian"},
    {"ELY", "El Al"},
    {"FDB", "flydubai"},
    {"AXB", "Air India Express"},
    {"IAW", "Iraqi Airways"},
    {"IRA", "Iran Air"},

    // === ASIA ===
    {"AIC", "Air India"},
    {"IGO", "IndiGo"},
    {"SEJ", "SpiceJet"},
    {"CCA", "Air China"},
    {"CES", "China Eastern"},
    {"CSN", "China Southern"},
    {"CHH", "Hainan Airlines"},
    {"CSZ", "Shenzhen Air"},
    {"CXA", "Xiamen Air"},
    {"CPA", "Cathay Pacific"},
    {"HDA", "HK Airlines"},
    {"EVA", "EVA Air"},
    {"CAL", "China Airlines"},
    {"ANA", "All Nippon"},
    {"JAL", "Japan Airlines"},
    {"JJP", "Jetstar Japan"},
    {"APJ", "Peach"},
    {"KAL", "Korean Air"},
    {"AAR", "Asiana"},
    {"JNA", "Jin Air"},
    {"TWB", "T'way Air"},

    // === SOUTHEAST ASIA ===
    {"SIA", "Singapore Air"},
    {"THA", "Thai Airways"},
    {"MAS", "Malaysia Air"},
    {"AXM", "AirAsia"},
    {"GIA", "Garuda"},
    {"PAL", "Philippine Air"},
    {"CEB", "Cebu Pacific"},
    {"HVN", "Vietnam Airlines"},
    {"VJC", "VietJet"},
    {"JSA", "Jetstar Asia"},
    {"AIQ", "AirAsia India"},
    {"SLK", "Silk Air"},

    // === AFRICA ===
    {"SAA", "South African"},
    {"ETH", "Ethiopian"},
    {"MSR", "EgyptAir"},
    {"KQA", "Kenya Airways"},
    {"RAM", "Royal Air Maroc"},
    {"RWD", "RwandAir"},
    {"NML", "Air Namibia"},

    // === SOUTH AMERICA ===
    {"LAN", "LATAM Chile"},
    {"TAM", "LATAM Brasil"},
    {"AVA", "Avianca"},
    {"GLO", "Gol"},
    {"ARG", "Aerolineas Arg."},
    {"CMP", "Copa Airlines"},
    {"BOV", "Boliviana"},

    // === OCEANIA ===
    {"QFA", "Qantas"},
    {"VOZ", "Virgin Australia"},
    {"JST", "Jetstar"},
    {"ANZ", "Air New Zealand"},
    {"FJI", "Fiji Airways"},

    // === CARGO ===
    {"FDX", "FedEx"},
    {"UPS", "UPS"},
    {"GTI", "Atlas Air"},
    {"CLX", "Cargolux"},
    {"ABW", "AirBridgeCargo"},
    {"CAO", "Air China Cargo"},
    {"BOX", "Aerologic"},
    {"KER", "Kalitta Air"},
};

#define NUM_AIRLINES (sizeof(airlines) / sizeof(airlines[0]))

// Decode callsign to airline name.
// Callsigns are typically 3-letter ICAO code + flight number.
// Returns airline name or NULL if not found.
static inline const char *airline_lookup(const char *callsign) {
    if (!callsign || !callsign[0] || !callsign[1] || !callsign[2]) return NULL;

    for (int i = 0; i < (int)NUM_AIRLINES; i++) {
        if (airlines[i].icao[0] == callsign[0] &&
            airlines[i].icao[1] == callsign[1] &&
            airlines[i].icao[2] == callsign[2])
            return airlines[i].name;
    }
    return NULL;
}

#endif
