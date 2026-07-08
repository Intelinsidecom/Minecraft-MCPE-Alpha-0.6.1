#ifndef NET_MINECRAFT_CLIENT_PLAYER_INPUT_SDL2TurnInput_H__
#define NET_MINECRAFT_CLIENT_PLAYER_INPUT_SDL2TurnInput_H__

#include "ITurnInput.h"

class SDL2TurnInput : public ITurnInput {
public:
	SDL2TurnInput()
	{
	}

	static void feedCamera(float x, float y) {
		_cameraX = x;
		_cameraY = y;
	}

	TurnDelta getTurnDelta() {
		const float MaxTurnSpeedX = 12.0f;
		const float MaxTurnSpeedY = 8.0f;
		const float DeadZone = 0.15f;

		float dx = 0, dy = 0;

		float stickX = (_cameraX > -DeadZone && _cameraX < DeadZone) ? 0.0f : _cameraX;
		float stickY = (_cameraY > -DeadZone && _cameraY < DeadZone) ? 0.0f : _cameraY;

		dx = stickX * MaxTurnSpeedX;
		dy = stickY * MaxTurnSpeedY;

		return TurnDelta(dx, -dy);
	}

private:
	static float _cameraX;
	static float _cameraY;
};

#endif /*NET_MINECRAFT_CLIENT_PLAYER_INPUT_SDL2TurnInput_H__*/
