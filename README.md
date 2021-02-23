HX711 Bela example program
=====

Bela HX711 weight scale interface

this thing interfaces with 24-Bit Analog-to-Digital Converter (ADC) for Weigh Scales

Build and run 
---
```
gcc  -o HX711 HX711.cpp gb_common.h
```

To actually be able to convert to any usable reading, you would have to calibrate the scale 
using a known weight. Easiest thing to do is to use water and a measurement cup. Once you have a few readings at uniform increments
you can determine how heavy "1" is. 

at the top of HX711.cpp there two defines:

```
#define CLOCK_PIN       59
#define DATA_PIN        50
```

these map to Bela mini's D0 and D1 pin (there is a pinout in the Bela docs).

The shield comes defaulted to 10 samples per second. To switch to 80 samples per second, one has to desolder the RATE pin from the pad it's soldered to, lift it and solder it to +vcc on HX711 shield (pin 15). 
By default, this pin is grounded, which sets HX711 to run at 10 samples per second
