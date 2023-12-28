/*
	This example demonstrates simple Breakout game using the library.
*/

#include <lc7981.hpp>

// Prepare display object using `DisplayByPins` (compile-time pin definition)
LC7981::DisplayByPins<
	// EN / CS / DI / RW
	22,  23,  20,  21,
	// DB0 to DB7
	10, 11, 12, 13, 14, 15, 18, 19
> display;

// Prepare display object using example fast I/O specialization (see README).
// #include "../testing/fastio_example.hpp"
// MyDisplay display;

#define BUTTON_LEFT  30
#define BUTTON_RIGHT 31

uint8_t paddleWidth;
constexpr uint8_t paddleHeight = 3;
uint8_t paddleX;
constexpr uint8_t paddleY = 128 - paddleHeight;

void setup()
{
	Serial.begin(115200);
	Serial.println(F("hello!"));

	display.initGraphicMode();
	display.clearWhite();

	pinMode(BUTTON_LEFT,  INPUT_PULLUP);
	pinMode(BUTTON_RIGHT, INPUT_PULLUP);

	reset();
}

void reset() {
	Serial.println(F("resetting the game"));
	display.drawWhiteRectangle(paddleX, paddleY, paddleWidth, paddleHeight);
	paddleWidth = 40;
	paddleX = (display.width - paddleWidth) / 2;
	display.drawBlackRectangle(paddleX, paddleY, paddleWidth, paddleHeight);
}

void updatePaddle(int8_t deltaX, int8_t deltaWidth) {
	if (!deltaX && !deltaWidth) return;
	display.drawWhiteRectangle(paddleX, paddleY, paddleWidth, paddleHeight);
	if (static_cast<uint16_t>(paddleX) + paddleWidth + deltaX > display.width) {
		paddleX = display.width - paddleWidth;
	}
	else if (paddleX <= -deltaX) {
		paddleX = 0;
	}
	else {
		paddleX += deltaX;
	}
	display.drawBlackRectangle(paddleX, paddleY, paddleWidth, paddleHeight);
	// TODO: smooth it by re-drawing only the changed part of the paddle
}

void loop()
{
	int8_t paddleXChange = 0;
	if (digitalRead(BUTTON_LEFT) == HIGH) {
		if (digitalRead(BUTTON_RIGHT) == HIGH) {
			// TODO: Short press: pause/unpause; Long press: reset
		}
		else {
			paddleXChange = -1;
		}
	}
	else if (digitalRead(BUTTON_RIGHT) == HIGH) {
		paddleXChange = 1;
	}
	updatePaddle(paddleXChange, 0);
}
