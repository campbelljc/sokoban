#include <string>
#include <time.h>

#include "graphics.h"
#include "g_Game.h"
#include "coord.h"

#include <boost/filesystem.hpp>
using namespace boost::filesystem;

SDL_Window* C_Graphics::screen;
C_Rect C_Graphics::view;
boost::unordered_map<std::string, GLuint> C_Graphics::image; //Handle to texture
boost::unordered_map<std::string, C_Size> C_Graphics::imageSize; //Handle to texture

int C_Graphics::init()
{
	view.x = view.y = 0;
	view.h = SHEIGHT;
	view.w = SWIDTH;
	
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    { // initialize sdl video
		showMessageBox("SDL error", "Couldn't initialize SDL: " + toString(SDL_GetError()));
        return 0;
    }

    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	screen = SDL_CreateWindow("521 project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SWIDTH, SHEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
	if (SDL_GL_CreateContext(screen) == NULL || screen == NULL)
	{
		showMessageBox("SDL error", "Couldn't initialize SDL window or context: " + toString(SDL_GetError()));
		return 0;
	}
				
	if (initOpenGL() < 0) return 0;
	
	loadImages();
	
    return true;
}

int C_Graphics::initOpenGL()
{
    glEnable(GL_TEXTURE_2D);
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glViewport(0, 0, SWIDTH, SHEIGHT);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, SWIDTH,
            SHEIGHT, 0.0f,
            -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //Transparency
    glEnable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glAlphaFunc(GL_GREATER,0.0f);
    
    glGenTextures(1, &image["*screen copy"]); //A texture to store the screenshot in
    glBindTexture(GL_TEXTURE_2D, image["*screen copy"]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    imageSize["*screen copy"].w = SWIDTH;
	imageSize["*screen copy"].h = SHEIGHT;
	
	return 1;
}

void C_Graphics::draw(C_Entity *ent, C_Rect dstrect, int tileHeight, int tileWidth, C_Board* board)
{
	// we assume that if the entity has more than one state, then it will default
	// to the most top-left tile-sized state in the image unless specified
	// in the case statements below.
    C_Rect srcRect(0, 0, tileHeight, tileWidth);
    int id = ent->getId();
	switch (id)
 	{
		case EID_PLAYER:
        { // the player is still alive. THIS WAS A TRIUMPH
			if (((C_Player*)ent)->isSleeping())
			{
				srcRect.x = 3*tileHeight;
				draw("animations/sleepyowl.png", &srcRect, &dstrect);
				break;
			}
			else
			{
				draw(ent->getImage(), 0, &dstrect);
				break;
			}
        }
		default:
		{ // works for non-animated 1 tile images, and n-step animated images.
	            draw(ent->getImage(), 0, &dstrect);
				break;
		}
	}
}

void C_Graphics::drawGame(bool fSwitchBuffer, C_Entity* skip)
{
	if (g_Game.isNetworkGame)
	{
		C_Rect dstrect(0,0,C_Board::TWIDTH,C_Board::THEIGHT);
		drawGame(g_Game.getBoardPtr(), false, skip, dstrect, true);
	
		C_Rect dstrect2((g_Game.width+1)*C_Board::TWIDTH, 0, C_Board::TWIDTH,C_Board::THEIGHT);
		drawGame(g_Game.getOpponentBoardPtr(), true, NULL, dstrect2, false);
	}
	else
	{
		C_Rect dstrect(0,0,C_Board::TWIDTH,C_Board::THEIGHT);
		drawGame(g_Game.getBoardPtr(), true, skip, dstrect, true);
	}
}

void C_Graphics::drawGame(C_Board* board, bool fSwitchBuffer, C_Entity* skip, C_Rect dstrect, bool cls)
{
	C_Rect origrect = dstrect;
    if (cls) clearScreen();
	
	int tileHeight = dstrect.h;
	int tileWidth = dstrect.w;

    C_Entity *ent;

    //Level1(Ground)
    for (int h = 0; h < C_Board::MAX_HEIGHT; h++)
    {
        for (int w = 0; w < C_Board::MAX_WIDTH; w++)
        {
            ent = board->getEntityPtrG(w,h);
            if (ent == 0)
            {
                dstrect.x += tileHeight;
                continue;
            }
            draw(ent, dstrect, tileHeight, tileWidth, board);
            dstrect.x += tileWidth;
        }
        dstrect.x = origrect.x;
        dstrect.y += tileHeight;
    }

	dstrect.x = origrect.x;
    dstrect.y = origrect.y;

    //Level2(regular)
    for (int h = 0; h < C_Board::MAX_HEIGHT; h++)
    {
        for (int w = 0; w < C_Board::MAX_WIDTH; w++)
        {
            ent = board->getEntityPtr(w,h);
            if (ent == 0 || ent == skip)
            {
                dstrect.x += tileHeight;
                continue;
            }
            draw(ent, dstrect, tileHeight, tileWidth, board);
            dstrect.x += tileWidth;
        }
        dstrect.x = origrect.x;
        dstrect.y += tileHeight;
    }
	
    if ( fSwitchBuffer )
    {
        switchBuffer();
        return;
    }
	
	if (cls) saveScreenshot();
}

bool C_Graphics::loadImages()
{ // load all images in images/ directory except for the mod folders.
	boost::filesystem::recursive_directory_iterator end;
	boost::filesystem::recursive_directory_iterator dir("images32/");
	for ( ; dir != end; ++dir )
	{
		std::string path = dir->path().string();
		if (path.find(".png") != std::string::npos)
		{ // it is an image, so we can load it.
			std::replace( path.begin(), path.end(), '\\', '/');
			loadImage(path.c_str(), true);
		}
	}
	
    return true;
}

void C_Graphics::loadImage(const char* path, bool alpha = true)
{ // some code here borrowed from nehe's opengl tutorial
    if (image[path] != 0) return;
    SDL_Surface *result = IMG_Load(path);
    if (result)
    {
        {
            GLint colours;
            GLenum textureFormat;
            colours = result->format->BytesPerPixel;
            switch(colours)
            {
                case 4: //RGBA (With alpha)
                    if (result->format->Rmask == 0x000000ff)
						textureFormat = GL_RGBA;
#ifndef WIN32
		            else
						textureFormat = GL_BGRA;
#endif		
                    break;
                case 3: //RGB
                    if (result->format->Rmask == 0x000000ff)
						textureFormat = GL_RGB;
#ifndef WIN32					
		            else
						textureFormat = GL_BGR;
#endif					
                    break;
            }

			std::cout<<"Path:"<<path<<"\n";
            imageSize[path].w = result->w;
            imageSize[path].h = result->h;
			
            glGenTextures(1, &image[path]);
            glBindTexture(GL_TEXTURE_2D, image[path]);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			
			glTexImage2D(GL_TEXTURE_2D,
                         0,
                         colours,
                         result->w,
                         result->h,
                         0,
                         textureFormat,
                         GL_UNSIGNED_BYTE,
                         result->pixels);
			
            SDL_FreeSurface(result);
        }
    }
    else
    {
        std::cout << SDL_GetError() << std::endl;
        showMessageBox("Image error", "Couldn't load " + toString(path) + ": " + toString(SDL_GetError()));
    }
}

void C_Graphics::draw(std::string name, C_Rect *src, C_Rect *dest)
{
	name = "images32/" + name;

    glBindTexture(GL_TEXTURE_2D, image[name]);
	
    //Source coordinates not defined
    if (src == 0)
    {
        glBegin( GL_QUADS );
            glTexCoord2i( 0, 0 );
            glVertex3f( dest->x, dest->y, 0.f );

			glTexCoord2i( 1, 0 );
			glVertex3f( dest->x+dest->w, dest->y, 0.f );

	        glTexCoord2i( 1, 1 );
            glVertex3f( dest->x+dest->w, dest->y+dest->h, 0.f ); //TODO: make dest->h equal to the source image's height

			glTexCoord2i( 0, 1 );
            glVertex3f( dest->x, dest->y+dest->h, 0.f );
        glEnd();
        return;
    }
    //Source coordinates defined
    glBegin( GL_QUADS );
	glTexCoord2f(((float)src->x)/((float)imageSize[name].w), ((float)src->y)/((float)imageSize[name].h));		
    glVertex3f( dest->x, dest->y, 0.f );

	glTexCoord2f(((float)(src->x+src->w))/((float)imageSize[name].w), ((float)src->y)/((float)imageSize[name].h));
    glVertex3f( dest->x+dest->w, dest->y, 0.f );

	glTexCoord2f(((float)(src->x+src->w))/((float)imageSize[name].w), ((float)(src->y+src->h))/((float)imageSize[name].h));
    glVertex3f( dest->x+dest->w, (dest->y+dest->h), 0.f ); //TODO: make dest->h equal to the source image's height

	glTexCoord2f(((float)src->x)/((float)imageSize[name].w), ((float)(src->y+src->h))/((float)imageSize[name].h));
    glVertex3f( dest->x, (dest->y+dest->h), 0.f );
    glEnd();

}

void C_Graphics::animate(C_Entity* ent)
{
	// Draw the game into the texture buffer.
    drawGame(false,ent);
    C_Coord pointA = ent->getPrevPos();
    C_Coord pointB = ent->getPos();
    C_Coord delta = pointB-pointA;
    delta.x *= C_Board::TWIDTH;
    delta.y *= C_Board::THEIGHT;

	C_Rect srcRect(0, 0, C_Board::TWIDTH, C_Board::THEIGHT);
    C_Rect dest(pointA.x*C_Board::TWIDTH, pointA.y*C_Board::THEIGHT, C_Board::TWIDTH, C_Board::THEIGHT);
    C_Rect dest2(pointA.x*C_Board::TWIDTH, pointA.y*C_Board::THEIGHT, C_Board::TWIDTH, C_Board::THEIGHT);
    delta.x /= 5;
    delta.y /= 5;
	
	int tileHeight = dest.h;
	int tileWidth = dest.w;
	
	int delayms = 5;
	
	for (int i = 0; i < 5; i++)
	{ // animation (5 stages)
		dest.x += delta.x;
		dest.y += delta.y;
		dest2.x += delta.x;
		dest2.y += delta.y;
						
		drawScreenshot();
		
			//Draw the image onto the screen.
            draw(ent->getImage(), &srcRect, &dest);
			
		// Displays contents of buffer to the screen.
		switchBuffer();
		g_Game.delay(delayms);
	}
}

void C_Graphics::animateTileOverSpace(C_Entity* ent, const char* filename, int steps, int stepDelay, C_Rect dstrectDelta, C_Rect srcrectDelta)
{
	C_Rect srcrect(0, 0, srcrectDelta.w, srcrectDelta.h);
    C_Rect dstrect((ent->getPosX()*C_Board::TWIDTH)+dstrectDelta.x, (ent->getPosY()*C_Board::THEIGHT)+dstrectDelta.y, dstrectDelta.w, dstrectDelta.h);
    C_Rect dstrect2((ent->getPosX()*C_Board::TWIDTH)+dstrectDelta.x, (ent->getPosY()*C_Board::THEIGHT)+dstrectDelta.y, dstrectDelta.w, dstrectDelta.h);
	
	drawGame(false, ent);
	
	for (int count = 0; count < steps; count ++)
	{
		clearScreen();
		drawScreenshot();
		if (srcrectDelta.h >= srcrectDelta.w)
			srcrect.x = count*C_Board::TWIDTH;
		else srcrect.y = count*C_Board::THEIGHT;

		draw(filename, &srcrect, &dstrect);
		switchBuffer();
		
		g_Game.delay(stepDelay);
	}
	
	drawScreenshot();
	if (srcrectDelta.h >= srcrectDelta.w)
		srcrect.x = (steps-1)*C_Board::TWIDTH;
	else srcrect.y = (steps-1)*C_Board::THEIGHT;
	draw(filename, &srcrect, &dstrect);
	saveScreenshot();
}

void C_Graphics::drawScreenshot()
{
    glBindTexture(GL_TEXTURE_2D, image["*screen copy"]);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin( GL_QUADS );
        glTexCoord2i( 0, 1 );
        glVertex3f( (float)view.x, (float)view.y, 0.f );

        glTexCoord2i( 1, 1 );
        glVertex3f( (float)(view.x+view.w), (float)view.y, 0.f );

		int height = view.h;

        glTexCoord2i( 1, 0 );
        glVertex3f( (float)(view.x+view.w), (float)(view.y+height), 0.f );

        glTexCoord2i( 0, 0 );
        glVertex3f( (float)view.x, (float)(view.y+height), 0.f );
    glEnd();
}

void C_Graphics::toggleFullscreen()
{
	Uint32 flags = SDL_GetWindowFlags(screen);
	flags ^= SDL_WINDOW_FULLSCREEN;
	SDL_SetWindowFullscreen(screen, flags);
}

void C_Graphics::clearScreen()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
}

void C_Graphics::saveScreenshot(bool frontToBack)
{
	glBindTexture(GL_TEXTURE_2D, image["*screen copy"]);
	if (frontToBack)
	{
		glReadBuffer (GL_FRONT);
		glDrawBuffer (GL_BACK);
		glCopyPixels (0, 0, SWIDTH, SHEIGHT, GL_COLOR);
		glReadBuffer (GL_BACK);
		glDrawBuffer (GL_BACK);
	}
	
	int height = SHEIGHT;
	
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, SWIDTH, height, 0);
}

void C_Graphics::switchBuffer()
{
	SDL_GL_SwapWindow(screen);
}

void C_Graphics::setWindowTitle(std::string text)
{
	SDL_SetWindowTitle(screen, text.c_str());
}

void C_Graphics::showMessageBox(std::string title, std::string msg)
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), msg.c_str(), NULL);
}

void C_Graphics::resizeWindow(int width, int height)
{
	SDL_SetWindowSize(screen, width, height);
}

C_Size C_Graphics::getImageSize(std::string path)
{
	return imageSize[path];
}

void C_Graphics::drawUI(std::string path_, int midXPos, int topYPos)
{
	std::string path = path_;
	C_Size imageSize = getImageSize("images32/" + path);
	
	C_Rect dstRect(midXPos-(imageSize.w/2), topYPos, imageSize.w, imageSize.h);
	if (midXPos < (imageSize.w/2)) dstRect.x = midXPos;
	else if (midXPos == SWIDTH) dstRect.x = midXPos - imageSize.w;
	if (topYPos > SHEIGHT-imageSize.h) dstRect.y = topYPos - imageSize.h;
		
    draw(path, 0, &dstRect);
}

void C_Graphics::renderToTextureStart(std::string name, C_Rect *dest)
{
	GLuint gFBO = NULL;
	glGenFramebuffers(1, &gFBO);
	glBindFramebuffer( GL_FRAMEBUFFER, gFBO );
	
	glGenTextures(1, &image[name]);
	glBindTexture(GL_TEXTURE_2D, image[name]);
  
	GLuint* mPixels32 = new GLuint[ dest->h*dest->w ];
	memset( mPixels32, 0, sizeof( sizeof(GLuint) * (dest->h * dest->w) )); //dest->h*dest->w * 4 );
	
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);// GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //GL_LINEAR );
		
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, dest->w, dest->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, mPixels32 );
	delete mPixels32;

	glBindTexture( GL_TEXTURE_2D, NULL );

	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, image[name], 0 );

	glClearColor(1.0, 1.0, 1.0, 0.0);
	glClear( GL_COLOR_BUFFER_BIT );
	
	imageSize[name].w = dest->w;
	imageSize[name].h = dest->h;
}

void C_Graphics::renderToTextureEnd(std::string name, C_Rect *dest)
{
	glBindFramebuffer( GL_FRAMEBUFFER, NULL );
}

void C_Graphics::fadeIn(std::string name, bool frontToBack, bool flip)
{
	saveScreenshot(frontToBack);
			
    glPushMatrix();
	for (GLfloat a = 0.0f; a <= 1.1f; a+=0.05f) //Q: Huh? Why does a <= 1.1f go up to 1.0f?
	{
	    g_Game.delay(40);

	    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); // clear the screen
		
	    drawScreenshot(); // draw the previously saved screenshot
		
        glBindTexture(GL_TEXTURE_2D, image[name]);
	    glColor4f(1.0f, 1.0f, 1.0f, a);
		
		glBegin( GL_QUADS ); // then we draw the image to fade in, with the appropriate alpha value.
		if (flip)
		{
	        glTexCoord2i( 0, 1 );
	        glVertex3f( 0, 0, 0.f );

	        glTexCoord2i( 1, 1 );
	        glVertex3f( SWIDTHF, 0, 0.f );

	        glTexCoord2i( 1, 0 );
	        glVertex3f( SWIDTHF, SHEIGHTF, 0.f );

	        glTexCoord2i( 0, 0 );
	        glVertex3f( 0, SHEIGHTF, 0.f );
		}
		else
		{
            glTexCoord2i( 0, 0 );
            glVertex3f( 0, 0, 0.f );

            glTexCoord2i( 1, 0 );
            glVertex3f( SWIDTHF, 0, 0.f );

            glTexCoord2i( 1, 1 );
            glVertex3f( SWIDTHF, SHEIGHTF, 0.f );

            glTexCoord2i( 0, 1 );
            glVertex3f( 0, SHEIGHTF, 0.f );
		}
        glEnd();
	    switchBuffer(); // push contents of back buffer to screen.
	}
	glPopMatrix();
    
}