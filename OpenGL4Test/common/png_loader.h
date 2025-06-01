#ifndef _PNG_LOADER_H_
#define _PNG_LOADER_H_

#include <GL/glew.h>

GLuint png_load_LIBPNG(const char * file_name, int * width, int * height, bool bMipMap);
GLuint png_load_SOIL(const char * file_name, int * width, int * height, bool bMipMap=false);
GLuint CubeMap_load_SOIL(); 

#endif
