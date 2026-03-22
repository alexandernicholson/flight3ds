# Flight3DS

Real-time flight tracker for the Nintendo 3DS. Displays live aircraft positions on a world map using data from the [OpenSky Network](https://opensky-network.org/).

## Features

- **Live flight map** on the top screen with world coastlines, grid lines, and altitude-colored aircraft
- **Flight list** on the bottom screen with callsign, country, altitude, and speed
- **Detail view** with full flight info (altitude, speed, heading, vertical rate, coordinates)
- **Offline mode** — caches flight data to SD card, works without WiFi
- **Cursor selection** — circle pad to aim, A to select nearest flight
- **Zoom & pan** — explore any region of the world
- **Color-coded altitude** — yellow (high), orange (mid), red (low), gray (ground)
- **Heading indicators** — small ticks showing flight direction

## Controls

| Button | Action |
|--------|--------|
| D-Pad | Pan map |
| L / R | Zoom out / in |
| Circle Pad | Move cursor |
| A | Select nearest / detail / next flight |
| B | Back / deselect |
| X | Toggle cursor crosshair |
| Y | Force data refresh |
| START | Help screen |
| SELECT | Quit |

## Building

Requires [devkitPro](https://devkitpro.org/) with devkitARM and libctru.

```bash
export DEVKITPRO=/opt/devkitpro
export DEVKITARM=$DEVKITPRO/devkitARM
make
```

Output: `flight3ds.3dsx` — copy to your 3DS SD card under `/3ds/flight3ds/`.

## Data Source

Flight data from [OpenSky Network REST API](https://openskynetwork.github.io/opensky-api/rest.html) (free, no authentication required for basic access). Updates every 15 seconds when connected.

## Architecture

```
┌─────────────────────────┐
│    Top Screen (400x240) │
│    World map + flights  │
└─────────────────────────┘
┌─────────────────────────┐
│  Bottom Screen (320x240)│
│  Flight list / detail   │
└─────────────────────────┘
         │
    ┌────┴─────┐
    │ WiFi     │──→ OpenSky API (HTTPS)
    │          │←── JSON flight data
    └────┬─────┘
         │
    ┌────┴─────┐
    │ SD Card  │ ← Offline cache (binary)
    └──────────┘
```

- **Equirectangular projection** for fast lat/lon → pixel conversion
- **Embedded coastline data** (~500 points covering all continents)
- **Custom JSON parser** optimized for the OpenSky response format
- **Direct framebuffer rendering** — no console/printf, all pixel-level
- **Column-major BGR8** framebuffer layout (3DS native format)

## License

MIT
