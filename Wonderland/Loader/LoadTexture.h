#ifndef __LOADTEXTURE_H__
#define __LOADTEXTURE_H__

#include <string>
#include <windows.h>
#include <glad/glad.h> // Switched to GLAD

GLuint LoadTexture(const std::string& fname);
GLuint LoadCubemap(const std::string filename[6]);

#endif