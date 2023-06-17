# Custom components for ESPHome
My collection of custom components for ESPHome

## AC101
Based on https://github.com/Yveaux/AC101  
Headphone output is working, tested with `media_player` and `speaker`.
Microphone is working as well!

### Important note for ESP32 AudioKit users
The AudioKit comes in three different flavours, please verify that it actually _has_ a AC101.
The AC101 uses I²C address `0x1A` while the ES3833 uses `0x10`, so ESPHome should show this
```
[19:37:27][I][i2c.arduino:069]: Results from i2c bus scan:
[19:37:27][I][i2c.arduino:075]: Found i2c device at address 0x1A
```
If you see `address 0x10` instead, your board is uses an ES3833.

## Notes
### Stream audio to speaker component
This command streams an audio file to ESPHome using ffmpeg:
```bash
ffmpeg -re -i <some music file> -ar 16000 -f s8 udp://<ESPHome IP>:6055\?pkt_size=1024
```
