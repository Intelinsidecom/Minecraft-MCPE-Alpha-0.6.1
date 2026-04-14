#include "GLESLoader.h"
#include "../../platform/log.h"

// Define function pointers as global extern C
#ifdef __cplusplus
extern "C" {
#endif

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

PFNGLVIEWPORT p_glViewport = NULL;
PFNGLCLEAR p_glClear = NULL;
PFNGLCLEARCOLOR p_glClearColor = NULL;
PFNGLSCISSOR p_glScissor = NULL;
PFNGLREADPIXELS p_glReadPixels = NULL;
PFNGLDEPTHRANGEF p_glDepthRangef = NULL;
PFNGLDEPTHFUNC p_glDepthFunc = NULL;
PFNGLCULLFACE p_glCullFace = NULL;
PFNGLPOLYGONOFFSET p_glPolygonOffset = NULL;
PFNGLLINEWIDTH p_glLineWidth = NULL;
PFNGLGETERROR p_glGetError = NULL;
PFNGLDEPTHMASK p_glDepthMask = NULL;
PFNGLCOLORMASK p_glColorMask = NULL;
PFNGLBINDTEXTURE p_glBindTexture = NULL;
PFNGLBINDBUFFER p_glBindBuffer = NULL;
PFNGLDELETEBUFFERS p_glDeleteBuffers = NULL;
PFNGLENABLE p_glEnable = NULL;
PFNGLDISABLE p_glDisable = NULL;
PFNGLBLENDFUNC p_glBlendFunc = NULL;
PFNGLDRAWARRAYS p_glDrawArrays = NULL;
PFNGLGETFLOATV p_glGetFloatv = NULL;
PFNGLGENTEXTURES p_glGenTextures = NULL;
PFNGLDELETETEXTURES p_glDeleteTextures = NULL;
PFNGLTEXIMAGE2D p_glTexImage2D = NULL;
PFNGLTEXSUBIMAGE2D p_glTexSubImage2D = NULL;
PFNGLTEXPARAMETERI p_glTexParameteri = NULL;
PFNGLBUFFERDATA p_glBufferData = NULL;
PFNGLGETSTRING p_glGetString = NULL;

static bool glesLoaded = false;

void LoadGLESFunctions() {
    if (glesLoaded) return;

#if !defined(__APPLE__)
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
    p_glViewport = (PFNGLVIEWPORT)eglGetProcAddress("glViewport");
    p_glClear = (PFNGLCLEAR)eglGetProcAddress("glClear");
    p_glClearColor = (PFNGLCLEARCOLOR)eglGetProcAddress("glClearColor");
    p_glScissor = (PFNGLSCISSOR)eglGetProcAddress("glScissor");
    p_glReadPixels = (PFNGLREADPIXELS)eglGetProcAddress("glReadPixels");
    p_glDepthRangef = (PFNGLDEPTHRANGEF)eglGetProcAddress("glDepthRangef");
    p_glDepthFunc = (PFNGLDEPTHFUNC)eglGetProcAddress("glDepthFunc");
    p_glCullFace = (PFNGLCULLFACE)eglGetProcAddress("glCullFace");
    p_glPolygonOffset = (PFNGLPOLYGONOFFSET)eglGetProcAddress("glPolygonOffset");
    p_glLineWidth = (PFNGLLINEWIDTH)eglGetProcAddress("glLineWidth");
    p_glGetError = (PFNGLGETERROR)eglGetProcAddress("glGetError");
    p_glDepthMask = (PFNGLDEPTHMASK)eglGetProcAddress("glDepthMask");
    p_glColorMask = (PFNGLCOLORMASK)eglGetProcAddress("glColorMask");
    p_glBindTexture = (PFNGLBINDTEXTURE)eglGetProcAddress("glBindTexture");
    p_glBindBuffer = (PFNGLBINDBUFFER)eglGetProcAddress("glBindBuffer");
    p_glDeleteBuffers = (PFNGLDELETEBUFFERS)eglGetProcAddress("glDeleteBuffers");
    p_glEnable = (PFNGLENABLE)eglGetProcAddress("glEnable");
    p_glDisable = (PFNGLDISABLE)eglGetProcAddress("glDisable");
    p_glBlendFunc = (PFNGLBLENDFUNC)eglGetProcAddress("glBlendFunc");
    p_glDrawArrays = (PFNGLDRAWARRAYS)eglGetProcAddress("glDrawArrays");
    p_glGetFloatv = (PFNGLGETFLOATV)eglGetProcAddress("glGetFloatv");
    p_glGenTextures = (PFNGLGENTEXTURES)eglGetProcAddress("glGenTextures");
    p_glDeleteTextures = (PFNGLDELETETEXTURES)eglGetProcAddress("glDeleteTextures");
    p_glTexImage2D = (PFNGLTEXIMAGE2D)eglGetProcAddress("glTexImage2D");
    p_glTexSubImage2D = (PFNGLTEXSUBIMAGE2D)eglGetProcAddress("glTexSubImage2D");
    p_glTexParameteri = (PFNGLTEXPARAMETERI)eglGetProcAddress("glTexParameteri");
    p_glBufferData = (PFNGLBUFFERDATA)eglGetProcAddress("glBufferData");
    p_glGetString = (PFNGLGETSTRING)eglGetProcAddress("glGetString");
#else
    p_glCreateShader = ::glCreateShader;
    p_glShaderSource = ::glShaderSource;
    p_glCompileShader = ::glCompileShader;
    p_glGetShaderiv = ::glGetShaderiv;
    p_glGetShaderInfoLog = ::glGetShaderInfoLog;
    p_glCreateProgram = ::glCreateProgram;
    p_glAttachShader = ::glAttachShader;
    p_glLinkProgram = ::glLinkProgram;
    p_glGetProgramiv = ::glGetProgramiv;
    p_glGetProgramInfoLog = ::glGetProgramInfoLog;
    p_glUseProgram = ::glUseProgram;
    p_glGetAttribLocation = ::glGetAttribLocation;
    p_glGetUniformLocation = ::glGetUniformLocation;
    p_glUniformMatrix4fv = ::glUniformMatrix4fv;
    p_glUniform1f = ::glUniform1f;
    p_glUniform1i = ::glUniform1i;
    p_glUniform4f = ::glUniform4f;
    p_glEnableVertexAttribArray = ::glEnableVertexAttribArray;
    p_glDisableVertexAttribArray = ::glDisableVertexAttribArray;
    p_glVertexAttribPointer = ::glVertexAttribPointer;
    p_glVertexAttrib4f = ::glVertexAttrib4f;
    p_glDeleteShader = ::glDeleteShader;
    p_glDeleteProgram = ::glDeleteProgram;
    p_glViewport = ::glViewport;
    p_glClear = ::glClear;
    p_glClearColor = ::glClearColor;
    p_glScissor = ::glScissor;
    p_glReadPixels = ::glReadPixels;
    p_glDepthRangef = ::glDepthRangef;
    p_glDepthFunc = ::glDepthFunc;
    p_glCullFace = ::glCullFace;
    p_glPolygonOffset = ::glPolygonOffset;
    p_glLineWidth = ::glLineWidth;
    p_glGetError = ::glGetError;
    p_glDepthMask = ::glDepthMask;
    p_glColorMask = ::glColorMask;
    p_glBindTexture = ::glBindTexture;
    p_glBindBuffer = ::glBindBuffer;
    p_glDeleteBuffers = ::glDeleteBuffers;
    p_glEnable = ::glEnable;
    p_glDisable = ::glDisable;
    p_glBlendFunc = ::glBlendFunc;
    p_glDrawArrays = ::glDrawArrays;
    p_glGetFloatv = ::glGetFloatv;
    p_glGenTextures = ::glGenTextures;
    p_glDeleteTextures = ::glDeleteTextures;
    p_glTexImage2D = ::glTexImage2D;
    p_glTexSubImage2D = ::glTexSubImage2D;
    p_glTexParameteri = ::glTexParameteri;
    p_glBufferData = ::glBufferData;
    p_glGetString = ::glGetString;
#endif

#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
    if (!p_glCreateShader) p_glCreateShader = (PFNGLCREATESHADER)::glCreateShader;
    if (!p_glShaderSource) p_glShaderSource = (PFNGLSHADERSOURCE)::glShaderSource;
    if (!p_glCompileShader) p_glCompileShader = (PFNGLCOMPILESHADER)::glCompileShader;
    if (!p_glGetShaderiv) p_glGetShaderiv = (PFNGLGETSHADERIV)::glGetShaderiv;
    if (!p_glGetShaderInfoLog) p_glGetShaderInfoLog = (PFNGLGETSHADERINFOLOG)::glGetShaderInfoLog;
    if (!p_glCreateProgram) p_glCreateProgram = (PFNGLCREATEPROGRAM)::glCreateProgram;
    if (!p_glAttachShader) p_glAttachShader = (PFNGLATTACHSHADER)::glAttachShader;
    if (!p_glLinkProgram) p_glLinkProgram = (PFNGLLINKPROGRAM)::glLinkProgram;
    if (!p_glGetProgramiv) p_glGetProgramiv = (PFNGLGETPROGRAMIV)::glGetProgramiv;
    if (!p_glGetProgramInfoLog) p_glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOG)::glGetProgramInfoLog;
    if (!p_glUseProgram) p_glUseProgram = (PFNGLUSEPROGRAM)::glUseProgram;
    if (!p_glGetAttribLocation) p_glGetAttribLocation = (PFNGLGETATTRIBLOCATION)::glGetAttribLocation;
    if (!p_glGetUniformLocation) p_glGetUniformLocation = (PFNGLGETUNIFORMLOCATION)::glGetUniformLocation;
    if (!p_glUniformMatrix4fv) p_glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FV)::glUniformMatrix4fv;
    if (!p_glUniform1f) p_glUniform1f = (PFNGLUNIFORM1F)::glUniform1f;
    if (!p_glUniform1i) p_glUniform1i = (PFNGLUNIFORM1I)::glUniform1i;
    if (!p_glUniform4f) p_glUniform4f = (PFNGLUNIFORM4F)::glUniform4f;
    if (!p_glEnableVertexAttribArray) p_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAY)::glEnableVertexAttribArray;
    if (!p_glDisableVertexAttribArray) p_glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAY)::glDisableVertexAttribArray;
    if (!p_glVertexAttribPointer) p_glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTER)::glVertexAttribPointer;
    if (!p_glVertexAttrib4f) p_glVertexAttrib4f = (PFNGLVERTEXATTRIB4F)::glVertexAttrib4f;
    if (!p_glDeleteShader) p_glDeleteShader = (PFNGLDELETESHADER)::glDeleteShader;
    if (!p_glDeleteProgram) p_glDeleteProgram = (PFNGLDELETEPROGRAM)::glDeleteProgram;
    if (!p_glViewport) p_glViewport = (PFNGLVIEWPORT)::glViewport;
    if (!p_glClear) p_glClear = (PFNGLCLEAR)::glClear;
    if (!p_glClearColor) p_glClearColor = (PFNGLCLEARCOLOR)::glClearColor;
    if (!p_glScissor) p_glScissor = (PFNGLSCISSOR)::glScissor;
    if (!p_glReadPixels) p_glReadPixels = (PFNGLREADPIXELS)::glReadPixels;
    if (!p_glDepthRangef) p_glDepthRangef = (PFNGLDEPTHRANGEF)::glDepthRangef;
    if (!p_glDepthFunc) p_glDepthFunc = (PFNGLDEPTHFUNC)::glDepthFunc;
    if (!p_glCullFace) p_glCullFace = (PFNGLCULLFACE)::glCullFace;
    if (!p_glPolygonOffset) p_glPolygonOffset = (PFNGLPOLYGONOFFSET)::glPolygonOffset;
    if (!p_glLineWidth) p_glLineWidth = (PFNGLLINEWIDTH)::glLineWidth;
    if (!p_glGetError) p_glGetError = (PFNGLGETERROR)::glGetError;
    if (!p_glDepthMask) p_glDepthMask = (PFNGLDEPTHMASK)::glDepthMask;
    if (!p_glColorMask) p_glColorMask = (PFNGLCOLORMASK)::glColorMask;
    if (!p_glBindTexture) p_glBindTexture = (PFNGLBINDTEXTURE)::glBindTexture;
    if (!p_glGenTextures) p_glGenTextures = (PFNGLGENTEXTURES)::glGenTextures;
    if (!p_glDeleteTextures) p_glDeleteTextures = (PFNGLDELETETEXTURES)::glDeleteTextures;
    if (!p_glTexImage2D) p_glTexImage2D = (PFNGLTEXIMAGE2D)::glTexImage2D;
    if (!p_glTexSubImage2D) p_glTexSubImage2D = (PFNGLTEXSUBIMAGE2D)::glTexSubImage2D;
    if (!p_glTexParameteri) p_glTexParameteri = (PFNGLTEXPARAMETERI)::glTexParameteri;
    if (!p_glEnable) p_glEnable = (PFNGLENABLE)::glEnable;
    if (!p_glDisable) p_glDisable = (PFNGLDISABLE)::glDisable;
    if (!p_glBlendFunc) p_glBlendFunc = (PFNGLBLENDFUNC)::glBlendFunc;
    if (!p_glDrawArrays) p_glDrawArrays = (PFNGLDRAWARRAYS)::glDrawArrays;
    if (!p_glGetFloatv) p_glGetFloatv = (PFNGLGETFLOATV)::glGetFloatv;
    if (!p_glBindBuffer) p_glBindBuffer = (PFNGLBINDBUFFER)::glBindBuffer;
    if (!p_glDeleteBuffers) p_glDeleteBuffers = (PFNGLDELETEBUFFERS)::glDeleteBuffers;
    if (!p_glBufferData) p_glBufferData = (PFNGLBUFFERDATA)::glBufferData;
    if (!p_glGetString) p_glGetString = (PFNGLGETSTRING)::glGetString;
#endif

    glesLoaded = (p_glCreateShader != NULL && p_glCreateProgram != NULL);
}

bool GLESFunctionsLoaded() {
    return glesLoaded;
}

#ifdef __cplusplus
}
#endif
