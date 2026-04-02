#ifndef GLES_LOADER_H
#define GLES_LOADER_H

#if defined(__APPLE__)
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#else
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

void LoadGLESFunctions();
bool GLESFunctionsLoaded();

#ifdef __cplusplus
}
#endif

#endif
