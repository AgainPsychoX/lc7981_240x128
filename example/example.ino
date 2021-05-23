/*
	This example allow to test various features of the library. You can compile
	and upload this sketch, then connect serial monitor to send simple commands
	to use basic methods or run some tests. For syntax look in the code below.
*/

#include <lc7981_240x128.hpp>

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