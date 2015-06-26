#ifndef UIELEMENT_H
#define UIELEMENT_H

#include <vector>
#include <list>
#include <string>
#include "Const.h"
#include "coord.h"
#include "SDLheaders.h"

class C_UIElement
{
public:
	C_UIElement(std::string path, int midXPos_, int topYPos_);
	virtual void draw();
	void hide() { visible = false; }
	void show() { visible = true; }
	virtual ~C_UIElement() { }
protected:
	std::string imagePath; // e.g. "menu_play.png"
	bool visible;
	int midXPos;
	int topYPos;
};

#ifdef __APPLE__
typedef void(*funcPtr)();
typedef bool(*boolPtr)();
#else
typedef std::function<void()> funcPtr;
typedef std::function<bool()> boolPtr;
#endif
class C_Button : public C_UIElement
{
public:
	C_Button(std::string path, SDL_Keycode key_, funcPtr func_, int midXPos_, int topYPos_);
	virtual void draw();
	void activate();
	virtual bool parseInput(SDL_Event &event, C_Coord &mousePos); // check if click was in rect or was of key type
	std::string getPath();
	funcPtr activatePtr;
	virtual ~C_Button() { }
protected:
	SDL_Keycode key;
};

class C_Screen : public C_UIElement
{
public:
	C_Screen(std::string path, const std::list<C_Button> buttonList, const std::list<C_UIElement> uiElementList, int state_, int delay_ = 1) : C_UIElement(path, SWIDTH/2, 0), state(state_), toUpdate(false), inputReceived(true), delay(delay_), buttons(buttonList.begin(), buttonList.end()), uiElements(uiElementList.begin(), uiElementList.end()) { }
	virtual void draw();
	virtual void setup() { }
	virtual void cleanup() { }
	virtual void drawToScreen();
	virtual void update() { }
	virtual bool shouldRedraw();
	virtual std::string drawToTexture();
	void fadeIn();
	virtual bool parseInput(SDL_Event &event); // check all buttons this screen contains & keycodes for this screen
	void frame();
	C_Coord mousePos;
	virtual void animateIn(C_Screen* previousScreen);
	int getState() { return state; }
	virtual ~C_Screen() { }
protected:
	int state;
	int delay;
	bool toUpdate;
	bool inputReceived;
	std::vector<C_Button> buttons;
	std::vector<C_UIElement> uiElements;
	std::string getTexPath() { return "tex_" + imagePath; }
};

class C_GameScreen : public C_Screen
{
public:
	C_GameScreen() : C_Screen("", std::list<C_Button>(), std::list<C_UIElement>(), STATE_PLAY, 50) {}
	void setup();
	void draw();
	bool parseInput(SDL_Event &event);
	void animateIn(C_Screen* previousScreen);
	void cleanup();
	void drawToScreen();
	bool shouldRedraw();
	void update();
private:
	int lastInputTime;
	bool blinked;
};

class C_EndScreen : public C_Screen
{
public:
	C_EndScreen(std::string path, const std::list<C_Button> buttonList, const std::list<C_UIElement> uiElementList, int state_) : C_Screen(path, buttonList, uiElementList, state_) { }
	void setup();
};

class C_UI
{
public:
	C_UI();
	void start();
	void clearEventQueue();
private:
	std::vector<C_Screen*> screens; // the position in the array corresponds to the STATE_ enum in Const.h	
	C_Screen mainMenuScreen;
	C_GameScreen gameScreen;
	C_EndScreen endScreen;
	C_Screen* currentScreen;
	C_Screen* previousScreen;
};

#endif
