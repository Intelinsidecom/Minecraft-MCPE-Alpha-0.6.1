#include "GLESLoader.h"
#include "../../platform/log.h"

PFNGLCREATESHADER p_glCreateShader = nullptr;
PFNGLSHADERSOURCE p_glShaderSource = nullptr;
PFNGLCOMPILESHADER p_glCompileShader = nullptr;
PFNGLGETSHADERIV p_glGetShaderiv = nullptr;
PFNGLGETSHADERINFOLOG p_glGetShaderInfoLog = nullptr;
PFNGLCREATEPROGRAM p_glCreateProgram = nullptr;
PFNGLATTACHSHADER p_glAttachShader = nullptr;
PFNGLLINKPROGRAM p_glLinkProgram = nullptr;
PFNGLGETPROGRAMIV p_glGetProgramiv = nullptr;
PFNGLGETPROGRAMINFOLOG p_glGetProgramInfoLog = nullptr;
PFNGLUSEPROGRAM p_glUseProgram = nullptr;
PFNGLGETATTRIBLOCATION p_glGetAttribLocation = nullptr;
PFNGLGETUNIFORMLOCATION p_glGetUniformLocation = nullptr;
PFNGLUNIFORMMATRIX4FV p_glUniformMatrix4fv = nullptr;
PFNGLUNIFORM1I p_glUniform1i = nullptr;
PFNGLUNIFORM4F p_glUniform4f = nullptr;
PFNGLENABLEVERTEXATTRIBARRAY p_glEnableVertexAttribArray = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAY p_glDisableVertexAttribArray = nullptr;
PFNGLVERTEXATTRIBPOINTER p_glVertexAttribPointer = nullptr;
PFNGLVERTEXATTRIB4F p_glVertexAttrib4f = nullptr;
PFNGLDELETESHADER p_glDeleteShader = nullptr;
PFNGLDELETEPROGRAM p_glDeleteProgram = nullptr;

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

    glesLoaded = (p_glCreateShader != nullptr && p_glCreateProgram != nullptr);
}

bool GLESFunctionsLoaded() {
    return glesLoaded;
}
