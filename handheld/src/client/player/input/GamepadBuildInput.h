#ifndef NET_MINECRAFT_CLIENT_PLAYER_INPUT_GamepadBuildInput_H__
#define NET_MINECRAFT_CLIENT_PLAYER_INPUT_GamepadBuildInput_H__

#include "IBuildInput.h"
#include "../../../platform/input/Controller.h"

class GamepadBuildInput : public IBuildInput {
public:
    GamepadBuildInput()
    :   buildDelayTicks(10),
        buildHoldTicks(0),
        lastAttackState(false),
        lastUseState(false)
    {}

    virtual bool tickBuild(Player* p, BuildActionIntention* bai) {
        bool attackPressed = Controller::isTouched(1);
        bool usePressed = Controller::isTouched(0);
        
        if (attackPressed)
        {
            *bai = BuildActionIntention(BuildActionIntention::BAI_REMOVE | BuildActionIntention::BAI_ATTACK);
            return true;
        }
        
        if (usePressed)
        {
            if (buildHoldTicks >= buildDelayTicks) buildHoldTicks = 0;
            if (++buildHoldTicks == 1)
            {
                *bai = BuildActionIntention(BuildActionIntention::BAI_BUILD | BuildActionIntention::BAI_INTERACT);
                return true;
            }
        }
        else
        {
            buildHoldTicks = 0;
        }
        
        lastAttackState = attackPressed;
        lastUseState = usePressed;
        
        return false;
    }

private:
    int buildHoldTicks;
    int buildDelayTicks;
    bool lastAttackState;
    bool lastUseState;
};

#endif /*NET_MINECRAFT_CLIENT_PLAYER_INPUT_GamepadBuildInput_H__*/
