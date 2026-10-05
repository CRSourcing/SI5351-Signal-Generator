# SI5351-Signal-Generator
A signal generator using SI5351, ESP32 and an ILI9341 touch screen. Besides the basic signal generator functions it includes an AM modulator (using 2 mosfets) and test patterns for various amateur radio modes. Audio .wav files can be uploaded to the ESP32 file system via WiFi and then get played via the AM modulator (3 min. max duration). Audio format: 8000, 8 bit, mono. Audio can also come from an external signals source. Written in plain C on Arduino IDE. Additional modules can easily be added. Most hardware components are not critical, the two dual gate mosfets can be replaced with similar types. The output is square wave, in the low williwatts order. The output level can be adjusted.
This is an experimental device, not a fail safe building description.

![Alt text](/pics/20261005_114617.jpg)
![Alt text](/pics/20261005_114627.jpg)
