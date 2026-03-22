# Flight3DS

Real-time flight tracker for the Nintendo 3DS. Displays live aircraft positions on a world map using data from the [OpenSky Network](https://opensky-network.org/).

## Features

- **Live flight map** on the top screen with world coastlines, grid lines, and altitude-colored aircraft
- **Airport markers** — 150+ major world airports rendered as diamonds/squares with ICAO labels at high zoom
- **Airline identification** — decodes callsigns to airline names (130+ airlines)
- **Flight list** on the bottom screen with callsign, airline, altitude, and speed
- **Detail view** with full flight info, airline name, and nearest airport
- **Offline data packs** — save named flight snapshots to SD card, load anytime without WiFi
- **Auto-cache** — last flight state cached automatically, restores on boot
- **Cursor selection** — circle pad to aim, A to select nearest flight
- **Zoom & pan** — explore any region of the world
- **Color-coded altitude** — yellow (high), orange (mid), red (low), gray (ground)
- **Heading indicators** — small ticks showing flight direction

## Controls

| Button | Action |
|--------|--------|
| D-Pad | Pan map |
| L / R | Zoom out / in |
| L + R | Open data pack browser |
| Circle Pad | Move cursor |
| A | Select nearest / detail / next flight |
| B | Back / deselect |
| X | Toggle cursor crosshair |
| Y | Save current view as data pack |
| START | Help screen |
| SELECT | Quit |

### Data Pack Browser

| Button | Action |
|--------|--------|
| D-Pad | Browse packs |
| A | Load selected pack |
| X | Delete selected pack |
| Y | Save new pack |
| B | Back to map |

## Offline Data Packs

Data packs are named snapshots of flight state saved to the SD card at `sdmc:/3ds/flight3ds/packs/`. Each pack stores:

- All tracked flights with position, altitude, speed, heading
- The map view (center coordinates and zoom level)
- Auto-detected region name
- Timestamp

Packs are saved in a compact binary format (`.f3d`) and load instantly. Use them to:
- Record interesting traffic patterns
- Share flight snapshots with friends
- Browse flight data offline on the go

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

## Embedded Databases

| Database | Entries | Purpose |
|----------|---------|---------|
| Airports | 150+ | Major world airports (ICAO, name, coords) |
| Airlines | 130+ | ICAO callsign prefix to airline name |
| Coastlines | ~500 pts | World map outlines for all continents |
| Regions | 19 | Auto-detect region from coordinates |

## Architecture

```
┌─────────────────────────┐
│    Top Screen (400x240) │
│  Map + airports + flights│
└─────────────────────────┘
┌─────────────────────────┐
│  Bottom Screen (320x240)│
│  List / detail / packs  │
└─────────────────────────┘
         │
    ┌────┴─────┐
    │ WiFi     │──→ OpenSky API (HTTPS)
    │          │←── JSON flight data
    └────┬─────┘
         │
    ┌────┴─────┐
    │ SD Card  │ ← Cache + data packs (.f3d)
    └──────────┘
```

- **Equirectangular projection** for fast lat/lon to pixel conversion
- **Embedded coastline data** (~500 points covering all continents)
- **Custom JSON parser** optimized for the OpenSky response format
- **Direct framebuffer rendering** — no console/printf, all pixel-level
- **Column-major BGR8** framebuffer layout (3DS native format)
- **Binary data pack format** for instant save/load

## License

MIT
