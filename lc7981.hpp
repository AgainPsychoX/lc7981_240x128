#pragma once

#include <Arduino.h>

namespace LC7981
{

struct font_header_t {
	uint8_t width;
	uint8_t height;
};

enum register_t {
	Data = 0,	// RS = LOW
	Command = 1	// RS = HIGH 
};

/// Display class base. The other class should be extend it providing basic IO.
class DisplayBase
{
	/* Virtual methods to be provided by specialized class, to allow custom IO. */
protected:
	/// Write byte to register.
	virtual void write(const register_t reg, const uint8_t val);

	/// Read byte from register.
	virtual uint8_t read(const register_t reg);

	/// Prepare display to receiving commands and data.
	virtual void init();



	/* Fancier way of accessing write/read methods */
	/// Write byte to register.
	template <register_t reg>
	inline void write(const uint8_t val)
	{
		this->write(reg, val);
	}

	/// Read byte from register.
	template <register_t reg>
	inline uint8_t read()
	{
		return this->read(reg);
	}
	



	/* Variables and flags */
public:
	const uint8_t width;
	const uint8_t height;
protected:
	struct {
		/// Flag to keep track of dummy read required for reading data after moving cursor.
		bool needDummyRead : 1;
	};



	/* Initializers */
public:
	/// Constructor
	DisplayBase(uint8_t width, uint8_t height) 
		: width(width), height(height)
	{}
	DisplayBase() : DisplayBase(240, 128) {}

	/// Prepare display to use graphical mode.
	void initGraphicMode()
	{
		// Prepare display to receiving commands and data
		this->init();

		// Set mode register to display ON, master mode, graphic mode
		write<Command>(0b0000);
		write<Data>(0b00110010);

		// Set chars/bits per pixel to use 8 bits of 1 byte to display 8 dots
		write<Command>(0b0001);
		write<Data>(0b00000111);

		// Set width of the screen
		write<Command>(0b0010);
		write<Data>(width / 8 - 1);

		// Set display duty to max
		write<Command>(0b0011);
		write<Data>(127);

		// Set display start lower address
		write<Command>(0b1000);
		write<Data>(0);

		// Set display start upper address
		write<Command>(0b1001);
		write<Data>(0);
	}



	/* Basic methods */
public:
	/// Move data read/write cursor to address inside display.
	void setCursorAddress(uint16_t address)
	{
		write<Command>(0b1010); // Set cursor lower address
		write<Data>(address & 0xff);
		write<Command>(0b1011); // Set cursor upper address
		write<Data>(address >> 8);
		needDummyRead = true;
	}

	/// Start writing.
	inline void writeStart()
	{
		write<Command>(0b1100); // Write display data
	}
	/// Write next byte (after writing started).
	inline void writeNextByte(uint8_t value)
	{
		write<Data>(value);
	}
	/// Write single byte.
	inline void writeSingleByte(uint8_t value)
	{
		writeStart();
		writeNextByte(value);
	}

	/// Start reading.
	void readStart()
	{
		if (needDummyRead) {
			needDummyRead = false;
			write<Command>(0b1101);
			read<Data>();
		}
		write<Command>(0b1101); // Read display data
	}
	/// Read next byte (after writing started).
	inline uint8_t readNextByte()
	{
		return read<Data>();
	}
	/// Read single byte.
	inline uint8_t readSingleByte()
	{
		readStart();
		return readNextByte();
	}

	/// Set bit in next byte.
	inline void setDataBit(const uint8_t which)
	{
		write<Command>(0b1111);
		write<Data>(which);
	}
	/// Clear bit in next byte.
	inline void clearDataBit(const uint8_t which) {
		write<Command>(0b1110);
		write<Data>(which);
	}
	/// Set or clear bit in byte depending on requested color.
	inline void setDataBit(const uint8_t which, const bool black)
	{
		write<Command>(0b1110 | black);
		write<Data>(which);
	}

	/// Set display duty to `1 / (value + 1)` (from 1:1 to 1:127). 
	/// Note: The LC7981 specifies up to 256 divider, but it seems to glitch.
	inline void setDisplayDuty(const uint8_t value)
	{
		write<Command>(0b0011);
		write<Data>(value);
	}



	/* Basic drawing */
public:
	/// Clear whole display using specified pattern.
	void clear(const uint8_t pattern)
	{
		setCursorAddress(0);
		writeStart();
		for (uint8_t y = 0; y < height; y++) {
			for (uint8_t x = 0; x < width; x += 8) {
				writeNextByte(pattern);
			}
		}
	}
	/// Clear whole display white (empty).
	inline void clearWhite()
	{
		clear(0);
	}
	/// Clear whole display black (filled).
	inline void clearBlack()
	{
		clear(0b11111111);
	}
	/// Clear whole display gray (alternating bits pattern).
	void clearGray()
	{
		setCursorAddress(0);
		writeStart();
		for (uint8_t y = 0; y < height; y += 2) {
			for (uint8_t x = 0; x < width; x += 8) {
				writeNextByte(0b10101010);
			}
			for (uint8_t x = 0; x < width; x += 8) {
				writeNextByte(0b01010101);
			}
		}
	}

	/// Set single bit at given coordinates.
	/// For multiple bits you should more efficient methods than `setPixel` or `clearPixel`.
	inline void setPixel(const uint8_t x, const uint8_t y)
	{
		setCursorAddress(width / 8 * y + x / 8);
		setDataBit(x % 8);
	}
	/// Clear single bit at given coordinates.
	/// For multiple bits you should more efficient methods than `setPixel` or `clearPixel`.
	inline void clearPixel(const uint8_t x, const uint8_t y)
	{
		setCursorAddress(width / 8 * y + x / 8);
		clearDataBit(x % 8);
	}
	/// Set or clear single bit at given coordinates depending on requested value.
	/// For multiple bits you should more efficient methods than `setPixel` or `clearPixel`.
	inline void setPixel(const uint8_t x, const uint8_t y, const bool black)
	{
		setCursorAddress(width / 8 * y + x / 8);
		setDataBit(x % 8, black);
	}

	/// Draw horizontal line from specified point of specified length using specified pattern.
	void drawHorizontalLine(const uint8_t x, const uint8_t y, const uint8_t length, const uint8_t pattern)
	{
		setCursorAddress(width / 8 * y + x / 8);
		uint8_t remainingLength = length;
		uint8_t p = x % 8;
		if (p != 0) {
			uint8_t mask;
			if (length + p >= 8) {
				mask = 0b11111111 << p;
				remainingLength -= (8 - p);
			}
			else {
				mask = 0;
				while (remainingLength) {
					mask <<= 1;
					mask |= 1;
					remainingLength -= 1;
				}
				mask <<= p;
			}
			uint8_t current = readSingleByte();
			setCursorAddress(width / 8 * y + x / 8);
			writeStart();
			writeNextByte((pattern & mask) | (current & ~mask));
		}
		else {
			writeStart();
		}
		while (remainingLength >= 8) {
			writeNextByte(pattern);
			remainingLength -= 8;
		}
		if (remainingLength > 0) {
			uint8_t mask = 0;
			while (remainingLength) {
				mask <<= 1;
				mask |= 1;
				remainingLength -= 1;
			}
			uint8_t current = readSingleByte();
			setCursorAddress(width / 8 * y + (x + length) / 8);
			writeSingleByte((pattern & mask) | (current & ~mask));
		}
	}
	/// Draw black horizontal line from specified point of specified length.
	inline void drawBlackHorizontalLine(const uint8_t x, const uint8_t y, const uint8_t length)
	{
		drawHorizontalLine(x, y, length, 0b11111111);
	}
	/// Draw white horizontal line from specified point of specified length.
	inline void drawWhiteHorizontalLine(const uint8_t x, const uint8_t y, const uint8_t length)
	{
		drawHorizontalLine(x, y, length, 0b00000000);
	}

	/// Draw black vertical line from specified point of specified length.
	void drawBlackVerticalLine(const uint8_t x, const uint8_t y, const uint8_t length)
	{
		for (uint8_t i = y; i < y + length; i++) {
			setCursorAddress(width / 8 * i + x / 8);
			setDataBit(x % 8);
		}
	}
	/// Draw white vertical line from specified point of specified length.
	void drawWhiteVerticalLine(const uint8_t x, const uint8_t y, const uint8_t length)
	{
		for (uint8_t i = y; i < y + length; i++) {
			setCursorAddress(width / 8 * i + x / 8);
			clearDataBit(x % 8);
		}
	}

	/// Draw line from specified point of specified length using white or black.
	void drawLine(uint8_t x0, uint8_t y0, const uint8_t x1, const uint8_t y1, const bool black)
	{
		if (x0 > x1) {
			return drawLine(x1, y1, x0, y0, black);
		}
		
		const uint8_t dx = x1 - x0;
		if (y0 == y1) {
			return drawHorizontalLine(x0, y0, dx + 1, black ? 0b11111111 : 0);
		}
		
		if (y1 > y0) {
			const uint8_t dy = y1 - y0;
			if (dx == 0) {
				if (black) {
					return drawWhiteVerticalLine(x0, y0, dy + 1);
				}
				else {
					return drawBlackVerticalLine(x0, y0, dy + 1);
				}
			}

			int16_t err = dx - dy;
			while (true) {
				setPixel(x0, y0, black);
				if (x0 == x1 && y0 == y1) {
					break;
				}
				
				const int16_t e2 = 2 * err;
				if (-e2 <= dy) {
					err -= dy;
					x0 += 1;
				}
				if (e2 <= dx) {
					err += dx;
					y0 += 1;
				}
			}
		}
		else {
			const uint8_t dy = y0 - y1;
			if (dx == 0) {
				if (black) {
					return drawWhiteVerticalLine(x1, y1, dy + 1);
				}
				else {
					return drawBlackVerticalLine(x1, y1, dy + 1);
				}
			}

			int16_t err = dx - dy;
			while (true) {
				setPixel(x0, y0, black);
				if (x0 == x1 && y0 == y1) {
					break;
				}
				
				const int16_t e2 = 2 * err;
				if (-e2 <= dy) {
					err -= dy;
					x0 += 1;
				}
				if (e2 <= dx) {
					err += dx;
					y0 -= 1;
				}
			}
		}
	}
	/// Draw black line from specified point of specified length.
	inline void drawBlackLine(uint8_t x0, uint8_t y0, const uint8_t x1, const uint8_t y1)
	{
		drawLine(x0, y0, x1, y1, true);
	}
	/// Draw white line from specified point of specified length.
	inline void drawWhiteLine(uint8_t x0, uint8_t y0, const uint8_t x1, const uint8_t y1)
	{
		drawLine(x0, y0, x1, y1, false);
	}



	/* Basic shapes */
public:
	/// Draw black rectangle on give point with given size.
	void drawBlackRectangle(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h)
	{
		drawBlackHorizontalLine(x, y, w);
		drawBlackHorizontalLine(x, y + h - 1, w);
		drawBlackVerticalLine(x, y + 1, h - 2);
		drawBlackVerticalLine(x + w - 1, y + 1, h - 2);
	}
	/// Draw white rectangle on give point with given size.
	void drawWhiteRectangle(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h)
	{
		drawWhiteHorizontalLine(x, y, w);
		drawWhiteHorizontalLine(x, y + h - 1, w);
		drawWhiteVerticalLine(x, y + 1, h - 2);
		drawWhiteVerticalLine(x + w - 1, y + 1, h - 2);
	}

	/// Draw filled black rectangle on give point with given size.
	inline void drawBlackFill(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h)
	{
		for (uint8_t i = y; i < y + h; i++) {
			drawBlackHorizontalLine(x, i, w);
		}
	}
	/// Draw filled white rectangle on give point with given size.
	inline void drawWhiteFill(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h)
	{
		for (uint8_t i = y; i < y + h; i++) {
			drawWhiteHorizontalLine(x, i, w);
		}
	}
	/// Draw filled gray rectangle on give point with given size.
	inline void drawGrayFill(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h)
	{
		static const uint8_t PROGMEM pattern[] = {
			0b1,
			0b01010101,
			0b10101010,
		};
		drawPatternFill(x, y, w, h, pattern);
	}

	/// Draw custom pattern filling rectangle from given point with given size.
	/// Pattern should be provided as pointer to PROGMEM array of bytes, where 
	/// first value have value of `number of rows - 1`, followed by next rows 
	/// bytes. Pattern width is 8 bits, number of rows must be power of two.
	/// See `example/nice_custom_fill_patterns.hpp` for details and examples.
	void drawPatternFill(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const uint8_t* pattern)
	{
		const uint8_t mask = pgm_read_byte(pattern + 0);
		const uint8_t limit = y + h; 
		uint8_t i = y;
		uint8_t p;
		do {
			p = pgm_read_byte(pattern + (i & mask) + 1);
			drawHorizontalLine(x, i, w, p);
			i += 1;
		}
		while (i < limit);
	}

	// TODO: draw<*>Fill could be optimized - do mask calculation only once, instead per line.




	/* Text */
public:
#ifdef FONT_ANY_8X16
	/// Draw text vertically using selected font, assuming font is 8x16 (special fast case)
	void drawTextVertical_8x16(const uint8_t x, uint8_t y, const char* string, const void* font) {
		if (!*string) return;
		const font_header_t* fontHeader = static_cast<const font_header_t*>(font);
		const uint8_t* fontData = static_cast<const uint8_t*>(font + sizeof(font_header_t));
		const uint8_t p = x % 8; // bitsOffset
		if (p != 0) {
			const char* pointer;
			for (uint8_t i = 0; i < 16; i++) {
				pointer = string;

				// First block
				uint8_t mask = 0b11111111 << p;
				setCursorAddress(width / 8 * y + x / 8);
				uint8_t current = readSingleByte();
				uint8_t prev = pgm_read_byte(fontData + (*pointer - ' ') * 16 + i);
				pointer += 1;
				setCursorAddress(width / 8 * y + x / 8);
				writeStart();
				// p == 3, prev == hgfedcba : (prev << p) == edcba???
				writeNextByte((current & ~mask) | (prev << p));

				// Middle blocks
				while (*pointer) {
					uint8_t next = pgm_read_byte(fontData + (*pointer - ' ') * 16 + i);
					pointer += 1;
					// if p == 3, prev == hgfedcba, next == HGFEDCBA :
					//   (prev >> (8 - p)) == ?????hgf, (next << p) == EDCBA???
					writeNextByte((prev >> (8 - p)) | (next << p));
					prev = next;
				}

				// Last block
				current = readSingleByte();
				setCursorAddress(width / 8 * y + x / 8 + static_cast<size_t>(pointer - string));
				writeStart();
				writeNextByte((prev >> (8 - p)) | (current & mask));

				y += 1;
			}
		}
		else {
			const char* pointer;
			for (uint8_t i = 0; i < 16; i++) {
				pointer = string;
				setCursorAddress(width / 8 * y + x / 8);
				writeStart();
				while (*pointer) {
					writeNextByte(pgm_read_byte(fontData + (*pointer - ' ') * 16 + i));
					pointer += 1;
				}
				y += 1;
			}
		}
	}
#endif

	/// Draw text vertically using selected font, assuming font width is 8 bits or narrower.
	/// Font chars rows bits are required to be padded with zeros while narrower than 8 bits.
	void drawTextVertical_narrow(const uint8_t x, uint8_t y, const char* string, const void* font) {
		const auto fontWidth  = static_cast<const font_header_t*>(font)->width;
		const auto fontHeight = static_cast<const font_header_t*>(font)->height;
		const uint8_t* fontData = static_cast<const uint8_t*>(font + sizeof(font_header_t));
		const uint8_t fontRowBytes = fontHeight;
		const uint8_t bitsOffset = x % 8;
		const char* pointer;
		for (uint8_t r = 0; r < fontHeight; r++) {
			pointer = string;

			uint8_t bitsPending = 0; // to be written
			uint8_t nextByte = 0; // buffer for bits to be yet written

			// Read background for first block if not aligned start
			if (bitsOffset) {
				setCursorAddress(width / 8 * y + x / 8);
				nextByte = (readSingleByte() & ~(0b11111111 << bitsOffset));
				bitsPending = bitsOffset;
			}

			// Process next blocks
			setCursorAddress(width / 8 * y + x / 8);
			writeStart();
			while (*pointer) {
				const uint8_t data = pgm_read_byte(fontData + (*pointer - ' ') * fontRowBytes + r); // & mask

				nextByte |= data << bitsPending;
				bitsPending += fontWidth;

				if (bitsPending >= 8) {
					writeNextByte(nextByte);
					bitsPending -= 8;
					nextByte = data >> (fontWidth - bitsPending);
				}

				pointer += 1;
			}

			// Write last block, with background, if not aligned end
			if (bitsPending > 0) {
				nextByte |= readSingleByte() & (0b11111111 << bitsPending);
				const uint8_t widthTotal = static_cast<size_t>(pointer - string) * fontWidth + bitsOffset;
				setCursorAddress(width / 8 * y + x / 8 + widthTotal / 8);
				writeStart();
				writeNextByte(nextByte);
			}

			y += 1;
		}
	}

	/// Draw text vertically using selected font, assuming font width is above 8 bits.
	/// Font chars rows bits should be connected and padded only to avoid mixing characters.
	void drawTextVertical_wide(const uint8_t x, uint8_t y, const char* string, const void* font) {
		const auto fontWidth  = static_cast<const font_header_t*>(font)->width;
		const auto fontHeight = static_cast<const font_header_t*>(font)->height;
		const uint8_t* fontData = static_cast<const uint8_t*>(font + sizeof(font_header_t));
		const uint8_t fontRowBytes = (fontWidth * fontHeight + 7) / 8;
		const uint8_t bitsOffset = x % 8;
		const char* pointer;
		for (uint8_t r = 0; r < fontHeight; r++) {
			pointer = string;

			uint8_t bitsPending = 0; // to be written
			uint8_t nextByte = 0; // buffer for bits to be yet written

			// Read background for first block if not aligned start
			if (bitsOffset) {
				setCursorAddress(width / 8 * y + x / 8);
				nextByte = (readSingleByte() & ~(0b11111111 << bitsOffset));
				bitsPending = bitsOffset;
			}

			const uint8_t rowOffsetByte = r * fontWidth / 8;
			const uint8_t rowOffsetBits = r * fontWidth % 8;

			// Process next blocks
			setCursorAddress(width / 8 * y + x / 8);
			writeStart();
			while (*pointer) {
				uint8_t remainingFontWidth = fontWidth;
				const uint8_t* charAddress = fontData + (*pointer - ' ') * fontRowBytes;
				uint8_t byteOffset;

				// TODO: use lambda to reduce program size a bit?

				// Partial first byte 
				if (rowOffsetBits) {
					const uint8_t data = pgm_read_byte(charAddress + rowOffsetByte) >> rowOffsetBits;
					const uint8_t length = 8 - rowOffsetBits;

					nextByte |= data << bitsPending;
					bitsPending += length;
					remainingFontWidth -= length;

					if (bitsPending >= 8) {
						writeNextByte(nextByte);
						bitsPending -= 8;
						nextByte = data >> (length - bitsPending);
					}

					byteOffset = 1;
				}
				else /* first byte is full too */ {
					byteOffset = 0;
				}

				// Full bytes
				while (remainingFontWidth > 8) {
					const uint8_t data = pgm_read_byte(charAddress + rowOffsetByte + byteOffset);

					nextByte |= data << bitsPending;
					writeNextByte(nextByte);
					remainingFontWidth -= 8;
					nextByte = data >> (8 - bitsPending);

					byteOffset += 1;
				}

				// Last bits
				{
					const uint8_t data = pgm_read_byte(charAddress + rowOffsetByte + byteOffset) 
						& ~(static_cast<uint8_t>(0b11111111) << remainingFontWidth);

					nextByte |= data << bitsPending;
					bitsPending += remainingFontWidth;

					if (bitsPending >= 8) {
						writeNextByte(nextByte);
						bitsPending -= 8;
						nextByte = data >> (remainingFontWidth - bitsPending);
					}
				}

				pointer += 1;
			}

			// Write last block, with background, if not aligned end
			if (bitsPending > 0) {
				nextByte |= readSingleByte() & (0b11111111 << bitsPending);
				const uint8_t widthTotal = static_cast<size_t>(pointer - string) * fontWidth + bitsOffset;
				setCursorAddress(width / 8 * y + x / 8 + widthTotal / 8);
				writeStart();
				writeNextByte(nextByte);
			}

			y += 1;
		}
	}

	/// Draw text vertically using selected font
	void drawTextVertical(const uint8_t x, const uint8_t y, const char* string, const void* font) {
		const auto fontWidth  = static_cast<const font_header_t*>(font)->width;
		const auto fontHeight = static_cast<const font_header_t*>(font)->height;
#ifdef FONT_ANY_8X16
		if (fontWidth == 8 && fontHeight == 16) {
			return drawTextVertical_8x16(x, y, string, font);
		}
#endif
		if (fontWidth <= 8) {
			return drawTextVertical_narrow(x, y, string, font);
		}
		else {
			return drawTextVertical_wide(x, y, string, font);
		}
	}
};



/// Display class using compilation-time defined pins for all IO, including data bus.
/// Note: It might use a bit slow mapping function, but should work for all 
/// boards and core. There is room for improvements, but currently Arduino team
/// don't want to use compile-time mappings inside main core in fear of 
/// incompatibility to other 3rd party cores. 
/// See https://github.com/arduino/ArduinoCore-avr/issues/264 (and related issues).
/// See `fastio_example.hpp` for example how to use faster IO.
template <
	// Enable: HIGH -> LOW enables
	uint8_t EN,
	// Chip select: LOW - selected. Can be set to `NOT_A_PIN` allow the code 
	// to assume chip is always selected by connecting display CS pin to ground.
	uint8_t CS,
	// Register select: HIGH - instruction, LOW - data
	uint8_t RS,
	// Read/write: HIGH - read, LOW - write
	uint8_t RW,
	// Data pins
	uint8_t DB0,
	uint8_t DB1,
	uint8_t DB2,
	uint8_t DB3,
	uint8_t DB4,
	uint8_t DB5,
	uint8_t DB6,
	uint8_t DB7,
	// Allow code to keep (and assume) chip is always selected. If false,
	// you can use the data pins between operations on display for other stuff,
	// possibly also other display (that use only other control ports).
	// Its true for default, as it allow to increase code efficiency.
	bool chipAlwaysSelected = true
>
class DisplayByPins : public DisplayBase
{
protected:
	inline void setDataBusAsInput()
	{
		pinMode(DB0, INPUT);
		pinMode(DB1, INPUT);
		pinMode(DB2, INPUT);
		pinMode(DB3, INPUT);
		pinMode(DB4, INPUT);
		pinMode(DB5, INPUT);
		pinMode(DB6, INPUT);
		pinMode(DB7, INPUT);
	}

	inline void setDataBusAsOutput()
	{
		pinMode(DB0, OUTPUT);
		pinMode(DB1, OUTPUT);
		pinMode(DB2, OUTPUT);
		pinMode(DB3, OUTPUT);
		pinMode(DB4, OUTPUT);
		pinMode(DB5, OUTPUT);
		pinMode(DB6, OUTPUT);
		pinMode(DB7, OUTPUT);
	}

	inline uint8_t readDataBus()
	{
		uint8_t out = 0;
		out |= (digitalRead(DB0) << 0);
		out |= (digitalRead(DB1) << 1);
		out |= (digitalRead(DB2) << 2);
		out |= (digitalRead(DB3) << 3);
		out |= (digitalRead(DB4) << 4);
		out |= (digitalRead(DB5) << 5);
		out |= (digitalRead(DB6) << 6);
		out |= (digitalRead(DB7) << 7);
		return out;
	}

	inline void writeDataBus(const uint8_t in)
	{
		digitalWrite(DB0, (in >> 0) & 1);
		digitalWrite(DB1, (in >> 1) & 1);
		digitalWrite(DB2, (in >> 2) & 1);
		digitalWrite(DB3, (in >> 3) & 1);
		digitalWrite(DB4, (in >> 4) & 1);
		digitalWrite(DB5, (in >> 5) & 1);
		digitalWrite(DB6, (in >> 6) & 1);
		digitalWrite(DB7, (in >> 7) & 1);
	}

	inline void selectChip()
	{
		if (!chipAlwaysSelected || CS == NOT_A_PIN) {
			// // Fake read without chip enable might be required
			// digitalWrite(CS, HIGH);
			// digitalWrite(RW, LOW);
			// digitalWrite(EN, HIGH);
			// digitalWrite(EN, LOW);

			digitalWrite(CS, LOW);
		}
	}

	inline void deselectChip()
	{
		if (!chipAlwaysSelected || CS == NOT_A_PIN) {
			digitalWrite(CS, HIGH);
		}
	}

	uint8_t read(const register_t reg) override
	{
		digitalWrite(EN, LOW);
		
		// Data bus is input only inside `read`, as writes are more common.
		setDataBusAsInput();

		selectChip();
		digitalWrite(RW, HIGH);
		digitalWrite(RS, reg);

		// Wait set-up time
		_delay_us(0.090);

		digitalWrite(EN, HIGH);

		// Wait data delay time (reading)
		_delay_us(0.140);

		uint8_t out = readDataBus();

		digitalWrite(EN, LOW);

		deselectChip();

		setDataBusAsOutput();

		return out;
	}

	void write(const register_t reg, const uint8_t value) override
	{
		digitalWrite(EN, LOW);

		writeDataBus(value);

		selectChip();
		digitalWrite(RW, LOW);
		digitalWrite(RS, reg);

		// Wait set-up time
		_delay_us(0.090);

		digitalWrite(EN, HIGH);

		// Wait data set-up time (writing)
		_delay_us(0.220);

		digitalWrite(EN, LOW);
		deselectChip();
	}

	void init() override
	{
		pinMode(EN, OUTPUT);
		digitalWrite(EN, LOW);

		pinMode(RS, OUTPUT);
		pinMode(RW, OUTPUT);

		if (CS != NOT_A_PIN) {
			pinMode(CS, OUTPUT);
			if (chipAlwaysSelected) {
				digitalWrite(CS, LOW);
			}
			else {
				digitalWrite(CS, HIGH);
			}
		}

		setDataBusAsOutput();
	}

	void waitBusy()
	{
		setDataBusAsInput();
		selectChip();
		digitalWrite(RW, HIGH);
		digitalWrite(RS, HIGH);

		while (true) {
			digitalWrite(EN, LOW);

			// Wait set-up time
			_delay_us(0.090);

			digitalWrite(EN, HIGH);

			// Wait data delay time (reading)
			_delay_us(0.140);

			if (!digitalRead(DB7)) {
				break;
			}
		}
	}
};



}
