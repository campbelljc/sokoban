#include "ui.h"
#include "g_Game.h"

#include <boost/assign/list_of.hpp>
using boost::assign::list_of;

// src : http://stackoverflow.com/questions/2049582/how-to-determine-a-point-in-a-triangle
float sign(int p1X, int p1Y, int p2X, int p2Y, int p3X, int p3Y)
{
    return (p1X - p3X) * (p2Y - p3Y) - (p2X - p3X) * (p1Y - p3Y);
}

bool pointInTriangle(int ptX, int ptY, int v1X, int v1Y, int v2X, int v2Y, int v3X, int v3Y)
{
    bool b1, b2, b3;

    b1 = sign(ptX, ptY, v1X, v1Y, v2X, v2Y) < 0.0f;
    b2 = sign(ptX, ptY, v2X, v2Y, v3X, v3Y) < 0.0f;
    b3 = sign(ptX, ptY, v3X, v3Y, v1X, v1Y) < 0.0f;

    return ((b1 == b2) && (b2 == b3));
}

/// ///////////////////////////////////////////////
/// C_UIElement
/// ///////////////////////////////////////////////

C_UIElement::C_UIElement(std::string path, int midXPos_, int topYPos_) : imagePath(path), midXPos(midXPos_), visible(true), topYPos(topYPos_) { }

void C_UIElement::draw()
{
	if (!visible || imagePath == "") return;
	std::string path = "screens/" + imagePath;
	C_Graphics::drawUI(path, midXPos, topYPos); // adds images/imagesX/ to path
}

/// ///////////////////////////////////////////////
/// C_Button
/// ///////////////////////////////////////////////

C_Button::C_Button(std::string path, SDL_Keycode key_, funcPtr func_, int midXPos_, int topYPos_) : C_UIElement(path, midXPos_, topYPos_), activatePtr(func_), key(key_) { }

std::string C_Button::getPath()
{
	std::string path = "screens/" + imagePath;
	return path;
}

void C_Button::draw()
{
	if (imagePath == "") return;
	C_Graphics::drawUI(this->getPath(), midXPos, topYPos); // adds images/imagesX/ to path
}

bool C_Button::parseInput(SDL_Event &event, C_Coord &mousePos)
{
	switch(event.type)
	{
		case SDL_MOUSEBUTTONDOWN:
		{
			return g_Game.inRect(mousePos, this->getPath(), midXPos, topYPos);
		}
		case SDL_KEYDOWN:
		{
			return (event.key.keysym.sym == this->key);
		}
	}
	return false;
}

void C_Button::activate()
{
	this->activatePtr();
}

/// ///////////////////////////////////////////////
/// C_Screen
/// ///////////////////////////////////////////////

void C_Screen::draw()
{
	C_Graphics::clearScreen();
	this->C_UIElement::draw();
	for (int count = 0; count < buttons.size(); count ++)
	{
		buttons.at(count).draw();
	}
	for (int count = 0; count < uiElements.size(); count ++)
	{
		uiElements.at(count).draw();
	}
}

void C_Screen::drawToScreen()
{
	this->draw();
	C_Graphics::switchBuffer();
}

std::string C_Screen::drawToTexture()
{
	C_Rect dstrect(0, 0, SWIDTH, SHEIGHT);
	C_Graphics::renderToTextureStart("images32/" + this->getTexPath(), &dstrect);
	this->draw();
	C_Graphics::renderToTextureEnd("images32/" + this->getTexPath(), &dstrect);
	return this->getTexPath();
}

bool C_Screen::parseInput(SDL_Event &event)
{
	if (event.type == SDL_QUIT)
	{
        g_Game.setState(STATE_QUIT);
		return true;
	}
	
    if (event.type == SDL_MOUSEMOTION)
    {
        mousePos.x = event.motion.x;
        mousePos.y = event.motion.y;
        return false;
    }
	
	for (int count = 0; count < buttons.size(); count ++)
	{
		if (buttons.at(count).parseInput(event, mousePos))
		{
			inputReceived = true;
			buttons.at(count).activate();
			this->setup();
			return true;
		}
	}
	
	return false;
}

void C_Screen::animateIn(C_Screen* previousScreen)
{
	std::string texture = this->drawToTexture();
	C_Graphics::fadeIn(texture, false, true);
}

bool C_Screen::shouldRedraw()
{
	if (inputReceived)
	{
		inputReceived = false;
		return true;
	}
	else
	{
		return false;
	}
}

void C_Screen::frame()
{
	inputReceived = true;
	while(this->state == g_Game.getState())
	{
		this->update();
		if (shouldRedraw())
			this->drawToScreen();
	
	    SDL_Event event;
	    while(SDL_PollEvent(&event))
		{
			this->parseInput(event);
			if (this->toUpdate)
			{
				break;
			}
		}
		
		if (this->toUpdate)
		{
			toUpdate = false;
			break;
		}
		
		g_Game.delay(this->delay);
	}
}

/// ///////////////////////////////////////////////
/// C_GameScreen
/// ///////////////////////////////////////////////

void C_GameScreen::setup()
{
	lastInputTime = time(0);
	blinked = false;
	C_Graphics::resizeWindow(g_Game.width * C_Board::TWIDTH, g_Game.height * C_Board::THEIGHT);
}

void C_GameScreen::draw()
{
	C_Graphics::clearScreen();
	C_Graphics::drawGame();
	g_Game.checkForCompletion();
}

void C_GameScreen::drawToScreen()
{
	if ((time(0) - this->lastInputTime) > 10 && !this->blinked && !g_Game.getPlayerPtr()->isSleeping())
	{
		g_Game.animateTile(g_Game.getPlayerPtr(), "animations/sleepyowl.png", 4, 50);
		blinked = true;
	}

	if ((time(0) - this->lastInputTime) > 30 && !g_Game.getPlayerPtr()->isSleeping())
	{
		g_Game.animateTile(g_Game.getPlayerPtr(), "animations/sleepyowl.png", 4, 100);
		g_Game.getPlayerPtr()->sleep();
	}
	
	this->draw();
	
	if ((time(0) - this->lastInputTime) <= 30 && g_Game.getPlayerPtr()->isSleeping())
	{
		g_Game.getPlayerPtr()->wakeUp();
		this->blinked = false;
	}
}

bool C_GameScreen::parseInput(SDL_Event &event)
{
    switch (event.type)
    {
        // check for keypresses
        case SDL_KEYDOWN:
        {
			inputReceived = true;
            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
				g_Game.setState(STATE_MENU);
				return true;
            }
			if (event.key.keysym.sym == SDLK_UP)
			{
				lastInputTime = time(0);			
				g_Game.getPlayerPtr()->move(0, -1);
				return true;
			}
			if (event.key.keysym.sym == SDLK_DOWN)
			{
				lastInputTime = time(0);			
				g_Game.getPlayerPtr()->move(0, 1);
				return true;
			}
			if (event.key.keysym.sym == SDLK_RIGHT)
			{
				lastInputTime = time(0);
				g_Game.getPlayerPtr()->move(1, 0);
				return true;
			}
			if (event.key.keysym.sym == SDLK_LEFT)
			{
				lastInputTime = time(0);
				g_Game.getPlayerPtr()->move(-1, 0);
			    return true;
			}
            break;
        }
		case SDL_MOUSEBUTTONDOWN:
		{
			inputReceived = true;
            mousePos.x = event.motion.x;
            mousePos.y = event.motion.y;
			
			// get current x,y tile.
			int mouseX = mousePos.x / C_Board::TWIDTH;
			int mouseY = mousePos.y / C_Board::THEIGHT;
			
			int playerX = g_Game.getPlayerPtr()->getPosX();
			int playerY = g_Game.getPlayerPtr()->getPosY();
			
			if (pointInTriangle(mousePos.x, mousePos.y, playerX * C_Board::TWIDTH, playerY * C_Board::THEIGHT, 0, 0, 0, C_Board::THEIGHT * g_Game.height))
			{
				lastInputTime = time(0);
				g_Game.getPlayerPtr()->move(-1, 0);
			    return true;
			}
			else if (pointInTriangle(mousePos.x, mousePos.y, playerX * C_Board::TWIDTH, playerY * C_Board::THEIGHT, 0, 0, C_Board::TWIDTH * g_Game.width, 0))
			{
				lastInputTime = time(0);
				g_Game.getPlayerPtr()->move(0, -1);
				return true;
			}
			else if (pointInTriangle(mousePos.x, mousePos.y, playerX * C_Board::TWIDTH, playerY * C_Board::THEIGHT, C_Board::TWIDTH * g_Game.width, 0, C_Board::TWIDTH * g_Game.width, C_Board::THEIGHT * g_Game.height))
			{
				lastInputTime = time(0);
				g_Game.getPlayerPtr()->move(1, 0);
				return true;
			}
			else if (pointInTriangle(mousePos.x, mousePos.y, playerX * C_Board::TWIDTH, playerY * C_Board::THEIGHT, 0, C_Board::THEIGHT * g_Game.height, C_Board::TWIDTH * g_Game.width, C_Board::THEIGHT * g_Game.height))
			{
				lastInputTime = time(0);
				g_Game.getPlayerPtr()->move(0, 1);
				return true;
			}
			break;
		}
		case SDL_MOUSEMOTION:
		{
			inputReceived = true;
            mousePos.x = event.motion.x;
            mousePos.y = event.motion.y;
			
			// get current x,y tile.
			int mouseX = mousePos.x / C_Board::TWIDTH;
			int mouseY = mousePos.y / C_Board::THEIGHT;
			
			int playerX = g_Game.getPlayerPtr()->getPosX();
			int playerY = g_Game.getPlayerPtr()->getPosY();
			
			if (mouseX == playerX && mouseY == playerY)
			{ // space
				SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND));
			}
			else if (pointInTriangle(mousePos.x, mousePos.y, playerX * C_Board::TWIDTH, playerY * C_Board::THEIGHT, 0, 0, 0, C_Board::THEIGHT * g_Game.height) || pointInTriangle(mousePos.x, mousePos.y, playerX * C_Board::TWIDTH, playerY * C_Board::THEIGHT, C_Board::TWIDTH * g_Game.width, 0, C_Board::TWIDTH * g_Game.width, C_Board::THEIGHT * g_Game.height))
			{ // left
				SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE));
			}
			else
			{ // down
				SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS));
			}
			break;
		}
	}	
	return this->C_Screen::parseInput(event);
}

void C_GameScreen::animateIn(C_Screen* previousScreen)
{
#ifndef _DEBUG_
	int delay = 20;
	if (previousScreen->getState() == STATE_GAMEOVER) delay = 5;
	else if (previousScreen->getState() == STATE_LEVELSELECT) delay = 15;
	C_Graphics::animateZoomOnPlayer(false, delay); // square out from player
#endif
}

void C_GameScreen::cleanup()
{
	this->C_Screen::cleanup();
	SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW));
	if (g_Game.isNetworkGame)
	{
		g_Game.bcastMoveThread.join();
	}
}

bool C_GameScreen::shouldRedraw()
{
	return C_Screen::shouldRedraw();
}

void C_GameScreen::update()
{
	if (((time(0) - this->lastInputTime) > 10 && !this->blinked && !g_Game.getPlayerPtr()->isSleeping()) || ((time(0) - this->lastInputTime) > 30 && !g_Game.getPlayerPtr()->isSleeping()) || ((time(0) - this->lastInputTime) <= 30 && g_Game.getPlayerPtr()->isSleeping()))
	{
		inputReceived = true;
	}
}

/// ///////////////////////////////////////////////
/// C_EndScreen
/// ///////////////////////////////////////////////

void C_EndScreen::setup()
{
	if (g_Game.isNetworkGame && g_Game.totalMoves > g_Game.totalOpponentMoves)
	{
		uiElements.at(0).hide();
		uiElements.at(1).show();
	}
	else
	{
		uiElements.at(1).hide();
		uiElements.at(0).show();
	}
}

/// ///////////////////////////////////////////////
/// C_UI
/// ///////////////////////////////////////////////

C_UI::C_UI() : mainMenuScreen("menu.png",
								list_of(C_Button("menu_play.png", SDLK_p, []() { g_Game.generateLevel(false, g_Game.chosenBoxLines, g_Game.chosenBoxLines+2, g_Game.chosenNumGoals); g_Game.setState(STATE_PLAY); g_Game.isNetworkGame = false; }, SWIDTH/2, 329))
									(C_Button("menu_generate.png", SDLK_g, []() { g_Game.continuallyGenerateLevels(); }, SWIDTH/2, 374))
									(C_Button("menu_load.png", SDLK_l, []() { if (g_Game.loadLevel()) { g_Game.setState(STATE_PLAY); g_Game.isNetworkGame = false; } }, SWIDTH/2, 419))
									(C_Button("menu_host.png", SDLK_h, []() { g_Game.hostGame(); }, SWIDTH/2, 487))
									(C_Button("menu_join.png", SDLK_j, []() { g_Game.joinGame(); }, SWIDTH/2, 533))
									(C_Button("menu_quit.png", SDLK_q, []() { g_Game.setState(STATE_QUIT); }, SWIDTH/2, 604))
									(C_Button("", SDLK_w, []() { g_Game.addSize(3); g_Game.refreshWindowCaption(); }, 0, 0))
									(C_Button("", SDLK_s, []() { g_Game.addSize(-3); g_Game.refreshWindowCaption(); }, 0, 0))
									(C_Button("", SDLK_LEFT, []() { g_Game.diffSelected = !g_Game.diffSelected; }, 0, 0))
									(C_Button("", SDLK_RIGHT, []() { g_Game.diffSelected = !g_Game.diffSelected; }, 0, 0))
									(C_Button("", SDLK_UP, []() { if (g_Game.diffSelected) g_Game.chosenBoxLines ++; else g_Game.chosenNumGoals ++; g_Game.refreshWindowCaption(); }, 0, 0))
									(C_Button("", SDLK_DOWN, []() { if (g_Game.diffSelected) g_Game.chosenBoxLines --; else g_Game.chosenNumGoals --; g_Game.refreshWindowCaption(); }, 0, 0)),
								list_of(C_UIElement("menu_title.png", 0, 0)),
								STATE_MENU),
				gameScreen(),
				endScreen("menu.png",
								list_of(C_Button("menu_menu.png", SDLK_m, []() { g_Game.setState(STATE_MENU); }, SWIDTH/2, 632))
									(C_Button("", SDLK_ESCAPE, []() { g_Game.setState(STATE_QUIT); }, 0, 0)),
								list_of(C_UIElement("menu_won.png", SWIDTH/2, 369))
									(C_UIElement("menu_lost.png", SWIDTH/2, 369)),
								STATE_ENDSCREEN),
				currentScreen(&mainMenuScreen),
				previousScreen(NULL)
{
	screens.push_back(NULL); // quit
	screens.push_back(&gameScreen); // play
	screens.push_back(&mainMenuScreen);
	screens.push_back(&endScreen); // end screen
}

void C_UI::start()
{
	currentScreen->setup();
	while(true)
	{
		g_Game.refreshWindowCaption();

		currentScreen->frame();
		currentScreen->cleanup();
		
		this->clearEventQueue();
		
		int currentState = g_Game.getState();
		if (currentState == STATE_QUIT)
		{
			break;
		}
		
		previousScreen = currentScreen;
		
		currentScreen = screens.at(currentState);
		currentScreen->setup(); // may change current state
		if (currentState != g_Game.getState())
		{
			currentState = g_Game.getState();
			currentScreen = screens.at(currentState);
			currentScreen->setup(); // may change current state
		}
		
		currentScreen->mousePos = previousScreen->mousePos;
				
		currentScreen->animateIn(previousScreen);
	}
}

void C_UI::clearEventQueue()
{
    SDL_Event event;
    while (SDL_PollEvent(&event));
}
