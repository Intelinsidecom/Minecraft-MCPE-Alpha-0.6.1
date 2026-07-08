#ifndef SDL2CONTROLLER_H__
#define SDL2CONTROLLER_H__

#include "Controller.h"

extern bool g_controllerConnected;

class SDL2Controller
{
public:
    static bool init();
    static void shutdown();
    static void poll();
    static void tick();
    static bool isControllerConnected();

private:
    static bool _initialized;
    static void* _gameController;
    static int _controllerId;
    static int _tickCounter;
    
    static void handleButtonPress(int button, bool pressed);
    static void handleAxisMotion(int axis, short value);
    static float normalizeAxis(short value);
};

#endif /*SDL2CONTROLLER_H__*/
