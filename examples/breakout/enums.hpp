
// File for enums used as workaround for Arduino build process being fucked up.
// See https://forum.arduino.cc/t/enum-bizarre-compiler-behaviour/424324
// and https://arduino.stackexchange.com/questions/66530/class-enum-was-not-declared-in-this-scope

enum BrickType
{
	BRICK_NONE = 0,
	BRICK_HP_1,
	BRICK_HP_2,
	BRICK_HP_3,
	BRICK_INDESTRUCTIBLE = 0b1000000,
};
