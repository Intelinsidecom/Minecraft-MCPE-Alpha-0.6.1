#include "GLESLoader.h"
#include "../../platform/log.h"

PFNGLCREATESHADER p_glCreateShader = NULL;
PFNGLSHADERSOURCE p_glShaderSource = NULL;
PFNGLCOMPILESHADER p_glCompileShader = NULL;
PFNGLGETSHADERIV p_glGetShaderiv = NULL;
PFNGLGETSHADERINFOLOG p_glGetShaderInfoLog = NULL;
PFNGLCREATEPROGRAM p_glCreateProgram = NULL;
PFNGLATTACHSHADER p_glAttachShader = NULL;
PFNGLLINKPROGRAM p_glLinkProgram = NULL;
PFNGLGETPROGRAMIV p_glGetProgramiv = NULL;
PFNGLGETPROGRAMINFOLOG p_glGetProgramInfoLog = NULL;
PFNGLUSEPROGRAM p_glUseProgram = NULL;
PFNGLGETATTRIBLOCATION p_glGetAttribLocation = NULL;
PFNGLGETUNIFORMLOCATION p_glGetUniformLocation = NULL;
PFNGLUNIFORMMATRIX4FV p_glUniformMatrix4fv = NULL;
PFNGLUNIFORM1F p_glUniform1f = NULL;
PFNGLUNIFORM1I p_glUniform1i = NULL;
PFNGLUNIFORM4F p_glUniform4f = NULL;
PFNGLENABLEVERTEXATTRIBARRAY p_glEnableVertexAttribArray = NULL;
PFNGLDISABLEVERTEXATTRIBARRAY p_glDisableVertexAttribArray = NULL;
PFNGLVERTEXATTRIBPOINTER p_glVertexAttribPointer = NULL;
PFNGLVERTEXATTRIB4F p_glVertexAttrib4f = NULL;
PFNGLDELETESHADER p_glDeleteShader = NULL;
PFNGLDELETEPROGRAM p_glDeleteProgram = NULL;

static bool glesLoaded = false;

void LoadGLESFunctions() {
    if (glesLoaded) return;

    p_glCreateShader = (PFNGLCREATESHADER)eglGetProcAddress("glCreateShader");
    p_glShaderSource = (PFNGLSHADERSOURCE)eglGetProcAddress("glShaderSource");
    p_glCompileShader = (PFNGLCOMPILESHADER)eglGetProcAddress("glCompileShader");
    p_glGetShaderiv = (PFNGLGETSHADERIV)eglGetProcAddress("glGetShaderiv");
    p_glGetShaderInfoLog = (PFNGLGETSHADERINFOLOG)eglGetProcAddress("glGetShaderInfoLog");
    p_glCreateProgram = (PFNGLCREATEPROGRAM)eglGetProcAddress("glCreateProgram");
    p_glAttachShader = (PFNGLATTACHSHADER)eglGetProcAddress("glAttachShader");
    p_glLinkProgram = (PFNGLLINKPROGRAM)eglGetProcAddress("glLinkProgram");
    p_glGetProgramiv = (PFNGLGETPROGRAMIV)eglGetProcAddress("glGetProgramiv");
    p_glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOG)eglGetProcAddress("glGetProgramInfoLog");
    p_glUseProgram = (PFNGLUSEPROGRAM)eglGetProcAddress("glUseProgram");
    p_glGetAttribLocation = (PFNGLGETATTRIBLOCATION)eglGetProcAddress("glGetAttribLocation");
    p_glGetUniformLocation = (PFNGLGETUNIFORMLOCATION)eglGetProcAddress("glGetUniformLocation");
    p_glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FV)eglGetProcAddress("glUniformMatrix4fv");
    p_glUniform1f = (PFNGLUNIFORM1F)eglGetProcAddress("glUniform1f");
    p_glUniform1i = (PFNGLUNIFORM1I)eglGetProcAddress("glUniform1i");
    p_glUniform4f = (PFNGLUNIFORM4F)eglGetProcAddress("glUniform4f");
    p_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAY)eglGetProcAddress("glEnableVertexAttribArray");
    p_glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAY)eglGetProcAddress("glDisableVertexAttribArray");
    p_glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTER)eglGetProcAddress("glVertexAttribPointer");
    p_glVertexAttrib4f = (PFNGLVERTEXATTRIB4F)eglGetProcAddress("glVertexAttrib4f");
    p_glDeleteShader = (PFNGLDELETESHADER)eglGetProcAddress("glDeleteShader");
    p_glDeleteProgram = (PFNGLDELETEPROGRAM)eglGetProcAddress("glDeleteProgram");

    LOGI("DIAGNOSTIC: glCreateShader pointer: %p\n", p_glCreateShader);
    LOGI("DIAGNOSTIC: glCreateProgram pointer: %p\n", p_glCreateProgram);

    glesLoaded = (p_glCreateShader != NULL && p_glCreateProgram != NULL);
}

bool GLESFunctionsLoaded() {
    return glesLoaded;
}
