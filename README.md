# Custom components for ESPHome
My collection of custom components for ESPHome

## AC101
Based on https://github.com/Yveaux/AC101  
Headphone output is working, tested with `media_player` and `speaker`.
Microphone is working as well!


## Notes
### Stream audio to speaker component
This command streams an audio file to ESPHome using ffmpeg:
```bash
ffmpeg -re -i <some music file> -ar 16000 -f s8 udp://<ESPHome IP>:6055\?pkt_size=1024
```
