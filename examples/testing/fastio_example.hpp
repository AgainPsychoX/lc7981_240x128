#include <lc7981.hpp>

/// Specialized display class using hardcoded IO for certain board with certain
/// core using certain pins, but avoiding slow functions like `pinMode`,
/// `digitalWrite`, `digitalRead` and so on, in order to get better efficiency.
/// 
/// This particular example work for:
/// * Microcontroller: Atmega32, external 20Mhz
/// * Arduino Core: MightyCore with standard pinout
/// * Display: EW24D40 240x128 with LC7981 controller
/// * Data pins: 
/// 	+ DB0 to DB6 are 10, 11, 12, 13, 14, 15 (6 MSB of port D)
/// 	+ DB7 and DB8 are 18, 19 (3rd and 4th bits of port C)
/// * Control pins:
/// 	+ RS (DI) / RW / EN / !CS are 20, 21, 22, 23 (PC4, PC5, PC6, PC7)
/// * Chip is not always selected.
/// 
/// Clear and draw whole screen 10 times benchmark:
/// 	+ Took around 420558us instead of around 4450821us for `DisplayByPins`.
/// 	+ Only 10th part of time! More around 4.3x faster!
class MyDisplay : public LC7981::DisplayBase
{
public:
	MyDisplay() : DisplayBase(240, 128) {}

protected:
	inline void setDataBusAsInput()
	{
		DDRD &= 0b00000011;
		DDRC &= 0b11110011;
		
		// Normal input (no pull-up)
		writeDataBus(0b00000000);
	}

	inline void setDataBusAsOutput()
	{
		DDRD |= 0b11111100;
		DDRC |= 0b00001100;
	}

	inline uint8_t readDataBus()
	{
		uint8_t out = (PORTD >> 2) | ((PORTC & 0b00001100) << 4);
		return out;
	}

	inline void writeDataBus(const uint8_t value)
	{
		PORTD = (value << 2) | (PORTD & 0b00000011);
		PORTC = ((value & 0b11000000) >> 4) | (PORTC & 0b11110011);
	}

	uint8_t read(const LC7981::register_t reg) override
	{
		uint8_t oldSREG = SREG;
		cli();

		// Data bus is input only inside `read`, as writes are more common.
		setDataBusAsInput();

		// Select chip, make sure EN was low, RW = HIGH (reading) and select register
		PORTC = (reg ? 0b00110000 : 0b00100000) | (PORTC & 0b00001111);

		// Wait set-up time
		_delay_us(0.090);

		// Set EN to high
		PORTC |= 0b01000000;

		// Wait data delay time (reading)
		_delay_us(0.140);

		uint8_t out = readDataBus();

		// Set EN to low
		PORTC &= 0b10111111;

		// Deselect chip
		PORTC |= 0b10000000;

		setDataBusAsOutput();

		SREG = oldSREG;

		return out;
	}

	void write(const LC7981::register_t reg, const uint8_t value) override
	{
		uint8_t oldSREG = SREG;
		cli();

		writeDataBus(value);

		// Select chip, make sure EN was low, RW = LOW (writing) and select register
		PORTC = (reg ? 0b00010000 : 0b00000000) | (PORTC & 0b00001111);

		// Wait set-up time
		_delay_us(0.090);

		// EN to high
		PORTC |= 0b01000000;

		// Wait data set-up time (writing)
		_delay_us(0.220);

		// Set EN to low
		PORTC &= 0b10111111;

		// Deselect chip
		PORTC |= 0b10000000;

		SREG = oldSREG;
	}

	void init() override
	{
		// Set control pins as outputs
		DDRC |= 0b11110000;

		// Disable timers on used pins
		{
			// TIMER1A
			TCCR1A &= ~_BV(COM1A1);

			// TIMER1B
			TCCR1A &= ~_BV(COM1B1);

			// TIMER2
			TCCR2 &= ~_BV(COM21);
		}

		// Set EN to low
		PORTC &= 0b10111111;

		// Deselect chip
		PORTC |= 0b10000000;

		setDataBusAsOutput();
	}

	void waitBusy()
	{
		setDataBusAsInput();

		while (true) {
			// Set CS to low (select chip), EN to low, RS, RW to high (read busy flag)
			PORTC = (0b00110000) | (PORTC & 0b00001111);

			// Wait set-up time
			_delay_us(0.090);

			// EN to high
			PORTC |= 0b01000000;

			// Wait data delay time (reading)
			_delay_us(0.140);

			// Wait while busy flag (DB7) is set
			if (!(PORTC & 0b00000100)) {
				break;
			}
		}
	}
};
