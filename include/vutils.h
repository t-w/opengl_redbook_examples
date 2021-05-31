#ifndef __VUTILS_H__
#define __VUTILS_H__

#include "vgl.h"

#define DEBUG_SHADER 1

#if DEBUG_SHADER == 1
#include <iostream>
#endif

void vglAttachShaderSource(GLuint prog, GLenum type, const char * source)
{
    GLuint sh;

    sh = glCreateShader(type);
    glShaderSource(sh, 1, &source, NULL);
    glCompileShader(sh);
    GLint status;
    glGetShaderiv ( sh, GL_COMPILE_STATUS, &status );
    if ( status == GL_FALSE ) {
#if DEBUG_SHADER == 1
        char buffer[4096];
        glGetShaderInfoLog ( sh, sizeof ( buffer ), NULL, buffer );
        std::cerr << "Shader compilation error: " << buffer << std::endl
                  << "Shader source: " << std::endl
                  << source << std::endl;
#endif
        glDeleteShader(sh);
        return;
    }
    glAttachShader(prog, sh);
    glDeleteShader(sh);
}

#endif /* __VUTILS_H__ */
