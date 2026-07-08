#include "Screen.h"
#include "components/Button.h"
#include "components/TextBox.h"
#include "../Minecraft.h"
#include "../renderer/Tesselator.h"
#include "../sound/SoundEngine.h"
#include "../../platform/input/Keyboard.h"
#include "../../platform/input/Mouse.h"
#include "../renderer/Textures.h"
#include "../../platform/input/SDL2Controller.h"
#include "../../platform/log.h"

Screen::Screen()
:   passEvents(false),
	clickedButton(NULL),
	tabButtonIndex(0),
	width(1),
	height(1),
	minecraft(NULL),
	font(NULL)
{
}

void Screen::render( int xm, int ym, float a )
{
	for (unsigned int i = 0; i < buttons.size(); i++) {
		Button* button = buttons[i];
		button->render(minecraft, xm, ym);
	}
}

void Screen::init( Minecraft* minecraft, int width, int height )
{
	//particles = /*new*/ GuiParticles(minecraft);
	this->minecraft = minecraft;
	this->font = minecraft->font;
	this->width = width;
	this->height = height;
	init();
	setupPositions();
	updateTabButtonSelection();
	
	extern bool g_controllerConnected;
	if (g_controllerConnected)
	{
		initControllerNavigation();
	}
}

void Screen::init()
{
}

void Screen::setSize( int width, int height )
{
	this->width = width;
	this->height = height;
	setupPositions();
}

bool Screen::handleBackEvent( bool isDown )
{
	return false;
}

void Screen::updateEvents()
{
	if (passEvents)
		return;

	while (Mouse::next())
		mouseEvent();

	while (Keyboard::next())
		keyboardEvent();
	while (Keyboard::nextTextChar())
		keyboardTextEvent();
}

void Screen::mouseEvent()
{
	const MouseAction& e = Mouse::getEvent();
	if (!e.isButton())
		return;

	if (Mouse::getEventButtonState()) {
		int xm = e.x * width / minecraft->width;
		int ym = e.y * height / minecraft->height - 1;
		mouseClicked(xm, ym, Mouse::getEventButton());
	} else {
		int xm = e.x * width / minecraft->width;
		int ym = e.y * height / minecraft->height - 1;
		mouseReleased(xm, ym, Mouse::getEventButton());
	}
}

void Screen::keyboardEvent()
{
	if (Keyboard::getEventKeyState()) {
		//if (Keyboard.getEventKey() == Keyboard.KEY_F11) {
		//    minecraft->toggleFullScreen();
		//    return;
		//}
		keyPressed(Keyboard::getEventKey());
	}
}
void Screen::keyboardTextEvent()
{
	keyboardNewChar(Keyboard::getChar());
}
void Screen::renderBackground()
{
	renderBackground(0);
}

void Screen::renderBackground( int vo )
{
	if (minecraft->isLevelGenerated()) {
		fillGradient(0, 0, width, height, 0xc0101010, 0xd0101010);
	} else {
		renderDirtBackground(vo);
	}
}

void Screen::renderDirtBackground( int vo )
{
	//glDisable2(GL_LIGHTING);
	glDisable2(GL_FOG);
	Tesselator& t = Tesselator::instance;
	minecraft->textures->loadAndBindTexture("gui/background.png");
	glColor4f2(1, 1, 1, 1);
	float s = 32;
	float fvo = (float) vo;
	t.begin();
	t.color(0x404040);
	t.vertexUV(0, (float)height, 0, 0, height / s + fvo);
	t.vertexUV((float)width, (float)height, 0, width / s, (float)height / s + fvo);
	t.vertexUV((float)width, 0, 0, (float)width / s, 0 + fvo);
	t.vertexUV(0, 0, 0, 0, 0 + fvo);
	t.draw();
}

bool Screen::isPauseScreen()
{
	return true;
}

bool Screen::isErrorScreen()
{
	return false;
}

bool Screen::isInGameScreen()
{
	return true;
}

bool Screen::closeOnPlayerHurt() {
    return false;
}

void Screen::keyPressed( int eventKey )
{
	if (eventKey == Keyboard::KEY_ESCAPE) {
		minecraft->setScreen(NULL);
		//minecraft->grabMouse();
	}
	if (minecraft->useTouchscreen())
		return;

	// "Tabbing" the buttons (walking with keys)
	const int tabButtonCount = tabButtons.size();
	if (!tabButtonCount)
		return;

	Options& o = minecraft->options;
	if (eventKey == o.keyMenuNext.key)
		if (++tabButtonIndex == tabButtonCount) tabButtonIndex = 0;
	if (eventKey == o.keyMenuPrevious.key)
		if (--tabButtonIndex == -1) tabButtonIndex = tabButtonCount-1;
	if (eventKey == o.keyMenuOk.key) {
		Button* button = tabButtons[tabButtonIndex];
		if (button->active) {
			minecraft->soundEngine->playUI("random.click", 1, 1);
			buttonClicked(button);
		}
	}

	updateTabButtonSelection();
}

void Screen::updateTabButtonSelection()
{
	if (minecraft->useTouchscreen())
		return;

	for (unsigned int i = 0; i < tabButtons.size(); ++i)
		tabButtons[i]->selected = (i == tabButtonIndex);
}

void Screen::mouseClicked( int x, int y, int buttonNum )
{
	if (buttonNum == MouseAction::ACTION_LEFT) {
		for (unsigned int i = 0; i < buttons.size(); ++i) {
			Button* button = buttons[i];
            //LOGI("Hit-testing button: %p\n", button);
			if (button->clicked(minecraft, x, y)) {
                button->setPressed();

                //LOGI("Hit-test successful: %p\n", button);
				clickedButton = button;
/*
#if !defined(ANDROID) && !defined(__APPLE__) //if (!minecraft->isTouchscreen()) {
					minecraft->soundEngine->playUI("random.click", 1, 1);
					buttonClicked(button);
#endif }
*/
			}
		}
	}
}

void Screen::mouseReleased( int x, int y, int buttonNum )
{
	//LOGI("b_id: %d, (%p), text: %s\n", buttonNum, clickedButton, clickedButton?clickedButton->msg.c_str():"<null>");
	if (!clickedButton || buttonNum != MouseAction::ACTION_LEFT) return;

#if 1
//#if defined(ANDROID) || defined(__APPLE__) //if (minecraft->isTouchscreen()) {
		for (unsigned int i = 0; i < buttons.size(); ++i) {
			Button* button = buttons[i];
			if (clickedButton == button && button->clicked(minecraft, x, y)) {
				buttonClicked(button);
				minecraft->soundEngine->playUI("random.click", 1, 1);
				clickedButton->released(x, y);
			}
		}
# else //	} else {
		clickedButton->released(x, y);
#endif // }
	clickedButton = NULL;
}

bool Screen::renderGameBehind() {
	return true;
}

bool Screen::hasClippingArea( IntRectangle& out )
{
	return false;
}

void Screen::lostFocus() {
	for(std::vector<TextBox*>::iterator it = textBoxes.begin(); it != textBoxes.end(); ++it) {
		TextBox* tb = *it;
		tb->loseFocus(minecraft);
	}
}

void Screen::toGUICoordinate( int& x, int& y ) {
	x = x * width / minecraft->width;
	y = y * height / minecraft->height - 1;
}

void Screen::tick()
{
    extern bool g_controllerConnected;
    if (g_controllerConnected && minecraft && minecraft->screen == this)
    {
        handleControllerInput();
    }
}

void Screen::initControllerNavigation()
{
    g_controllerNavigation.init(width, height);
    g_controllerNavigation.setGUIMode(true);
    

    for (unsigned int i = 0; i < buttons.size(); i++)
    {
        Button* button = buttons[i];
        if (button->active && button->visible)
        {
            g_controllerNavigation.registerButton(button);
        }
    }
    
    g_controllerNavigation.updateButtonSelection();
}

void Screen::handleControllerInput()
{
    extern float g_leftStickX;
    extern float g_leftStickY;
    extern float g_rightStickY;
    extern bool g_controllerAPressed;
    extern bool g_controllerBPressed;
    static bool wasAPressed = false;
    static bool wasBPressed = false;
    static bool bCycleUsed = false;
    
    if (g_controllerBPressed && !wasBPressed && !buttons.empty())
    {
        int focusedIndex = -1;
        for (unsigned int i = 0; i < buttons.size(); i++)
        {
            if (buttons[i]->selected && buttons[i]->active && buttons[i]->visible)
            {
                focusedIndex = i;
                break;
            }
        }
        
        if (focusedIndex >= 0 && focusedIndex < (int)buttons.size() - 1)
        {
            buttons[focusedIndex]->selected = false;
            buttons[focusedIndex + 1]->selected = true;
            bCycleUsed = true;
        }
        else if (focusedIndex == (int)buttons.size() - 1)
        {
            bCycleUsed = false;
        }
        else
        {
            if (!buttons.empty() && buttons[0]->active && buttons[0]->visible)
                buttons[0]->selected = true;
            bCycleUsed = true;
        }
    }
    else if (!g_controllerBPressed)
    {
        bCycleUsed = false;
    }

    if (bCycleUsed)
    {
        g_controllerNavigation.tick(minecraft, g_leftStickX, g_leftStickY, g_rightStickY, 
                                     g_controllerAPressed, false, wasAPressed, wasBPressed);
    }
    else
    {
        g_controllerNavigation.tick(minecraft, g_leftStickX, g_leftStickY, g_rightStickY, 
                                     g_controllerAPressed, g_controllerBPressed, wasAPressed, wasBPressed);
    }
    
    wasAPressed = g_controllerAPressed;
    wasBPressed = g_controllerBPressed;
}
