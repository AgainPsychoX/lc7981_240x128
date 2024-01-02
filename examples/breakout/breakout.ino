/*
	This example demonstrates simple Breakout game using the library.
*/

#include <lc7981.hpp>
#include "examples/testing/font_06x08_Terminal_Microsoft.hpp"
#include "examples/testing/font_12x16_Terminal_Microsoft.hpp"
#include "enums.hpp"

// Prepare display object using `DisplayByPins` (compile-time pin definition)
LC7981::DisplayByPins<
	// EN / CS / DI / RW
	22,  23,  20,  21,
	// DB0 to DB7
	10, 11, 12, 13, 14, 15, 18, 19
> display;

constexpr uint8_t displayWidth = 240;
constexpr uint8_t displayHeight = 128;

// Prepare display object using example fast I/O specialization (see README).
// #include "examples/testing/fastio_example.hpp"
// MyDisplay display;

#define BUTTON_LEFT  30
#define BUTTON_RIGHT 31

void(* crash) (void) = 0;

////////////////////////////////////////////////////////////////////////////////
// Paddle

/*     */ uint8_t paddleWidth;
constexpr uint8_t paddleHeight = 3;
/*     */ uint8_t paddleX;
constexpr uint8_t paddleY = displayHeight - paddleHeight;

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
	if (static_cast<uint16_t>(paddleX) + paddleWidth + deltaX > displayWidth) {
		paddleX = displayWidth - paddleWidth;
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
// Bricks
//  We assume all bricks are the same size and are statically positioned with gaps.
//  All bricks can be in serval type: 4 hit point stages and 1 indestructible.

constexpr uint8_t brickWidth = 15;
constexpr uint8_t brickHeight = 6;
constexpr uint8_t bricksCountX = 14;
constexpr uint8_t bricksCountY = 7;
constexpr uint8_t bricksGridStartX = 2;
constexpr uint8_t bricksGridStartY = 1;
constexpr uint8_t bricksGridGapX = 2;
constexpr uint8_t bricksGridGapY = 2;
constexpr uint8_t lastPossibleBrickX = bricksGridStartX + (brickWidth + bricksGridGapX) * bricksCountX - bricksGridGapX - 1;
constexpr uint8_t lastPossibleBrickY = bricksGridStartY + (brickHeight + bricksGridGapY) * bricksCountY - bricksGridGapY - 1;

BrickType bricksData[bricksCountX * bricksCountY];

struct BrickHandle
{
	static BrickHandle const invalid;

	uint8_t id;

	inline bool isValid() { return id < bricksCountX * bricksCountY; }

	inline BrickType getType() { return bricksData[id]; }
	inline void setType(BrickType type) { bricksData[id] = type; }

	inline uint8_t getGridX() { return id % bricksCountX; }
	inline uint8_t getGridY() { return id / bricksCountX; }

	inline static BrickHandle onGrid(uint8_t x, uint8_t y) {
		return BrickHandle { static_cast<uint8_t>(bricksCountX * y + x) };
	}

	/// Updates brick on hit. 
	/// Returns true if brick was solid (ball should bounce off), false otherwise.
	inline bool hit() {
	switch (getType()) {
			case BRICK_NONE: return false;
			case BRICK_HP_1: setType(BRICK_NONE); return true;
			case BRICK_HP_2: setType(BRICK_HP_1); return true;
			case BRICK_HP_3: setType(BRICK_HP_2); return true; 
			case BRICK_INDESTRUCTIBLE: return true;
		}
	}
};

constexpr BrickHandle const BrickHandle::invalid { static_cast<uint8_t>(-1) };

void drawBrick(BrickType type, uint8_t x, uint8_t y)
{
	switch (type) {
		case BRICK_NONE:
			display.drawWhiteFill(x, y, brickWidth, brickHeight);
			break;
		case BRICK_HP_1:
			// TODO: draw pattern from bitmap?
			display.drawGrayFill(x, y, brickWidth, brickHeight);
			break;
		case BRICK_HP_2:
			// TODO: draw pattern from bitmap?
			display.drawGrayFill(x, y, brickWidth, brickHeight);
			break;
		case BRICK_HP_3:
			display.drawBlackFill(x, y, brickWidth, brickHeight);
			break;
		case BRICK_INDESTRUCTIBLE:
			display.drawWhiteRectangle(x, y, brickWidth, brickHeight);
			// drawBlackFill(x + 1, y + 1, brickWidth - 2, brickHeight - 2);
			break;
	}
}

void drawBrick(struct BrickHandle handle)
{
	if (!handle.isValid()) return;
	const uint8_t x = bricksGridGapX + handle.getGridX() * (brickWidth + bricksGridGapX);
	const uint8_t y = bricksGridGapY + handle.getGridY() * (brickHeight + bricksGridGapY);
	drawBrick(handle.getType(), x, y);
}

void resetAllBricks()
{
	for (uint8_t gy = 0; gy < bricksCountY; gy++) {
		for (uint8_t gx = 0; gx < bricksCountX; gx++) {
			auto handle = BrickHandle::onGrid(gx, gy);
			handle.setType(BRICK_HP_3);
			drawBrick(handle);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Ball

constexpr uint8_t ballCollisionBoxRadius = 2;

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
	if (displayWidth < x + length) {
		length = displayWidth - x;
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

void updateBallAgainstBricks(uint8_t ballCenterX, uint8_t ballCenterY)
{
	/* Corners:
		A B - pos(A)=(x1, y1), pos(B)=(x2, y1),
		C D - pos(C)=(x1, y2), pos(B)=(x2, y2). 
	*/
	uint8_t gx1, gx2, gy1, gy2; // positions on the bricks grid
	uint8_t bx1, bx2, by1, by2; // positions within the brick/gap

	constexpr uint8_t leftOffset = ballCollisionBoxRadius + bricksGridStartX;
	bool hasX1 = ballCenterX >= leftOffset;
	if (hasX1) {
		uint8_t sx1 = ballCenterX - leftOffset;
		gx1 = sx1 / (brickWidth + bricksGridGapX);
		bx1 = sx1 % (brickWidth + bricksGridGapX);
		hasX1 = bx1 < brickWidth;
	}

	constexpr uint8_t topOffset = ballCollisionBoxRadius + bricksGridStartY;
	bool hasY1 = ballCenterY >= topOffset && ballCenterY < (lastPossibleBrickY - ballCollisionBoxRadius);
	if (hasY1) {
		uint8_t sy1 = ballCenterY - topOffset;
		gy1 = sy1 / (brickHeight + bricksGridGapY);
		by1 = sy1 % (brickHeight + bricksGridGapY);
		hasY1 = by1 < brickHeight;
	}

	bool hasX2 = ballCenterX < (lastPossibleBrickX - ballCollisionBoxRadius);
	if (hasX2) {
		uint8_t sx2 = ballCenterX - bricksGridStartX + ballCollisionBoxRadius;
		gx2 = sx2 / (brickWidth + bricksGridGapX);
		bx2 = sx2 % (brickWidth + bricksGridGapX);
		hasX2 = bx2 < brickWidth;
	}

	bool hasY2 = ballCenterY < (lastPossibleBrickY - ballCollisionBoxRadius);
	if (hasY2) {
		uint8_t sy2 = ballCenterY - bricksGridStartY + ballCollisionBoxRadius;
		gy2 = sy2 / (brickHeight + bricksGridGapY);
		by2 = sy2 % (brickHeight + bricksGridGapY);
		hasY2 = by2 < brickHeight;
	}

	BrickHandle a = hasX1 && hasY1 ? BrickHandle::onGrid(gx1, gy1) : BrickHandle::invalid;
	BrickHandle b = hasX2 && hasY1 ? BrickHandle::onGrid(gx2, gy1) : BrickHandle::invalid;
	BrickHandle c = hasX1 && hasY2 ? BrickHandle::onGrid(gx1, gy2) : BrickHandle::invalid;
	BrickHandle d = hasX2 && hasY2 ? BrickHandle::onGrid(gx2, gy2) : BrickHandle::invalid;
	
	bool aHit = a.isValid() && a.hit();
	bool bHit = b.isValid() && b.hit();
	bool cHit = c.isValid() && c.hit();
	bool dHit = d.isValid() && d.hit();
	// FIXME: decouple: no double damage if single brick hit with both corners

	if (aHit || bHit || cHit || dHit) {
		Serial.print("bCx: "); Serial.println(ballCenterX);
		Serial.print("bCy: "); Serial.println(ballCenterY);
		Serial.print("bVx: "); Serial.println(ballVelocityX);
		Serial.print("bVy: "); Serial.println(ballVelocityY);
		Serial.print("gx1: "); Serial.println(gx1);
		Serial.print("gy1: "); Serial.println(gy1);
		Serial.print("gx2: "); Serial.println(gx2);
		Serial.print("gy2: "); Serial.println(gy2);
	}
	if (aHit) { Serial.print("A: "); Serial.print(a.getGridX()); Serial.print(' '); Serial.println(a.getGridY()); }
	if (bHit) { Serial.print("B: "); Serial.print(b.getGridX()); Serial.print(' '); Serial.println(b.getGridY()); } 
	if (cHit) { Serial.print("C: "); Serial.print(c.getGridX()); Serial.print(' '); Serial.println(c.getGridY()); }
	if (dHit) { Serial.print("D: "); Serial.print(d.getGridX()); Serial.print(' '); Serial.println(d.getGridY()); }

	if (aHit) drawBrick(a);
	if (bHit) drawBrick(b);
	if (cHit) drawBrick(c);
	if (dHit) drawBrick(d);

	// if (aHit || bHit || cHit || dHit) {
	//   delay(1000);
	//   crash();
	// }

	// TODO: special logic for (rare) triple-hit?

	/**/ if (aHit && bHit) ballVelocityY *= -1;
	else if (aHit && cHit) ballVelocityX *= -1;
	else if (bHit && dHit) ballVelocityX *= -1;
	else if (cHit && dHit) ballVelocityY *= -1;
	else {
		if (aHit) {
			/**/ if (0 < ballVelocityX) ballVelocityY *= -1;
			else if (0 < ballVelocityY) ballVelocityX *= -1;
			else goto corner;
		}
		else if (bHit) {
			/**/ if (ballVelocityX < 0) ballVelocityY *= -1;
			else if (0 < ballVelocityY) ballVelocityX *= -1;
			else goto corner;
		}
		else if (cHit) {
			/**/ if (0 < ballVelocityX) ballVelocityY *= -1;
			else if (ballVelocityY < 0) ballVelocityX *= -1;
			else goto corner;
		}
		else if (dHit) {
			/**/ if (0 < ballVelocityX) ballVelocityY *= -1;
			else if (0 < ballVelocityY) ballVelocityX *= -1;
			else goto corner;
		}
		return;

		corner: // negate and swap velocity in both directions
		float tmp = ballVelocityX;
		ballVelocityX = -ballVelocityY;
		ballVelocityY = -tmp;
	}

	// TODO: fix corners bounce based on position in the brick
	// TODO: use union/binary operators (instead single boolean logic)?
}

void updateBall(uint32_t deltaTime)
{
	uint8_t previousBallScreenX = static_cast<uint8_t>(ballX);
	uint8_t previousBallScreenY = static_cast<uint8_t>(ballY);
	ballX += ballVelocityX * deltaTime;
	ballY += ballVelocityY * deltaTime;
	bool needRedraw = static_cast<uint8_t>(ballX) != previousBallScreenX 
		|| static_cast<uint8_t>(ballY) != previousBallScreenY;
	if (needRedraw) {
		drawBall(previousBallScreenX, previousBallScreenY, 0x00);
	}
	if (ballX < ballCollisionBoxRadius) {
		ballX *= -1;
		ballVelocityX *= -1;
	}
	else if (ballX >= displayWidth - ballCollisionBoxRadius - 1) {
		ballX = -ballX + static_cast<float>(displayWidth) * 2;
		ballVelocityX *= -1;
	}
	if (ballY < ballCollisionBoxRadius) {
		ballY *= -1;
		ballVelocityY *= -1;
	}
	else if (ballY >= paddleY - ballCollisionBoxRadius) {
		if (paddleX <= ballX && ballX <= paddleX + paddleWidth) {
			ballY = -ballY + static_cast<float>(paddleY) * 2;
			ballVelocityY *= -1;
			// TODO: change velocity based on point on paddle
		}
		else if (ballY >= displayHeight) {
			Serial.println(F("game over"));
			display.clearGray();
			display.drawTextVertical(16, 16, "Game over!", font_12x16_Terminal_Microsoft);
			delay(3000);
			reset();
			return;
		}
	}
	uint8_t nextBallScreenX = static_cast<uint8_t>(ballX);
	uint8_t nextBallScreenY = static_cast<uint8_t>(ballY);
	updateBallAgainstBricks(nextBallScreenX, nextBallScreenY);
	if (needRedraw) {
		drawBall(nextBallScreenX, nextBallScreenY, 0xFF);
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
	paddleX = (displayWidth - paddleWidth) / 2;
	drawPaddle();

	resetAllBricks();

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
