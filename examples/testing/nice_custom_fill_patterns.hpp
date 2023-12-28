// This file contains example set of nice patterns you can use with 
// `drawPatternFill` function. By the way, you can ease access by adding
// `using namespace LC7981::NiceCustomFillPatterns;` in block of code.
// See full example source code for more details.

namespace LC7981
{

namespace NiceCustomFillPatterns
{

/* Gray scale pattern */
// Gray scale patterns (`grayscale_X`) aren't the only possible patterns for 
// achieving the stated gray scale ratio (X per 16 pixels), those are examples.

/// Pattern for gray scale with 0:16 ratio (white).
/// Use `drawWhiteFill` as it's more efficient. 
/// This entry is only for example and symmetry.
const uint8_t PROGMEM grayscale_0[] = {
	0b0,
	0b00000000,
};

/// Pattern for gray scale with 1:16 ratio.
const uint8_t PROGMEM grayscale_1[] = {
	0b11,
	0b00000000,
	0b01000000,
	0b00000000,
	0b00000100,
};

/// Pattern for gray scale with 2:16 ratio.
const uint8_t PROGMEM grayscale_2[] = {
	0b11,
	0b10001000,
	0b00000000,
	0b00100010,
	0b00000000,
};

/// Pattern for gray scale with 3:16 ratio.
const uint8_t PROGMEM grayscale_3[] = {
	0b11,
	0b10001000,
	0b00000000,
	0b10101010,
	0b00000000,
};

/// Pattern for gray scale with 4:16 ratio.
const uint8_t PROGMEM grayscale_4[] = {
	0b11,
	0b00000000,
	0b01010101,
	0b00000000,
	0b10101010,
};

/// Pattern for gray scale with 5:16 ratio (honeycomb).
const uint8_t PROGMEM grayscale_5[] = {
	0b11,
	0b01000100,
	0b00111000,
	0b01000100,
	0b10000011,
};

/// Pattern for gray scale with 6:16 ratio.
const uint8_t PROGMEM grayscale_6[] = {
	0b11,
	0b10001000,
	0b01010101,
	0b00100010,
	0b01010101,
};

/// Pattern for gray scale with 7:16 ratio.
const uint8_t PROGMEM grayscale_7[] = {
	0b11,
	0b10100010,
	0b01010101,
	0b00101010,
	0b01010101,
};

/// Pattern for gray scale with 8:16 ratio (gray).
/// Use `drawGrayFill` as it's more efficient. 
/// This entry is only for example and symmetry.
const uint8_t PROGMEM grayscale_8[] = {
	0b1,
	0b01010101,
	0b10101010,
};

/// Pattern for gray scale with 9:16 ratio.
const uint8_t PROGMEM grayscale_9[] = {
	0b11,
	0b01011101,
	0b10101010,
	0b11010101,
	0b10101010,
};

/// Pattern for gray scale with 10:16 ratio.
const uint8_t PROGMEM grayscale_10[] = {
	0b11,
	0b01110111,
	0b10101010,
	0b11011101,
	0b10101010,
};

/// Pattern for gray scale with 11:16 ratio (honeycomb).
const uint8_t PROGMEM grayscale_11[] = {
	0b11,
	0b10111011,
	0b11000111,
	0b10111011,
	0b01111100,
};

/// Pattern for gray scale with 12:16 ratio.
const uint8_t PROGMEM grayscale_12[] = {
	0b11,
	0b11111111,
	0b10101010,
	0b11111111,
	0b01010101,
};

/// Pattern for gray scale with 13:16 ratio.
const uint8_t PROGMEM grayscale_13[] = {
	0b11,
	0b01110111,
	0b11111111,
	0b01010101,
	0b11111111,
};

/// Pattern for gray scale with 14:16 ratio.
const uint8_t PROGMEM grayscale_14[] = {
	0b11,
	0b01110111,
	0b11111111,
	0b11011101,
	0b11111111,
};

/// Pattern for gray scale with 15:16 ratio.
const uint8_t PROGMEM grayscale_15[] = {
	0b11,
	0b11111111,
	0b10111111,
	0b11111111,
	0b11111011,
};

/// Pattern for gray scale with 0:16 ratio (black).
/// Use `drawBlackFill` as it's more efficient. 
/// This entry is only for example and symmetry.
const uint8_t PROGMEM grayscale_16[] = {
	0b0,
	0b11111111,
};



/* Other gray patterns */

// Gray big mesh
const uint8_t PROGMEM gray_big[] = {
	0b11,
	0b00110011,
	0b00110011,
	0b11001100,
	0b11001100,
};

// Gray wide mesh
const uint8_t PROGMEM gray_wide[] = {
	0b11,
	0b00110011,
	0b11001100,
	0b00110011,
	0b11001100,
};

// Gray tall mesh
const uint8_t PROGMEM gray_tall[] = {
	0b11,
	0b01010101,
	0b01010101,
	0b10101010,
	0b10101010,
};



/* Lines patterns */

// Lines horizontally
const uint8_t PROGMEM lines_horizontally[] = {
	0b11,
	0b00000000,
	0b11111111,
	0b00000000,
	0b11111111,
};

// Lines vertically
const uint8_t PROGMEM lines_vertically[] = {
	0b0,
	0b01010101,
};

// Lines horizontally thick
const uint8_t PROGMEM lines_horizontally_thick[] = {
	0b11,
	0b00000000,
	0b11111111,
	0b11111111,
	0b00000000,
};

// Lines vertically thick
const uint8_t PROGMEM lines_vertically_thick[] = {
	0b0,
	0b01100110,
};

// Lines falling to left
const uint8_t PROGMEM lines_left[] = {
	0b11,
	0b00010001,
	0b00100010,
	0b01000100,
	0b10001000,
};

// Lines falling to right
const uint8_t PROGMEM lines_right[] = {
	0b11,
	0b10001000,
	0b01000100,
	0b00100010,
	0b00010001,
};



/* Waves patterns */

// Waves horizontally
const uint8_t PROGMEM waves_horizontally[] = {
	0b11,
	0b01100000,
	0b10010000,
	0b00001001,
	0b00000110,
};

// Waves vertically
const uint8_t PROGMEM waves_vertically[] = {
	0b111,
	0b01000100,
	0b10001000,
	0b10001000,
	0b01000100,
	0b00100010,
	0b00010001,
	0b00010001,
	0b00100010,
};

// Waves horizontally thick
const uint8_t PROGMEM waves_horizontally_thick[] = {
	0b11,
	0b01100110,
	0b11110000,
	0b10011001,
	0b00001111,
};

// Waves vertically thick
const uint8_t PROGMEM waves_vertically_thick[] = {
	0b111,
	0b01100110,
	0b11001100,
	0b11001100,
	0b01100110,
	0b00110011,
	0b10011001,
	0b10011001,
	0b00110011,
};

// Waves falling to left
const uint8_t PROGMEM waves_left[] = {
	0b111,
	0b00000001,
	0b00000001,
	0b00000001,
	0b00001110,
	0b01110000,
	0b10000000,
	0b10000000,
	0b10000000,
};

// Waves falling to right
const uint8_t PROGMEM waves_right[] = {
	0b111,
	0b10000000,
	0b10000000,
	0b10000000,
	0b01110000,
	0b00001110,
	0b00000001,
	0b00000001,
	0b00000001,
};

// Waves falling to left dense
const uint8_t PROGMEM waves_left_dense[] = {
	0b111,
	0b00010001,
	0b00010001,
	0b00100010,
	0b11001100,
	0b00110011,
	0b01000100,
	0b10001000,
	0b10001000,
};

// Waves falling to right dense
const uint8_t PROGMEM waves_right_dense[] = {
	0b111,
	0b10001000,
	0b10001000,
	0b01000100,
	0b00110011,
	0b11001100,
	0b00100010,
	0b00010001,
	0b00010001,
};

}

}
