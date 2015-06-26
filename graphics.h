#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <string>
#include "coord.h"
#include "board.h"
#include "SDLheaders.h"
#include "entity.h"
#include "coord.h"

#include <boost/unordered_map.hpp>

class C_Entity;

class C_GraphicsData
{
    public:
        int id;
        char data[16];
};

class C_Size
{
    public:
        int w;
        int h;
};

class C_Rect;

class C_Graphics
{
    public:
          static int init();
		
          static void drawGame(bool = true, C_Entity* = 0);
  		  static void drawUI(std::string path_, int midXPos, int topYPos);
  		  static C_Size getImageSize(std::string path);
		
		  static void toggleFullscreen();
		  static void setWindowTitle(std::string text);
		
		  static void drawScreenshot();
          static void saveScreenshot(bool frontToBack = false);
		
          static void animate(C_Entity*);
		  static void animateTileOverSpace(C_Entity* ent, const char* filename, int steps, int stepDelay, C_Rect dstrectDelta, C_Rect srcrectDelta);

          static void switchBuffer();

		  static void resizeWindow(int width, int height);
          static void clearScreen();
          static void fadeIn(std::string name, bool frontToback = false, bool flip = false);
		  
  		static void renderToTextureStart(std::string name, C_Rect *dest);
  		static void renderToTextureEnd(std::string name, C_Rect *dest);
		  
    private:
          static bool loadImages();
          static void loadImage(const char*, bool);
		
		// Draw the game board to the buffer; if fSwitchBuffer=true, then render it to the screen afterwards.
		  static void drawGame(C_Board* board, bool fSwitchBuffer, C_Entity* skip, C_Rect dstrect, bool cls);
		
		// Draw a single entity or image to the screen.
          static void draw(C_Entity*, C_Rect, int tileHeight, int tileWidth, C_Board* board);
          static void draw(std::string name, C_Rect *src, C_Rect *dest);
		 
		static int initOpenGL();
		static void showMessageBox(std::string, std::string);
				
        static SDL_Window* screen;
        static C_Rect view;
		static boost::unordered_map<std::string, GLuint> image; //Handle to texture
		static boost::unordered_map<std::string, C_Size> imageSize; //Handle to texture
};

#endif
