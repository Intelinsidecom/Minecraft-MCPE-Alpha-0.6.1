#ifndef NET_MINECRAFT_CLIENT_RENDERER__gles_H__
#define NET_MINECRAFT_CLIENT_RENDERER__gles_H__

#include "../../platform/log.h"
#include "../Options.h"

// Android should always run OPENGL_ES
#if defined(ANDROID) || defined(__APPLE__) || defined(RPI)
    #define OPENGL_ES
#endif

// Other systems might run it, if they #define OPENGL_ES
#if defined(OPENGL_ES) // || defined(ANDROID)
	#define USE_VBO
	#define GL_QUADS 0x0007
    #if defined(__APPLE__)
        #import <OpenGLES/ES2/gl.h>
        #import <OpenGLES/ES2/glext.h>
    #else
        #ifdef WIN32
            #define GL_APICALL
        #endif
        #include <GLES2/gl2.h>
        #if defined(ANDROID)
            #include <GLES2/gl2ext.h>
        #endif
    #endif
#else
    // Uglyness to fix redeclaration issues
    #ifdef WIN32
	   #include <WinSock2.h>
	   #include <Windows.h>
	#endif
	#include <gl/glew.h>
	#include <gl/GL.h>

	#define glFogx(a,b)	glFogi(a,b)
	#define glOrthof(a,b,c,d,e,f) glOrtho(a,b,c,d,e,f)
#endif


#define GLERRDEBUG 1
#if GLERRDEBUG
//#define GLERR(x) if((x) != 0) { LOGI("GLError: " #x "(%d)\n", __LINE__) }
#define GLERR(x) do { const int errCode = glGetError(); if (errCode != 0) LOGE("OpenGL ERROR @%d: #%d @ (%s : %d)\n", x, errCode, __FILE__, __LINE__); } while (0)
#else
#define GLERR(x) x
#endif

void anGenBuffers(GLsizei n, GLuint* buffer);

#ifdef USE_VBO
#define drawArrayVT_NoState drawArrayVT
#define drawArrayVTC_NoState drawArrayVTC
void drawArrayVT(int bufferId, int vertices, int vertexSize = 24, unsigned int mode = GL_TRIANGLES);
#ifndef drawArrayVT_NoState
//void drawArrayVT_NoState(int bufferId, int vertices, int vertexSize = 24);
#endif
void drawArrayVTC(int bufferId, int vertices, int vertexSize = 24);
#ifndef drawArrayVTC_NoState
void drawArrayVTC_NoState(int bufferId, int vertices, int vertexSize = 24);
#endif
#endif

#ifndef GL_SMOOTH
#define GL_SMOOTH 0x1D01
#endif
#ifndef GL_FLAT
#define GL_FLAT 0x1D00
#endif

#ifndef GL_ALPHA_TEST
#define GL_ALPHA_TEST 0x0BC0
#endif
#ifndef GL_FOG
#define GL_FOG 0x0B60
#endif
#ifndef GL_LIGHTING
#define GL_LIGHTING 0x0B50
#endif
#ifndef GL_COLOR_MATERIAL
#define GL_COLOR_MATERIAL 0x0B57
#endif
#ifndef GL_VERTEX_ARRAY
#define GL_VERTEX_ARRAY 0x8074
#endif
#ifndef GL_TEXTURE_COORD_ARRAY
#define GL_TEXTURE_COORD_ARRAY 0x8078
#endif
#ifndef GL_COLOR_ARRAY
#define GL_COLOR_ARRAY 0x8076
#endif
#ifndef GL_NORMAL_ARRAY
#define GL_NORMAL_ARRAY 0x8075
#endif
#ifndef GL_FOG_DENSITY
#define GL_FOG_DENSITY 0x0B62
#endif
#ifndef GL_FOG_START
#define GL_FOG_START 0x0B63
#endif
#ifndef GL_FOG_END
#define GL_FOG_END 0x0B64
#endif
#ifndef GL_FOG_MODE
#define GL_FOG_MODE 0x0B65
#endif
#ifndef GL_FOG_COLOR
#define GL_FOG_COLOR 0x0B66
#endif
#ifndef GL_EXP
#define GL_EXP 0x0800
#endif
#ifndef GL_EXP2
#define GL_EXP2 0x0801
#endif
#ifndef GL_PERSPECTIVE_CORRECTION_HINT
#define GL_PERSPECTIVE_CORRECTION_HINT 0x0C50
#endif
#ifndef GL_MODELVIEW
#define GL_MODELVIEW 0x1700
#endif
#ifndef GL_PROJECTION
#define GL_PROJECTION 0x1701
#endif
#ifndef GL_MODELVIEW_MATRIX
#define GL_MODELVIEW_MATRIX 0x0BA6
#endif
#ifndef GL_PROJECTION_MATRIX
#define GL_PROJECTION_MATRIX 0x0BA7
#endif

void glInit();
void gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar);
int glhUnProjectf(	float winx, float winy, float winz,
					float *modelview, float *projection,
					int *viewport, float *objectCoordinate);

struct RenderState {
    bool fogEnabled;
    int fogMode;
    float fogStart;
    float fogEnd;
    float fogDensity;
    float fogColor[4];
    bool alphaTestEnabled;
    float alphaTestRef;
    GLenum shadeModel;
    bool texture2DEnabled;
    float color[4];
};
extern RenderState renderState;

void mc_glFogf(GLenum pname, GLfloat param);
void mc_glFogfv(GLenum pname, const GLfloat *params);
void mc_glFogx(GLenum pname, GLint param);
void mc_glEnable(GLenum cap);
void mc_glDisable(GLenum cap);
void mc_glAlphaFunc(GLenum func, GLclampf ref);

#define glFogf mc_glFogf
#define glFogfv mc_glFogfv
#define glFogx mc_glFogx
#define glEnable mc_glEnable
#define glDisable mc_glDisable
#define glAlphaFunc mc_glAlphaFunc


void mc_glTranslatef(float x, float y, float z);
void mc_glRotatef(float angle, float x, float y, float z);
void mc_glScalef(float x, float y, float z);
void mc_glPushMatrix();
void mc_glPopMatrix();
void mc_glLoadIdentity();
void mc_glMultMatrixf(const GLfloat* m);
void mc_glMatrixMode(GLenum mode);

#define glTranslatef mc_glTranslatef
#define glRotatef mc_glRotatef
#define glScalef mc_glScalef
#define glPushMatrix mc_glPushMatrix
#define glPopMatrix mc_glPopMatrix
#define glLoadIdentity mc_glLoadIdentity
#define glMultMatrixf mc_glMultMatrixf
#define glMatrixMode mc_glMatrixMode

void mc_glShadeModel(GLenum mode);
void mc_glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);

#define glShadeModel mc_glShadeModel
#define glOrthof mc_glOrthof

void mc_glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void mc_glEnableClientState(GLenum array);
void mc_glDisableClientState(GLenum array);
void mc_glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void mc_glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void mc_glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void mc_glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer);
void mc_glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz);
void mc_glDrawArrays(GLenum mode, GLint first, GLsizei count);
void mc_glGetFloatv(GLenum pname, GLfloat *params);
void mc_glHint(GLenum target, GLenum mode);

#define glColor4f mc_glColor4f
#define glEnableClientState mc_glEnableClientState
#define glDisableClientState mc_glDisableClientState
#define glVertexPointer mc_glVertexPointer
#define glColorPointer mc_glColorPointer
#define glTexCoordPointer mc_glTexCoordPointer
#define glNormalPointer mc_glNormalPointer
#define glNormal3f mc_glNormal3f
#define glDrawArrays mc_glDrawArrays
#define glGetFloatv mc_glGetFloatv
#define glHint mc_glHint
void mc_glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
void mc_glClear(GLbitfield mask);
void mc_glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
#define glViewport mc_glViewport
#define glClear mc_glClear
#define glClearColor mc_glClearColor

// Used for "debugging" (...). Obviously stupid dependency on Options (and ugly gl*2 calls).
#ifdef GLDEBUG
	#define glTranslatef2(x, y, z) do{ if (Options::debugGl) LOGI("glTrans @ %s:%d: %f,%f,%f\n", __FILE__, __LINE__, x, y, z); glTranslatef(x, y, z); GLERR(0); } while(0)
	#define glRotatef2(a, x, y, z) do{ if (Options::debugGl) LOGI("glRotat @ %s:%d: %f,%f,%f,%f\n", __FILE__, __LINE__, a, x, y, z); glRotatef(a, x, y, z); GLERR(1); } while(0)
	#define glScalef2(x, y, z) do{ if (Options::debugGl) LOGI("glScale @ %s:%d: %f,%f,%f\n", __FILE__, __LINE__, x, y, z); glScalef(x, y, z); GLERR(2); } while(0)
	#define glPushMatrix2() do{ if (Options::debugGl) LOGI("glPushM @ %s:%d\n", __FILE__, __LINE__); glPushMatrix(); GLERR(3); } while(0)
	#define glPopMatrix2() do{ if (Options::debugGl) LOGI("glPopM  @ %s:%d\n", __FILE__, __LINE__); glPopMatrix(); GLERR(4); } while(0)
	#define glLoadIdentity2() do{ if (Options::debugGl) LOGI("glLoadI @ %s:%d\n", __FILE__, __LINE__); glLoadIdentity(); GLERR(5); } while(0)

	#define glVertexPointer2(a, b, c, d) do{ if (Options::debugGl) LOGI("glVertexPtr @ %s:%d : %d\n", __FILE__, __LINE__, 0); glVertexPointer(a, b, c, d); GLERR(6); } while(0)
	#define glColorPointer2(a, b, c, d) do{ if (Options::debugGl) LOGI("glColorPtr @ %s:%d : %d\n", __FILE__, __LINE__, 0); glColorPointer(a, b, c, d); GLERR(7); } while(0)
	#define glTexCoordPointer2(a, b, c, d) do{ if (Options::debugGl) LOGI("glTexPtr @ %s:%d : %d\n", __FILE__, __LINE__, 0); glTexCoordPointer(a, b, c, d); GLERR(8); } while(0)
	#define glEnableClientState2(s) do{ if (Options::debugGl) LOGI("glEnableClient @ %s:%d : %d\n", __FILE__, __LINE__, 0); glEnableClientState(s); GLERR(9); } while(0)
	#define glDisableClientState2(s) do{ if (Options::debugGl) LOGI("glDisableClient @ %s:%d : %d\n", __FILE__, __LINE__, 0); glDisableClientState(s); GLERR(10); } while(0)
	#define glDrawArrays2(m, o, v) do{ if (Options::debugGl) LOGI("glDrawA @ %s:%d : %d\n", __FILE__, __LINE__, 0); glDrawArrays(m,o,v); GLERR(11); } while(0)

	#define glTexParameteri2(m, o, v) do{ if (Options::debugGl) LOGI("glTexParameteri @ %s:%d : %d\n", __FILE__, __LINE__, v); glTexParameteri(m,o,v); GLERR(12); } while(0)
	#define glTexImage2D2(a,b,c,d,e,f,g,height,i) do{ if (Options::debugGl) LOGI("glTexImage2D @ %s:%d : %d\n", __FILE__, __LINE__, 0); glTexImage2D(a,b,c,d,e,f,g,height,i); GLERR(13); } while(0)
	#define glTexSubImage2D2(a,b,c,d,e,f,g,height,i) do{ if (Options::debugGl) LOGI("glTexSubImage2D @ %s:%d : %d\n", __FILE__, __LINE__, 0); glTexSubImage2D(a,b,c,d,e,f,g,height,i); GLERR(14); } while(0)
	#define glGenBuffers2(s, id) do{ if (Options::debugGl) LOGI("glGenBuffers @ %s:%d : %d\n", __FILE__, __LINE__, id); anGenBuffers(s, id); GLERR(15); } while(0)
	#define glBindBuffer2(s, id) do{ if (Options::debugGl) LOGI("glBindBuffer @ %s:%d : %d\n", __FILE__, __LINE__, id); glBindBuffer(s, id); GLERR(16); } while(0)
	#define glBufferData2(a, b, c, d) do{ if (Options::debugGl) LOGI("glBufferData @ %s:%d : %d\n", __FILE__, __LINE__, d); glBufferData(a, b, c, d); GLERR(17); } while(0)
	#define glBindTexture2(m, z) do{ if (Options::debugGl) LOGI("glBindTexture @ %s:%d : %d\n", __FILE__, __LINE__, z); glBindTexture(m, z); GLERR(18); } while(0)

	#define glEnable2(s) do{ if (Options::debugGl) LOGI("glEnable @ %s:%d : %d\n", __FILE__, __LINE__, s); glEnable(s); GLERR(19); } while(0)
	#define glDisable2(s) do{ if (Options::debugGl) LOGI("glDisable @ %s:%d : %d\n", __FILE__, __LINE__, s); glDisable(s); GLERR(20); } while(0)
	
	#define glColor4f2(r, g, b, a) do{ if (Options::debugGl) LOGI("glColor4f2 @ %s:%d : (%f,%f,%f,%f)\n", __FILE__, __LINE__, r,g,b,a); glColor4f(r,g,b,a); GLERR(21); } while(0)

	//#define glBlendMode2(s) do{ if (Options::debugGl) LOGI("glEnable @ %s:%d : %d\n", __FILE__, __LINE__, s); glEnable(s); GLERR(19); } while(0)
	#define glBlendFunc2(src, dst) do{ if (Options::debugGl) LOGI("glBlendFunc @ %s:%d : %d - %d\n", __FILE__, __LINE__, src, dst); glBlendFunc(src, dst); GLERR(23); } while(0)
	#define glShadeModel2(s) do{ if (Options::debugGl) LOGI("glShadeModel @ %s:%d : %d\n", __FILE__, __LINE__, s); glShadeModel(s); GLERR(25); } while(0)
#else
	#define glTranslatef2	glTranslatef
	#define glRotatef2		glRotatef
	#define glScalef2		glScalef
	#define glPushMatrix2	glPushMatrix
	#define glPopMatrix2	glPopMatrix
	#define glLoadIdentity2 glLoadIdentity

	#define glVertexPointer2	glVertexPointer
	#define glColorPointer2		glColorPointer
	#define glTexCoordPointer2  glTexCoordPointer
	#define glEnableClientState2  glEnableClientState
	#define glDisableClientState2 glDisableClientState
	#define glDrawArrays2		glDrawArrays

	#define glTexParameteri2 glTexParameteri
	#define glTexImage2D2	 glTexImage2D
	#define glTexSubImage2D2 glTexSubImage2D
	#define glGenBuffers2	 anGenBuffers
	#define glBindBuffer2	 glBindBuffer
	#define glBufferData2	 glBufferData
	#define glBindTexture2	 glBindTexture

	#define glEnable2		glEnable
	#define glDisable2		glDisable

	#define glColor4f2		glColor4f
	#define glBlendFunc2	glBlendFunc
	#define glShadeModel2	glShadeModel
#endif

//
// Extensions
//
#ifdef WIN32
	#define glGetProcAddress(a) wglGetProcAddress(a)
#else
	#define glGetProcAddress(a) (void*(0))
#endif



#endif /*NET_MINECRAFT_CLIENT_RENDERER__gles_H__ */
