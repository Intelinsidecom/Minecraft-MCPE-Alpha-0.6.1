#ifndef NET_MINECRAFT_CLIENT_PLAYER_INPUT_SDL2MoveInput_H__
#define NET_MINECRAFT_CLIENT_PLAYER_INPUT_SDL2MoveInput_H__

#include "IMoveInput.h"
#include "../../Options.h"
#include "../../../world/entity/player/Player.h"
#include <cmath>

class SDL2MoveInput : public IMoveInput {
public:
	SDL2MoveInput(Options* options)
	:   options(options),
	    wasSneakTouched(false)
	{
	}

	void setKey(int key, bool state) {
		int id = -1;
		if (key == options->keyUp.key) id = KEY_UP;
		if (key == options->keyDown.key) id = KEY_DOWN;
		if (key == options->keyLeft.key) id = KEY_LEFT;
		if (key == options->keyRight.key) id = KEY_RIGHT;
		if (key == options->keyJump.key) id = KEY_JUMP;
		if (key == options->keySneak.key) id = KEY_SNEAK;
		if (key == options->keyCraft.key) id = KEY_CRAFT;
		if (id >= 0) {
			keys[id] = state;
		}
	}

	void releaseAllKeys() {
		xa = 0;
		ya = 0;
		for (int i = 0; i < NumKeys; i++) {
			keys[i] = false;
		}
		wantUp = wantDown = false;
	}

	void tick(Player* player) {
		xa = 0;
		ya = 0;

		extern float g_leftStickX;
		extern float g_leftStickY;
		
		float deadzone = 0.15f;
		float rawX = g_leftStickX;
		float rawY = g_leftStickY;
		
		float magnitude = sqrt(rawX * rawX + rawY * rawY);
		float stickX = 0.0f;
		float stickY = 0.0f;
		
		if (magnitude > deadzone) {
			float scale = (magnitude - deadzone) / (1.0f - deadzone);
			if (scale > 1.0f) scale = 1.0f;
			
			stickX = (rawX / magnitude) * scale;
			stickY = (rawY / magnitude) * scale;
		}
		
		ya = stickY * 1.0f;
		xa = -stickX * 1.0f;
		
		if (keys[KEY_UP]) ya += 1.0f;
		if (keys[KEY_DOWN]) ya -= 1.0f;
		if (keys[KEY_LEFT]) xa += 1.0f;
		if (keys[KEY_RIGHT]) xa -= 1.0f;
		
		float len = sqrt(xa*xa + ya*ya);
		if (len > 1.0f) {
			xa /= len;
			ya /= len;
		}

		jumping = keys[KEY_JUMP];
		
		extern bool g_rightStickPressed;
		bool isSneakTouched = g_rightStickPressed;
		if (isSneakTouched && !wasSneakTouched) {
			sneaking = !sneaking;
		}
		wasSneakTouched = isSneakTouched;

		if (sneaking) {
			xa *= 0.3f;
			ya *= 0.3f;
		}
		
		wantUp = (jumping && keys[KEY_UP]);
		wantDown = (jumping && keys[KEY_DOWN]);
		if ((wantUp | wantDown) && (player && player->abilities.flying)) ya = 0;
	}

private:
	Options* options;
	bool wasSneakTouched;
	
	static const int NumKeys = 7;
	bool keys[NumKeys] = {false};
	
	enum KeyIndices {
		KEY_UP = 0,
		KEY_DOWN = 1,
		KEY_LEFT = 2,
		KEY_RIGHT = 3,
		KEY_JUMP = 4,
		KEY_SNEAK = 5,
		KEY_CRAFT = 6
	};
};

extern float g_leftStickX;
extern float g_leftStickY;
extern float g_rightStickX;
extern float g_rightStickY;
extern bool g_rightStickPressed;

#endif /*NET_MINECRAFT_CLIENT_PLAYER_INPUT_SDL2MoveInput_H__*/
