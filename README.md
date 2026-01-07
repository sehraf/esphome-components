# Custom components for ESPHome
My collection of custom components for ESPHome

## How to include these components in ESPHome
There are two common ways to use the components from this repository in your ESPHome project:

1) Pull the component directly from GitHub using external_components (recommended):

```yaml
external_components:
  - source: "github://sehraf/esphome-components@main"
    components: [dew_point, ac101]
```

Replace `main` with a specific tag or commit SHA if you want to pin a release.

2) Copy files locally

If you prefer to include the component files directly in your ESPHome configuration folder, copy the component directory (for example `dew_point`) into a `custom_components` subfolder alongside your main ESPHome YAML. The layout should look like:

```
/config
  /custom_components
    /dew_point
      dew_point.h
      dew_point.cpp
  your_esp_config.yaml
```

Then use the same `sensor:` configuration shown earlier.

## AC101
Based on https://github.com/Yveaux/AC101  
Headphone output is working, tested with `media_player` and `speaker`.  
Microphone is working as well!

### Important note for ESP32 AudioKit users
The AudioKit comes in three different flavours, please verify that it actually _has_ an AC101.
The AC101 uses I²C address `0x1A` while the ES3833 uses `0x10`, so ESPHome should show this
```
[19:37:27][I][i2c.arduino:069]: Results from i2c bus scan:
[19:37:27][I][i2c.arduino:075]: Found i2c device at address 0x1A
```
If you see `address 0x10` instead, your board uses an ES3833. Also _different PINs_ are used!

### Stream audio to speaker component
This command streams an audio file to ESPHome using ffmpeg:
```bash
ffmpeg -re -i <some music file> -ar 16000 -f s8 udp://<ESPHome IP>:6055\?pkt_size=1024
```

## Dew Point
A small component that calculates the dew point from an existing temperature and humidity sensor. It publishes a sensor entity with the calculated dew point value.



