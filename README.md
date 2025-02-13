# DMXCube

WiFi ArtNet V4 to DMX, with experimental RDM support

## Known Issues & Feedback

Please see the issues tab for known issues or to submit bugs or suggestions

## Getting Started

### Powering up

You can use the included USB to barrel jack adapter (only USB power is needed, no data transfer takes place), or any 5V power supply (center positive).
Once power is applied, the screen should light up after 1-2 seconds and say "booting...".
If the screen shows "emergency mode!", try reseting the device, and if the issue persists, contact support either at our issues tracker or on discord.

### First Boot

On your first boot, the device will start a WiFi hotspot called "DMXCube-00000" with a password of "DMXCube2024" (You can always check the password by pressing and holding the knob). Connect to the hotspot, then "Log in" or open 2.0.0.1 in a browser.

Note that the hotspot is, by default, only for accessing the settings page. You'll need to enable Standalone mode in the web UI if you want to send ArtNet to the device in hotspot mode.

### Web UI

Open the device's IP in the web browser (this is 2.0.0.1 in Hotspot Mode, but the display always shows the current IP)

In the Wifi tab, enter your SSID and password. Click save (wait a bit and it should go green and say Settings Saved). Now click reboot and the device should connect to your Wifi.

If the device can't connect to the wifi or get a DHCP assigned address within the specified timeout (by default 15 seconds), it will start the hotspot and wait for 30 seconds for you to connect. If a client doesn't connect to the WiFi hotspot in time, the device will restart and try again.

### Restore Factory Defaults

I have allowed for 2 methods to restore the factory default settings: Pressing the knob during power on or multiple power cycles.

Method 1: Press the encoder while the device boots, then release it after about 1-2 seconds and then short press it again to make sure the device doesn't reset accidentally. Wait for about 10 seconds to abort factory reset

Method 2: Allow the Cube about 1-4 seconds to start, then power cycle it. Do this at least 5 times to restore factory default settings.

### DMX Workshop

I have implemented as many DMX Workshop/ArtNet V4 features as I possibly could. You can change settings such as merge mode, IP address, DHCP, port addresses, and node ID (to differentiate multiple DMXCubes on the same network)  Most of these are also available via the web UI.

## Features

- sACN and ArtNet V4 support
- 2 full universes of DMX output with full RDM support for both outputs
- Up to 1360 ws2812(b) pixels - 8 full universes
- DMX/RDM out one port, ws2812(b) out the other
- DMX in - send to any ArtNet device
- Web UI with mobile support
- Web UI uses AJAX & JSON to minimize network traffic used & decrease latency
- Pixel FX - a 12 channel mode for ws2812 LED pixel control

## Pixel FX

To enable this mode, select WS2812 in the port settings and enter the number of pixels you wish to control.  Select '12 Channel FX'. 'Start Channel' is the DMX address of the first channel below.

Note: You still need to set the Artnet net, subnet and universe correctly.

| DMX Channel | Function  | Values (0-255) |                       |
| ----------- | --------- | -------------- | --------------------- |
| 1           | Intensity | 0 - 255        |                       |
| 2           | FX Select | 0 - 49         | Static                |
|             |           | 50 - 74        | Rainbow               |
|             |           | 75 - 99        | Theatre Chase         |
|             |           | 100 - 124      | Twinkle               |
| 3           | Speed     | 0 - 19         | Stop - Index Reset    |
|             |           | 20 - 122       | Slow - Fast CW        |
|             |           | 123 - 130      | Stop                  |
|             |           | 131 - 234      | Fast - Slow CCW       |
|             |           | 235 - 255      | Stop                  |
| 4           | Position  | 0 - 127 - 255  | Left - Centre - Right |
| 5           | Size      | 0 - 255        | Small - Big           |
| 6           | Colour 1  | 0 - 255        | Red                   |
| 7           |           | 0 - 255        | Green                 |
| 8           |           | 0 - 255        | Blue                  |
| 9           | Colour 2  | 0 - 255        | Red                   |
| 10          |           | 0 - 255        | Green                 |
| 11          |           | 0 - 255        | Blue                  |
| 12          | Modify    | 0 - 255        | *Modify FX            |

Modify FX is only currently used for the Static effect and is used to resize colour 1 within the overall size.

## Special Thanks To

I'd like to thank these people.  They have either contributed with donations, a large amount of testing and feedback, or with their own code/projects that have been of assistance or inspiration.

- Matthew Tong for creating the base code for our DMXCube mini
- Tristan Thiltges
- Cyprien Leduc
- Jochen Schefe
- Jan Raeymaekers
- Wiktor Kaluzny
- Jean-Michel Blouin
- Tobias Schulz
- Ben Small
- Harald Müller
- Volodymyr Bereza
- Felix Hartmann
- Ruud Leemans
- Bogumil palewicz
- Anton Manchenko
- Bogumil Palewicz
- Marcel Dolnak
- Paul Lim
- Geo Karavasilis
- [Claude Heintz](https://github.com/claudeheintz/)
- [Shelby Merrick (Forkineye)](https://github.com/forkineye)
