# SI5351-Signal-Generator
A signal generator using SI5351, ESP32 and an ILI9341 touch screen. Besides the basic signal generator functions it includes an AM modulator (using 2 mosfets). Test patterns for various amateur radio modes can be played continuously. Audio .wav files can be uploaded to the ESP32 file system via WiFi and then get played via the AM modulator (3 min. max duration). The file must be named play.wav and audio format is: 8000, 8 bit, mono. Audio can also come from an external signals source. Written in plain C on Arduino IDE. Additional test patterns can easily be added. Most hardware components are not critical, the two dual gate mosfets can be replaced with similar types. The RF output transistor can be almost any RF type with 100ma collector current, or be replaced with a FET. The output is square wave, in the low williwatts order. The output level can be adjusted.
This is an experimental device, not a fail safe building description. It requires some RF building experience.

![Alt text](/pics/20261005_114617.jpg)
![Alt text](/pics/20261005_114627.jpg)
