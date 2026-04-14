#include "MouseHandler.h"
#include "player/input/ITurnInput.h"
#include "../App.h"
#include "../AppPlatform.h"

#ifdef RPI
#include <SDL/SDL.h>
#elif defined(WIN32)
extern App* g_app;
#endif

MouseHandler::MouseHandler( ITurnInput* turnInput )
:	_turnInput(turnInput)
{}

MouseHandler::MouseHandler()
:	_turnInput(0)
{}

MouseHandler::~MouseHandler() {
}

void MouseHandler::setTurnInput( ITurnInput* turnInput ) {
	_turnInput = turnInput;
}

void MouseHandler::grab() {
	xd = 0;
	yd = 0;

#if defined(RPI)
	//LOGI("Grabbing input!\n");
	SDL_WM_GrabInput(SDL_GRAB_ON);
	SDL_ShowCursor(0);
#elif defined(WIN32)
	if (g_app && g_app->platform()) g_app->platform()->grabMouse();
#endif
}

void MouseHandler::release() {
#if defined(RPI)
	//LOGI("Releasing input!\n");
	SDL_WM_GrabInput(SDL_GRAB_OFF);
	SDL_ShowCursor(1);
#elif defined(WIN32)
	if (g_app && g_app->platform()) g_app->platform()->releaseMouse();
#endif
}

void MouseHandler::poll() {
	if (_turnInput != 0) {
		TurnDelta td = _turnInput->getTurnDelta();
		xd = td.x;
		yd = td.y;
	}
}
