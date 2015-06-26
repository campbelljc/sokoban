#ifndef SDLHEADERS_H
#define SDLHEADERS_H

#ifdef __cplusplus
    #include <cstdlib>
#else
    #include <stdlib.h>
#endif

	#ifdef __APPLE__
		#include <OpenGL/gl.h>
	#endif

	#ifndef __APPLE__
		#define NO_SDL_GLEXT
	#endif

	#ifdef WIN32
	    #include <GL/glew.h>
	    #include <GL/freeglut.h>
	    #include <GL/glut.h>
	#endif

#ifdef __APPLE__
    #include <SDL2/SDL.h>
    #include <SDL2_image/SDL_image.h>
	#include <SDL2_net/SDL_net.h>
	#include <curl/curl.h>
	#include <sys/socket.h>
#else
    #include <SDL.h>
    #include <SDL_image.h>
    #include <SDL_opengl.h>
	#include <SDL_net.h>
	#include <winsock2.h>
#endif

#endif
