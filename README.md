# Train Button

A physical button that starts the model train on the [unserioes24](https://unserioes24.de)
stream. An ESP32-S3 polls the backend, lights the button's LED when the train may run,
blinks for 120 seconds after a successful press, and strobes fast when someone presses
during the cooldown.

Everything is configured from a web interface on the device — Wi-Fi, server address,
token, and every LED pattern. Nothing is compiled in, nothing is hard-coded.

```
┌──────────────┐   X-Train-Token    ┌──────────────────────────┐
│ ESP32-S3     │ ─────────────────▶ │ data.unserioes24.de      │
│ + LED button │ ◀───────────────── │ /api/train-button/status │
└──────────────┘   canPress, cooldown└─────────────────────────┘
```

## Hardware

| Part | Model |
| --- | --- |
| Board | ESP32-S3 Mini (diymore, USB-C, 4 MB flash) |
| Button | 16 mm metal push button with LED ring (PC case power switch) |

### Where to solder

The button has four wires: two for the **switch** (not polarised) and two for the
**LED** (polarised).

| Wire | Pad on the ESP32-S3 Mini | Notes |
| --- | --- | --- |
| Switch A | **GPIO 4** | either of the two switch wires |
| Switch B | **GND** | the other switch wire |
| LED + | **GPIO 5** through a 100–220 Ω resistor | usually the red/coloured wire |
| LED − | **GND** | usually the black wire |

Optional second button for resetting the device — off until you enable it under
Hardware:

| Wire | Pad | Notes |
| --- | --- | --- |
| Switch A | **GPIO 6** | enable it under Hardware |
| Switch B | **GND** | |

```
   ESP32-S3 Mini
   ┌─────────────┐
   │        GPIO4├──────────────● switch A
   │             │
   │        GPIO5├──[ 150R ]────● LED +
   │             │
   │         GND ├──────┬───────● switch B
   │             │      └───────● LED −
   └─────────────┘
```

Notes:

- The internal pull-up is enabled, so the switch simply shorts GPIO 4 to GND.
- These 16 mm buttons are built for a mainboard's 3.3 V power-LED header and normally
  carry their own resistor. The extra 100–220 Ω protects the GPIO, which should not
  source more than about 20 mA. If the ring stays dim, drop to 100 Ω.
- Needs more current, or the LED is rated 5 V/12 V? Drive it through a small N-channel
  MOSFET (AO3400, 2N7000): gate to GPIO 5, source to GND, drain to LED −, LED + to the
  matching supply rail.
- The pins are fixed in `src/Config.h` (`PIN_BUTTON`, `PIN_LED`, `PIN_RESET`, `PIN_RGB`)
  and shown read-only in the web interface. Changing one means editing that file and
  reflashing — a pin number typed into a browser can only ever break the device. If you
  do change them, keep away from GPIO 0, 19, 20, 26–32, 43, 44, 45 and 46: boot straps,
  USB, flash and UART live there.
- The board carries a tiny WS2812 RGB LED of its own (GPIO 48 on most of these boards).
  It is a separate part from the button's single-colour ring and can act as a status
  light: amber in setup mode, red without Wi-Fi, teal when the train is ready, violet
  during the cooldown. It is **off by default** — turn it on under Hardware once you
  have confirmed your board has one.

## Flashing

### Pre-built binary

Download `train-button-factory.bin` from the
[latest release](https://github.com/unserioes24/train-button/releases) and flash it at
offset `0x0`:

```bash
esptool.py --chip esp32s3 --port COM5 write_flash 0x0 train-button-factory.bin
```

Or open [ESP Web Tools](https://esphome.github.io/esp-web-tools/) in Chrome and pick the
same file — no toolchain needed.

### From source

```bash
pip install platformio
pio run -t upload
pio device monitor
```

The board has no USB-UART bridge. If it is not detected, hold **BOOT**, tap **RESET**,
release **BOOT**, then flash.

## First start

1. The device opens an open Wi-Fi access point called **TrainButton-XXXX** — no password,
   so there is nothing to look up.
2. Connect. The configuration page opens by itself (captive portal); otherwise browse to
   `http://192.168.4.1`.
3. The **Setup** page holds everything the button needs — network, Wi-Fi password, server
   address and token. Fill it in and press **Save and connect**.
4. The device restarts and joins your network. From then on it is reachable at
   `http://trainbutton.local` (or by IP — the serial monitor prints it).

The access point only exists while the device has no working Wi-Fi connection.

## Resetting

| How | What it clears |
| --- | --- |
| Hold the main button 5 s | Wi-Fi network and password, reopens setup mode |
| Hold the reset button 8 s (if wired) | Wi-Fi, password, token and server address |
| **Reset Wi-Fi, token and server** in the web interface | same as the reset button |
| **Factory reset** in the web interface | everything, including the LED patterns |

Both hold times are adjustable under Hardware.

## What the LED does

| Situation | Default | Configurable |
| --- | --- | --- |
| Train may run | solid | solid / breathe / blink / off, brightness, rate |
| Cooldown running | off | same set, plus its own brightness |
| Press accepted | blinks 120 s | duration, rate, brightness |
| Press refused (429) or server error | fast blink 3 s | duration, rate, brightness |
| Setup mode | slow breathe | — |
| No Wi-Fi or no server | slow blink | — |

With **blink immediately** enabled (default), a press during the cooldown strobes right
away instead of waiting for the round trip — the device already knows the lock state from
its last poll. The server always stays the authority: the request goes out either way and
its answer wins.

## Backend API

Base URL is configurable; the device appends the paths itself.

| Method | Endpoint | Auth |
| --- | --- | --- |
| `GET` | `/api/train-button/status` | `X-Train-Token` header |
| `POST` | `/api/train-button/press` | `X-Train-Token` header |

```jsonc
// GET /api/train-button/status
{ "username": "kanzla23", "canPress": true, "secondsUntilReady": 0,
  "lastUsername": null, "lastPressedAt": null }

// POST /api/train-button/press  → 200
{ "success": true, "username": "kanzla23", "pressedAt": 1723400000, "secondsRemaining": 0 }

// POST /api/train-button/press  → 429 (cooldown)
{ "success": false, "username": "kanzla23", "pressedAt": null, "secondsRemaining": 2847 }
```

## Device API

The web interface talks to the device over these routes.

| Method | Route | Purpose |
| --- | --- | --- |
| `GET` | `/api/state` | live status, Wi-Fi, LED, last press |
| `GET` | `/api/config` | settings (token masked) |
| `POST` | `/api/config` | partial update |
| `GET` | `/api/scan` | Wi-Fi scan |
| `POST` | `/api/press` | queue a press |
| `POST` | `/api/led-test` | preview a pattern on the hardware |
| `POST` | `/api/test-connection` | one-off backend check |
| `POST` | `/api/reset-credentials` | clear Wi-Fi, token and server address |
| `POST` | `/api/reboot`, `/api/factory-reset` | maintenance |

## Security

- The Wi-Fi password and the backend token live in NVS on the device. Neither is in this
  repository, and `GET /api/config` returns the token only as its last four characters.
- The setup access point is open on purpose — a factory device cannot hand you a password.
  It is only up while no Wi-Fi is configured, and anyone within radio range during those
  minutes could reach the configuration page. Set it up somewhere you trust.
- The web interface has no login. Anyone on your network who can reach the device can
  reconfigure it, so keep it on a network you trust and never port-forward it.
- TLS certificates are not verified. The token is the only secret sent to the backend, and
  certificate pinning would break the device on every renewal. Do not put anything else
  behind that token.

## Project layout

```
src/
  main.cpp        state machine, button handling, network task
  Config.*        settings struct, NVS persistence, range clamping
  LedEngine.*     non-blocking PWM patterns
  TrainApi.*      backend client
  WebPortal.*     HTTP routes, captive portal
  WebPage.h       the web interface (single self-contained page)
```

The web interface is one PROGMEM string with no external assets, so it also loads in setup
mode where the device has no internet connection.

## License

MIT — see [LICENSE](LICENSE).
