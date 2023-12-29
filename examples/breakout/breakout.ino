/*
	This example demonstrates simple Breakout game using the library.
*/

#include <lc7981.hpp>
#include "examples/testing/font_06x08_Terminal_Microsoft.hpp"
#include "examples/testing/font_12x16_Terminal_Microsoft.hpp"

// Prepare display object using `DisplayByPins` (compile-time pin definition)
LC7981::DisplayByPins<
	// EN / CS / DI / RW
	22,  23,  20,  21,
	// DB0 to DB7
	10, 11, 12, 13, 14, 15, 18, 19
> display;

// Prepare display object using example fast I/O specialization (see README).
// #include "examples/testing/fastio_example.hpp"
// MyDisplay display;

#define BUTTON_LEFT  30
#define BUTTON_RIGHT 31

////////////////////////////////////////////////////////////////////////////////
// Paddle

/*     */ uint8_t paddleWidth;
constexpr uint8_t paddleHeight = 3;
/*     */ uint8_t paddleX;
constexpr uint8_t paddleY = 128 - paddleHeight;

void clearPaddle()
{
	display.drawWhiteRectangle(paddleX, paddleY, paddleWidth, paddleHeight);
}

void drawPaddle()
{
	display.drawBlackRectangle(paddleX, paddleY, paddleWidth, paddleHeight);
}

void updatePaddle(int8_t deltaX, int8_t deltaWidth)
{
	if (!deltaX && !deltaWidth) return;
	clearPaddle();
	if (static_cast<uint16_t>(paddleX) + paddleWidth + deltaX > display.width) {
		paddleX = display.width - paddleWidth;
	}
	else if (paddleX <= -deltaX) {
		paddleX = 0;
	}
	else {
		paddleX += deltaX;
	}
	drawPaddle();
	// TODO: smooth it by re-drawing only the changed part of the paddle
}

////////////////////////////////////////////////////////////////////////////////
// Ball

float ballX;
float ballY;
float ballVelocityX; // dots per ms
float ballVelocityY; // dots per ms

void drawHorizontalLineSafely(int16_t x, int16_t y, uint8_t length, uint8_t color)
{
	if (x < 0) {
		length += x;
		x = 0;
	}
	if (display.width < x + length) {
		length = display.width - x;
	}
	display.drawHorizontalLine(x, y < 0 ? : y, length, color);
}

void drawBall(uint8_t centerX, uint8_t centerY, uint8_t color)
{
	drawHorizontalLineSafely(centerX - 1, centerY - 2, 3, color); /* 0 1 1 1 0 */
	drawHorizontalLineSafely(centerX - 2, centerY - 1, 5, color); /* 1 1 1 1 1 */
	drawHorizontalLineSafely(centerX - 2, centerY,     5, color); /* 1 1 1 1 1 */
	drawHorizontalLineSafely(centerX - 2, centerY + 1, 5, color); /* 1 1 1 1 1 */
	drawHorizontalLineSafely(centerX - 1, centerY + 2, 3, color); /* 0 1 1 1 0 */
}

void updateBall(uint32_t deltaTime)
{
	uint8_t previousBallX = static_cast<uint8_t>(ballX);
	uint8_t previousBallY = static_cast<uint8_t>(ballY);
	ballX += ballVelocityX * deltaTime;
	ballY += ballVelocityY * deltaTime;
	bool needRedraw = static_cast<uint8_t>(ballX) != previousBallX 
		|| static_cast<uint8_t>(ballY) != previousBallY;
	if (needRedraw) {
		drawBall(previousBallX, previousBallY, 0x00);
	}
	if (ballX < 0) {
		ballX *= -1;
		ballVelocityX *= -1;
	}
	else if (ballX >= display.width - 1) {
		ballX = -ballX + static_cast<float>(display.width) * 2;
		// ballX = -ballX + display.width + display.width;
		// ballX = -ballX + display.width * 2; // not the same because uint8
		ballVelocityX *= -1;
	}
	if (ballY < 0) {
		ballY *= -1;
		ballVelocityY *= -1;
	}
	else if (ballY >= paddleY) {
		if (paddleX <= ballX && ballX <= paddleX + paddleWidth) {
			ballY = -ballY + static_cast<float>(paddleY) * 2;
			ballVelocityY *= -1;
			// TODO: change velocity based on point on paddle
		}
		else if (ballY >= display.height) {
			Serial.println("wtf");
			Serial.println(ballX);
			Serial.println(ballY);
			Serial.println(display.height);
			Serial.println(F("game over"));
			display.clearGray();
			display.drawTextVertical(16, 16, "Game over!", font_12x16_Terminal_Microsoft);
			delay(3000);
			reset();
			return;
		}
	}
	if (needRedraw) {
		drawBall(static_cast<uint8_t>(ballX), static_cast<uint8_t>(ballY), 0xFF);
	}
	// TODO: smooth it by re-drawing only the changed part of the ball
}

////////////////////////////////////////////////////////////////////////////////
// Setup & game logic loop

void setup()
{
	Serial.begin(115200);
	Serial.println(F("hello!"));

	pinMode(BUTTON_LEFT,  INPUT_PULLUP);
	pinMode(BUTTON_RIGHT, INPUT_PULLUP);

	display.initGraphicMode();

	reset();
}

uint32_t lastUpdate; // ms

void reset()
{
	Serial.println(F("resetting the game"));
	display.clearWhite();

	paddleWidth = 40;
	paddleX = (display.width - paddleWidth) / 2;
	drawPaddle();

	ballX = paddleX;
	ballY = paddleY - paddleWidth / 2;
	ballVelocityX = 0.04;
	ballVelocityY = 0.04;

	lastUpdate = millis();
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

	uint32_t now = millis();
	uint32_t deltaTime = now - lastUpdate; // ms
	lastUpdate = now;

	updateBall(deltaTime);
}
