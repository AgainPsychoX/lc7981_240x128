/*
	This example allow to test various features of the library. You can compile
	and upload this sketch, then connect serial monitor to send simple commands
	to use basic methods or run some tests. For syntax look in the code below.
*/

// #define FONT_ANY_8X16 // allows for some optimizations specific to 8x16 fonts
#include <lc7981_240x128.hpp>
#include "nice_custom_fill_patterns.hpp"
#include "font_08x16_leggibile.hpp"
#include "font_06x08_Terminal_Microsoft.hpp"

// Prepare display object using `DisplayByPins` (compile-time pin definition)
LC7981_240x128::DisplayByPins<
	// EN / CS / DI / RW
	22,  23,  20,  21,
	// DB0 to DB7
	10, 11, 12, 13, 14, 15, 18, 19
> display;

// Prepare display object using example fast I/O specialization (see README).
// #include "lc7981_240x128_fastio_example.hpp"
// MyDisplay display;

// #define DEFAULT_FONT font_08x16_leggibile
#define DEFAULT_FONT font_06x08_Terminal_Microsoft

// Display is easy to setup
void setup()
{
	Serial.begin(9600);
	Serial.println("initializing");

	display.initGraphicMode();
	display.clearWhite();
	display.drawBlackLine(0, 0, 239, 127);

	// Seed randomization, required for some tests.
	randomSeed(analogRead(A0));

	Serial.println("ready");
}

// Helper functions for dealing with serial
uint8_t forceSerialRead()
{
	while (true) {
		if (Serial.available()) {
			return Serial.read();
		}
	}
}
uint8_t forceSerialPeek()
{
	while (true) {
		if (Serial.available()) {
			return Serial.peek();
		}
	}
}

// Display does not require access to loop
void loop()
{
	if (Serial.available() > 0) {
		// Ignore spaces
		while (Serial.peek() == ' ') {
			Serial.read();
		}
		
		// Parse command op-code and arguments
		char op = forceSerialRead();
		switch (op) {
			/* MISC */
			// Read/write from raw address inside display
			case '*': {
				uint8_t x = Serial.parseInt();
				display.setCursorAddress(x);
				uint8_t y = forceSerialPeek();
				if (y == '=') {
					y = Serial.parseInt();
					display.writeSingleByte(y);
				}
				else {
					y = display.readSingleByte();
					Serial.println(y);
				}
				break;
			}
			// Set display duty
			case '%': {
				uint8_t p = 255;
				if (forceSerialPeek() == ' ') {
					p = Serial.parseInt();
				}
				display.setDisplayDuty(p);
				break;
			}

			/* BASICS */
			// Clear (white or provided pattern)
			case 'c': {
				uint8_t p = 0b00000000;
				if (forceSerialPeek() == ' ') {
					p = Serial.parseInt();
				}
				display.clear(p);
				break;
			}
			// Clear to gray
			case 'g': {
				display.clearGray();
				break;
			}
			// Horizontal line (support patterns)
			case 'h': {
				uint8_t p = forceSerialRead();
				uint8_t x = Serial.parseInt();
				uint8_t y = Serial.parseInt();
				uint8_t w = Serial.parseInt();
				switch (p) {
					case 'c': p = Serial.parseInt(); break;
					case 'w': p = 0b00000000; break;
					case 'p': p = 0b01010101; break;
					case 'd': p = 0b00110011; break;
					case 'l': p = 0b00001111; break;
					default:  p = 0b11111111; break;
				}
				display.drawHorizontalLine(x, y, w, p);
				break;
			}
			// Vertical line
			case 'v': {
				uint8_t c = 1;
				switch (forceSerialRead()) {
					case 'w': c = 0; break;
					default:  c = 1; break;
				}
				uint8_t x = Serial.parseInt();
				uint8_t y = Serial.parseInt();
				uint8_t h = Serial.parseInt();
				if (c)
					display.drawBlackVerticalLine(x, y, h);
				else
					display.drawWhiteVerticalLine(x, y, h);
				break;
			}
			// Line (any line, including horizontal/vertical)
			case 'l': {
				uint8_t c = 1;
				switch (forceSerialRead()) {
					case 'w': c = 0; break;
					default:  c = 1; break;
				}
				uint8_t x0 = Serial.parseInt();
				uint8_t y0 = Serial.parseInt();
				uint8_t x1 = Serial.parseInt();
				uint8_t y1 = Serial.parseInt();
				if (c)
					display.drawBlackLine(x0, y0, x1, y1);
				else
					display.drawWhiteLine(x0, y0, x1, y1);
				break;
			}
			// Rectangle (not filled)
			case 'r': {
				uint8_t c = 1;
				switch (forceSerialRead()) {
					case 'w': c = 0; break;
					default:  c = 1; break;
				}
				uint8_t x = Serial.parseInt();
				uint8_t y = Serial.parseInt();
				uint8_t w = Serial.parseInt();
				uint8_t h = Serial.parseInt();
				if (c)
					display.drawBlackRectangle(x, y, w, h);
				else
					display.drawWhiteRectangle(x, y, w, h);
				break;
			}
			// Filled rectangle (support patterns)
			case 'f': {
				uint8_t p = forceSerialRead();
				uint8_t x = Serial.parseInt();
				uint8_t y = Serial.parseInt();
				uint8_t w = Serial.parseInt();
				uint8_t h = Serial.parseInt();
				switch (p) {
					case 'w': display.drawWhiteFill(x, y, w, h); break;
					case 'g': display.drawGrayFill (x, y, w, h); break;
					default:  display.drawBlackFill(x, y, w, h); break;
				}
				break;
			}
			// Text
			case 't': {
				uint8_t x = Serial.parseInt();
				uint8_t y = Serial.parseInt();
				char buffer[40 + 1];
				uint8_t w = 0;
				while (w < 40) {
					uint8_t c = forceSerialPeek();
					if (c == '\n' || c == ';') {
						break;
					}
					Serial.read();
					if (c == ' ' && w == 0) {
						continue;
					}
					buffer[w] = c;
					w += 1;
				}
				buffer[w] = '\0';
				display.drawTextVertical(x, y, buffer, DEFAULT_FONT);
				break;
			}

			/* TESTS */
			// Pyramid using horizontal lines. Tests whenever horizontal line
			// drawing function can start on not aligned byte offset and
			// preserve pixels in their surrounding.
			case '^': {
				display.clear(0);
				uint8_t c = 0b11111111;
				if (forceSerialPeek() == ' ') {
					c = Serial.parseInt();
				}
				for (uint8_t y = 0; y < 120; y++) {
					const uint8_t x = (120 - y - 1);
					const uint8_t w = (y + 1) * 2;
					display.drawHorizontalLine(x, y, w, c);
				}
				break;
			}
			// Slide using vertical lines. Tests whenever vertical line drawing
			// can preserve pixels in their surrounding.
			case '\\': {
				display.clear(0);
				for (uint8_t y = 0; y < 120; y++) {
					uint8_t const x = y;
					display.drawBlackVerticalLine(x, y, 8);
				}
				break;
			}
			// Create multiple lines from selected point to points on edges.
			// Tests the rendering of custom lines.
			case 'X': {
				display.clear(0);

				uint8_t w = 64;
				uint8_t h = 32;
				if (forceSerialPeek() == 'r') {
					forceSerialRead();
					do {
						w = random();
					} while (w > 240);
					do {
						h = random();
					} while (h > 128);
				}
				else if (forceSerialPeek() == ' ') {
					w = Serial.parseInt();
					h = Serial.parseInt();
				}

				uint8_t c = 16; // line interval
				if (forceSerialPeek() == ' ') {
					c = Serial.parseInt();
				}

				uint8_t a = 0b1111; // edges selection
				if (forceSerialPeek() == ' ') {
					a = Serial.parseInt();
				}
				
				if (a & 0b0001) {
					for (int16_t y = 0; y < 128; y += c) {
						display.drawBlackLine(w, h, 240 - 1, constrain(y, 0, 128 - 1));
					}
				}
				if (a & 0b0010) {
					for (int16_t x = 240 - 1; x >= 0; x -= c) {
						display.drawBlackLine(w, h, constrain(x, 0, 240 - 1), 128 - 1);
					}
				}
				if (a & 0b0100) {
					for (int16_t y = 128 - 1; y >= 0; y -= c) {
						display.drawBlackLine(w, h, 0, constrain(y, 0, 128 - 1));
					}
				}
				if (a & 0b1000) {
					for (int16_t x = 0; x < 240; x += c) {
						display.drawBlackLine(w, h, constrain(x, 0, 240 - 1), 0);
					}
				}
				break;
			}
			// Text related tests
			case 'T': {
				const char* text = "LC7981";
				const uint8_t h = reinterpret_cast<const LC7981_240x128::font_header_t*>(DEFAULT_FONT)->height;
				for (uint8_t x = 0, y = 0; y < 128; x += 1, y += h) {
					display.drawTextVertical(0, y, String(x).c_str(), DEFAULT_FONT);
					display.drawTextVertical(x + 16, y, text, DEFAULT_FONT);
				}
				break;
			}
			// Multiple filling patterns, not much value apart of testing 
			// how nice they look. Random generated gradient included for fun.
			case '#': {
				using namespace LC7981_240x128::NiceCustomFillPatterns;
				
				// Manual smooth white to gray gradient
				display.drawPatternFill(0,   0, 15, 40, grayscale_1);
				display.drawPatternFill(15,  0, 15, 40, grayscale_2);
				display.drawPatternFill(30,  0, 15, 40, grayscale_3);
				display.drawPatternFill(45,  0, 15, 40, grayscale_4);
				display.drawPatternFill(60,  0, 15, 40, grayscale_5);
				display.drawPatternFill(75,  0, 15, 40, grayscale_6);
				display.drawPatternFill(90,  0, 15, 40, grayscale_7);
				display.drawPatternFill(105, 0, 15, 40, grayscale_8);
				display.drawPatternFill(120, 0, 15, 40, grayscale_9);
				display.drawPatternFill(135, 0, 15, 40, grayscale_10);
				display.drawPatternFill(150, 0, 15, 40, grayscale_11);
				display.drawPatternFill(165, 0, 15, 40, grayscale_12);
				display.drawPatternFill(180, 0, 15, 40, grayscale_13);
				display.drawPatternFill(195, 0, 15, 40, grayscale_14);
				display.drawPatternFill(210, 0, 15, 40, grayscale_15);
				display.drawPatternFill(225, 0, 15, 40, grayscale_16);

				// Randomly white to gray generated gradient
				display.setCursorAddress(240 / 8 * 40);
				display.writeStart();
				for (uint8_t y = 0; y < 20; y += 1) {
					for (uint8_t x = 8; x < 248; x += 8) {
						uint16_t a;
						uint8_t c = 0;
						a = random();
						c |= ((((a >> 0) & 0xff) < x) << 7) |
							 ((((a >> 8) & 0xff) < x) << 6);
						a = random();
						c |= ((((a >> 0) & 0xff) < x) << 5) |
							 ((((a >> 8) & 0xff) < x) << 4);
						a = random();
						c |= ((((a >> 0) & 0xff) < x) << 3) |
							 ((((a >> 8) & 0xff) < x) << 2);
						a = random();
						c |= ((((a >> 0) & 0xff) < x) << 1) |
							 ((((a >> 8) & 0xff) < x) << 0);
						display.writeNextByte(c);
					}
				}

				// Lines patterns
				display.drawPatternFill(0,   60, 15, 20, lines_horizontally);
				display.drawPatternFill(15,  60, 15, 20, lines_vertically);
				display.drawPatternFill(30,  60, 15, 20, lines_horizontally_thick);
				display.drawPatternFill(45,  60, 15, 20, lines_vertically_thick);
				display.drawPatternFill(60,  60, 15, 20, lines_left);
				display.drawPatternFill(75,  60, 15, 20, lines_right);
				display.drawPatternFill(90,  60, 15, 20, gray_wide);
				display.drawPatternFill(105, 60, 15, 20, gray_tall);

				// Waves patterns
				display.drawPatternFill(120, 60, 15, 20, waves_horizontally);
				display.drawPatternFill(135, 60, 15, 20, waves_vertically);
				display.drawPatternFill(150, 60, 15, 20, waves_horizontally_thick);
				display.drawPatternFill(165, 60, 15, 20, waves_vertically_thick);
				display.drawPatternFill(180, 60, 15, 20, waves_left);
				display.drawPatternFill(195, 60, 15, 20, waves_right);
				display.drawPatternFill(210, 60, 15, 20, waves_left_dense);
				display.drawPatternFill(225, 60, 15, 20, waves_right_dense);

				break;
			}
			// Small benchmark: Clear and draw whole screen 10 times
			case '$': {
				unsigned long timeStart = micros();
				uint8_t x = 0;
				while (x++ < 10) {
					display.clearWhite();
					display.clearBlack();
				}
				unsigned long timeEnd = micros();
				Serial.println(timeEnd - timeStart);
				break;
			}

			case '?': {
				break;
			}

			default:
				Serial.print("invalid command ");
				Serial.println((int)op);
		}

		// Ignore all arguments until new line or semi-colon
		while (true) {
			bool a = false;
			switch (Serial.read()) {
				case '\n':
				case ';':
					a = true;
					break;
			}
			if (a) {
				break;
			}
		}
	}
}