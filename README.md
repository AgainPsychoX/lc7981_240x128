
# Library to control LC7981 240x128 display for Arduino

Library aim to allow easy and efficient control over LC7981-based EW24D40 (or similar) 240x128 monochromatic display using Arduino compatible microcontroller. The repository include example Arduino project that allow test various features.



## About display

* [Sanyo LC7981 controller datasheet PDF](https://www.crystalfontz.com/controllers/Sanyo/LC7981/246/)

* [EW24D40 monochromatic display module (240x128) using LC7981 controller PDF](http://www.mstlcd.co.kr/s_source/download.php?table=es_free4&filename=EW24D40.pdf)



## Notes

### Alternative library

There already exist library for this controller/display, [u8glib](https://github.com/olikraus/u8glib) or [u8g2](https://github.com/olikraus/u8g2/), however, as those libraries support multiple other displays, they aren't fully optimized for handling this exact display - in result, they are using more memory (buffering image before putting on display) and processing power (small buffer require the drawing to be done 2-3 times on smaller Arduino). That was problem that forced me into developing other implementation myself.

### Display orientation, size and coords

Display orientation is specification 240 width and 128 height. Point at x = 0 and y = 0 relates to very first pixel in left top corner of the display. Last pixel have coords of x = 239 and y = 127. The display does not support changing orientation by hardware. It could be however implemented within library, with performance trade off.

In cases user try to draw on invalid coords (x > 239 or y > 127) the behavior is undefined, usually resulting in graphical glitches. The protection against invalid input could be implemented by user if necessary, no point in forcing it into the library (which could provide unnecessary overhead).

### Namespace

All code should be contained `LC7981` namespace and all defines should use `LC7981` prefix, to avoid conflicts with other libraries and user code.

### Display base class and specializations

Display management is divided into base display class, that use few virtual functions that divide I/O related code aside from providing actual features. This allow library users to define better I/O for their specific use. There is basic template (compile-time definable pins, to avoid memory usage) class `DisplayByPins` that provide a bit slow, but easy and compatible implementation for most Arduino. For more details see source code of `DisplayByPins` and example of specialization in [`examples/`](examples/) directory ([`fastio_example.hpp`](examples/testing/fastio_example.hpp)). In my example (described more in the file) it boost performance by over 4 times, so it's worth the hassle. When writing your own specialization for I/O, don't forget to use proper delays (90ns address/control setup, 140ns for reading, 220ns for writing - more details in datasheet).

### Chip select

If you are using only one display, chip select pin can be usually connected to ground resulting in the display always being selected. For `DisplayByPins` you can provide `NOT_A_PIN` option instead pin number to let the code get optimized for this case. If you are using multiple displays, they can share the data bus and Register Select and Read/Write control pins.



## Features

+ graphical mode
    + moving cursor
    + fast bulk (blocks) writing/reading,
    + single bit setting/clearing,
    + drawing lines and basic figures,
    + simple patterns fill drawing,
    + drawing vertical text with few fonts (converter script included),

Todo:

- fix problem with reading from the controller when using fast I/O example (my board specific)
- character mode
- graphical mode
    - drawing fonts (font converter with few example fonts included),
    - horizontal font drawing,
    - graphics bitmap drawing,
- minimalistic example,
- link to where buy the display,
- image of real life connections for main example,
- more interesting main example,
- benchmark in main example (compare with other libraries?),
- reference
- change orientation



## Example

Full example is included under [`example/`](example/) folder. It allow connect to Arduino via Serial port in order to send commands to draw on the display.

Below there is minimalistic example of setup and usage:

```cpp
// TODO
```


