#ifndef GLES_LOADER_H
#define GLES_LOADER_H

#if defined(__APPLE__)
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#else
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif
#if defined(WIN32) || defined(_WIN32)
#ifndef GL_APICALL
#define GL_APICALL
#endif
#endif
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Shader functions
typedef GLuint (GL_APIENTRY *PFNGLCREATESHADER)(GLenum type);
#if defined(__APPLE__)
    typedef void (GL_APIENTRY *PFNGLSHADERSOURCE)(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
#else
    typedef void (GL_APIENTRY *PFNGLSHADERSOURCE)(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
#endif
typedef void (GL_APIENTRY *PFNGLCOMPILESHADER)(GLuint shader);
typedef void (GL_APIENTRY *PFNGLGETSHADERIV)(GLuint shader, GLenum pname, GLint* params);
typedef void (GL_APIENTRY *PFNGLGETSHADERINFOLOG)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef GLuint (GL_APIENTRY *PFNGLCREATEPROGRAM)(void);
typedef void (GL_APIENTRY *PFNGLATTACHSHADER)(GLuint program, GLuint shader);
typedef void (GL_APIENTRY *PFNGLLINKPROGRAM)(GLuint program);
typedef void (GL_APIENTRY *PFNGLGETPROGRAMIV)(GLuint program, GLenum pname, GLint* params);
typedef void (GL_APIENTRY *PFNGLGETPROGRAMINFOLOG)(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void (GL_APIENTRY *PFNGLUSEPROGRAM)(GLuint program);
typedef GLint (GL_APIENTRY *PFNGLGETATTRIBLOCATION)(GLuint program, const GLchar* name);
typedef GLint (GL_APIENTRY *PFNGLGETUNIFORMLOCATION)(GLuint program, const GLchar* name);
typedef void (GL_APIENTRY *PFNGLUNIFORMMATRIX4FV)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (GL_APIENTRY *PFNGLUNIFORM1F)(GLint location, GLfloat v0);
typedef void (GL_APIENTRY *PFNGLUNIFORM1I)(GLint location, GLint v0);
typedef void (GL_APIENTRY *PFNGLUNIFORM4F)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (GL_APIENTRY *PFNGLENABLEVERTEXATTRIBARRAY)(GLuint index);
typedef void (GL_APIENTRY *PFNGLDISABLEVERTEXATTRIBARRAY)(GLuint index);
typedef void (GL_APIENTRY *PFNGLVERTEXATTRIBPOINTER)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
typedef void (GL_APIENTRY *PFNGLVERTEXATTRIB4F)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (GL_APIENTRY *PFNGLDELETESHADER)(GLuint shader);
typedef void (GL_APIENTRY *PFNGLDELETEPROGRAM)(GLuint program);
typedef void (GL_APIENTRY *PFNGLVIEWPORT)(GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GL_APIENTRY *PFNGLCLEAR)(GLbitfield mask);
typedef void (GL_APIENTRY *PFNGLCLEARCOLOR)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (GL_APIENTRY *PFNGLSCISSOR)(GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GL_APIENTRY *PFNGLREADPIXELS)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
typedef void (GL_APIENTRY *PFNGLDEPTHRANGEF)(GLclampf zNear, GLclampf zFar);
typedef void (GL_APIENTRY *PFNGLDEPTHFUNC)(GLenum func);
typedef void (GL_APIENTRY *PFNGLCULLFACE)(GLenum mode);
typedef void (GL_APIENTRY *PFNGLPOLYGONOFFSET)(GLfloat factor, GLfloat units);
typedef void (GL_APIENTRY *PFNGLLINEWIDTH)(GLfloat width);
typedef GLenum (GL_APIENTRY *PFNGLGETERROR)(void);
typedef void (GL_APIENTRY *PFNGLDEPTHMASK)(GLboolean flag);
typedef void (GL_APIENTRY *PFNGLCOLORMASK)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
typedef void (GL_APIENTRY *PFNGLBINDTEXTURE)(GLenum target, GLuint texture);
typedef void (GL_APIENTRY *PFNGLBINDBUFFER)(GLenum target, GLuint buffer);
typedef void (GL_APIENTRY *PFNGLDELETEBUFFERS)(GLsizei n, const GLuint* buffers);
typedef void (GL_APIENTRY *PFNGLENABLE)(GLenum cap);
typedef void (GL_APIENTRY *PFNGLDISABLE)(GLenum cap);
typedef void (GL_APIENTRY *PFNGLBLENDFUNC)(GLenum sfactor, GLenum dfactor);
typedef void (GL_APIENTRY *PFNGLDRAWARRAYS)(GLenum mode, GLint first, GLsizei count);
typedef void (GL_APIENTRY *PFNGLGETFLOATV)(GLenum pname, GLfloat* params);
typedef void (GL_APIENTRY *PFNGLGENTEXTURES)(GLsizei n, GLuint* textures);
typedef void (GL_APIENTRY *PFNGLDELETETEXTURES)(GLsizei n, const GLuint* textures);
typedef void (GL_APIENTRY *PFNGLTEXIMAGE2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
typedef void (GL_APIENTRY *PFNGLTEXSUBIMAGE2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
typedef void (GL_APIENTRY *PFNGLTEXPARAMETERI)(GLenum target, GLenum pname, GLint param);
typedef void (GL_APIENTRY *PFNGLBUFFERDATA)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
typedef const GLubyte* (GL_APIENTRY *PFNGLGETSTRING)(GLenum name);

#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
#endif

// Global function pointers
extern PFNGLCREATESHADER p_glCreateShader;
extern PFNGLSHADERSOURCE p_glShaderSource;
extern PFNGLCOMPILESHADER p_glCompileShader;
extern PFNGLGETSHADERIV p_glGetShaderiv;
extern PFNGLGETSHADERINFOLOG p_glGetShaderInfoLog;
extern PFNGLCREATEPROGRAM p_glCreateProgram;
extern PFNGLATTACHSHADER p_glAttachShader;
extern PFNGLLINKPROGRAM p_glLinkProgram;
extern PFNGLGETPROGRAMIV p_glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOG p_glGetProgramInfoLog;
extern PFNGLUSEPROGRAM p_glUseProgram;
extern PFNGLGETATTRIBLOCATION p_glGetAttribLocation;
extern PFNGLGETUNIFORMLOCATION p_glGetUniformLocation;
extern PFNGLUNIFORMMATRIX4FV p_glUniformMatrix4fv;
extern PFNGLUNIFORM1F p_glUniform1f;
extern PFNGLUNIFORM1I p_glUniform1i;
extern PFNGLUNIFORM4F p_glUniform4f;
extern PFNGLENABLEVERTEXATTRIBARRAY p_glEnableVertexAttribArray;
extern PFNGLDISABLEVERTEXATTRIBARRAY p_glDisableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTER p_glVertexAttribPointer;
extern PFNGLVERTEXATTRIB4F p_glVertexAttrib4f;
extern PFNGLDELETESHADER p_glDeleteShader;
extern PFNGLDELETEPROGRAM p_glDeleteProgram;

// Additional OpenGL function pointers
extern PFNGLVIEWPORT p_glViewport;
extern PFNGLCLEAR p_glClear;
extern PFNGLCLEARCOLOR p_glClearColor;
extern PFNGLSCISSOR p_glScissor;
extern PFNGLREADPIXELS p_glReadPixels;
extern PFNGLDEPTHRANGEF p_glDepthRangef;
extern PFNGLDEPTHFUNC p_glDepthFunc;
extern PFNGLCULLFACE p_glCullFace;
extern PFNGLPOLYGONOFFSET p_glPolygonOffset;
extern PFNGLLINEWIDTH p_glLineWidth;
extern PFNGLGETERROR p_glGetError;
extern PFNGLDEPTHMASK p_glDepthMask;
extern PFNGLCOLORMASK p_glColorMask;
extern PFNGLBINDTEXTURE p_glBindTexture;
extern PFNGLBINDBUFFER p_glBindBuffer;
extern PFNGLDELETEBUFFERS p_glDeleteBuffers;
extern PFNGLENABLE p_glEnable;
extern PFNGLDISABLE p_glDisable;
extern PFNGLBLENDFUNC p_glBlendFunc;
extern PFNGLDRAWARRAYS p_glDrawArrays;
extern PFNGLGETFLOATV p_glGetFloatv;
extern PFNGLGENTEXTURES p_glGenTextures;
extern PFNGLDELETETEXTURES p_glDeleteTextures;
extern PFNGLTEXIMAGE2D p_glTexImage2D;
extern PFNGLTEXSUBIMAGE2D p_glTexSubImage2D;
extern PFNGLTEXPARAMETERI p_glTexParameteri;
extern PFNGLBUFFERDATA p_glBufferData;
extern PFNGLGETSTRING p_glGetString;


void LoadGLESFunctions();
bool GLESFunctionsLoaded();

#ifdef __cplusplus
}
#endif

#endif
