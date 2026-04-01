#include "gles.h"
#include <cmath>
#include <cstdio>
#include <fstream>
#include <string>
#include "Shader.h"
#include "GLESLoader.h"
#include "../../util/MatrixStack.h"

#include <EGL/egl.h>
#pragma comment(lib, "libGLESv2.lib")
#pragma comment(lib, "libEGL.lib")

static const float __glPi = 3.14159265358979323846f;

static void __gluMakeIdentityf(GLfloat m[16]);


void gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar) {
    GLfloat m[4][4];
    GLfloat sine, cotangent, deltaZ;
    GLfloat radians=(GLfloat)(fovy/2.0f*__glPi/180.0f);

    deltaZ=zFar-zNear;
    sine=(GLfloat)sin(radians);
    if ((deltaZ==0.0f) || (sine==0.0f) || (aspect==0.0f))
    {
        return;
    }
    cotangent=(GLfloat)(cos(radians)/sine);

    __gluMakeIdentityf(&m[0][0]);
    m[0][0] = cotangent / aspect;
    m[1][1] = cotangent;
    m[2][2] = -(zFar + zNear) / deltaZ;
    m[2][3] = -1.0f;
    m[3][2] = -2.0f * zNear * zFar / deltaZ;
    m[3][3] = 0;
    glMultMatrixf(&m[0][0]);
}

void __gluMakeIdentityf(GLfloat m[16]) {
    m[0] = 1;  m[4] = 0;  m[8]  = 0;  m[12] = 0;
    m[1] = 0;  m[5] = 1;  m[9]  = 0;  m[13] = 0;
    m[2] = 0;  m[6] = 0;  m[10] = 1;  m[14] = 0;
    m[3] = 0;  m[7] = 0;  m[11] = 0;  m[15] = 1;
}

static Shader* defaultShader = NULL;

static void ensureShaders() {
    if (defaultShader && defaultShader->isLoaded()) return;

    LoadGLESFunctions();

    EGLContext ctx = eglGetCurrentContext();
    EGLDisplay dpy = eglGetCurrentDisplay();

#ifndef OPENGL_ES
    static bool glewDone = false;
    if (!glewDone) {
        GLenum err = glewInit();
        printf("DIAGNOSTIC: glewInit result: %d\n", err);
        glewDone = true;
    }
#endif


    const char* paths[] = {
        "data/shaders/",
        "../../data/shaders/",
        "../data/shaders/"
    };
    
    for (int i = 0; i < 3; ++i) {
        std::string vPath = std::string(paths[i]) + "default.vertex";
        std::string fPath = std::string(paths[i]) + "default.fragment";
        
        std::ifstream f(vPath.c_str());
        if (f.good()) {
            f.close();
            if (defaultShader) delete defaultShader;
            defaultShader = new Shader(vPath, fPath);
            if (defaultShader && defaultShader->isLoaded()) {
                defaultShader->bind();
                break;
            } else {
            }
        }
    }

    if (!defaultShader || !defaultShader->isLoaded()) {
        static int failCount = 0;
        if (failCount++ < 5) {
        }
    }
}

void glInit() {
    ensureShaders();
}

void anGenBuffers(GLsizei n, GLuint* buffers) {
	static GLuint k = 1;
	for (int i = 0; i < n; ++i)
		buffers[i] = ++k;
}

#ifdef USE_VBO
void drawArrayVT(int bufferId, int vertices, int vertexSize /* = 24 */, unsigned int mode /* = GL_TRIANGLES */) {
	if (currentShader) {
		currentShader->setUniformMatrix4("u_modelView", currentStack->getTop().m);
		currentShader->setUniformMatrix4("u_projection", projectionStack.getTop().m);
	}
	glBindBuffer2(GL_ARRAY_BUFFER, bufferId);
	GLint texLoc = currentShader ? currentShader->getAttribLocation("a_texCoord") : 1;
	GLint posLoc = currentShader ? currentShader->getAttribLocation("a_position") : 0;
	
	glEnableVertexAttribArray(texLoc);
	glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, vertexSize, (GLvoid*)(3 * 4));
	
	glEnableVertexAttribArray(posLoc);
	glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, vertexSize, 0);
	
	glDrawArrays2(mode, 0, vertices);
	
	glDisableVertexAttribArray(posLoc);
	glDisableVertexAttribArray(texLoc);
}

#ifndef drawArrayVT_NoState
void drawArrayVT_NoState(int bufferId, int vertices, int vertexSize /* = 24 */) {
	//if (Options::debugGl) LOGI("drawArray\n");
	glBindBuffer2(GL_ARRAY_BUFFER, bufferId);
	
	GLint posLoc = currentShader ? currentShader->getAttribLocation("a_position") : 0;
	GLint texLoc = currentShader ? currentShader->getAttribLocation("a_texCoord") : 1;
	GLint colLoc = currentShader ? currentShader->getAttribLocation("a_color") : 2;
	
	
	glEnableVertexAttribArray(posLoc);
	glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, vertexSize, 0);
	
	glEnableVertexAttribArray(texLoc);
	glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, vertexSize, (GLvoid*) (3 * 4));
	
	glEnableVertexAttribArray(colLoc);
	glVertexAttribPointer(colLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, vertexSize, (GLvoid*) (5 * 4));
	
	glDrawArrays2(GL_TRIANGLES, 0, vertices);
	
	glDisableVertexAttribArray(posLoc);
	glDisableVertexAttribArray(texLoc);
	glDisableVertexAttribArray(colLoc);
}
#endif

void drawArrayVTC(int bufferId, int vertices, int vertexSize /* = 24 */) {
	if (currentShader) {
		currentShader->setUniformMatrix4("u_modelView", currentStack->getTop().m);
		currentShader->setUniformMatrix4("u_projection", projectionStack.getTop().m);
	}
	glBindBuffer2(GL_ARRAY_BUFFER, bufferId);
	
	GLint posLoc = currentShader ? currentShader->getAttribLocation("a_position") : 0;
	GLint texLoc = currentShader ? currentShader->getAttribLocation("a_texCoord") : 1;
	GLint colLoc = currentShader ? currentShader->getAttribLocation("a_color") : 2;

	glEnableVertexAttribArray(posLoc);
	glEnableVertexAttribArray(texLoc);
	glEnableVertexAttribArray(colLoc);

	glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, vertexSize, 0);
	glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, vertexSize, (GLvoid*) (3 * 4));
	glVertexAttribPointer(colLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, vertexSize, (GLvoid*) (5 * 4));

	glDrawArrays2(GL_TRIANGLES, 0, vertices);

	glDisableVertexAttribArray(posLoc);
	glDisableVertexAttribArray(texLoc);
	glDisableVertexAttribArray(colLoc);
}

#ifndef drawArrayVTC_NoState
void drawArrayVTC_NoState(int bufferId, int vertices, int vertexSize /* = 24 */) {
	glBindBuffer2(GL_ARRAY_BUFFER, bufferId);

	glVertexPointer2(  3, GL_FLOAT, vertexSize, 0);
	glTexCoordPointer2(2, GL_FLOAT, vertexSize, (GLvoid*) (3 * 4));
	glColorPointer2(4, GL_UNSIGNED_BYTE, vertexSize, (GLvoid*) (5*4));

	glDrawArrays2(GL_TRIANGLES, 0, vertices);
}
#endif

#endif


//
// Code borrowed from OpenGL.org
// http://www.opengl.org/wiki/GluProject_and_gluUnProject_code
// The gluUnProject code in Android seems to be broken
//

void MultiplyMatrices4by4OpenGL_FLOAT(float *result, float *matrix1, float *matrix2)
{
	result[0]=matrix1[0]*matrix2[0]+
		matrix1[4]*matrix2[1]+
		matrix1[8]*matrix2[2]+
		matrix1[12]*matrix2[3];
	result[4]=matrix1[0]*matrix2[4]+
		matrix1[4]*matrix2[5]+
		matrix1[8]*matrix2[6]+
		matrix1[12]*matrix2[7];
	result[8]=matrix1[0]*matrix2[8]+
		matrix1[4]*matrix2[9]+
		matrix1[8]*matrix2[10]+
		matrix1[12]*matrix2[11];
	result[12]=matrix1[0]*matrix2[12]+
		matrix1[4]*matrix2[13]+
		matrix1[8]*matrix2[14]+
		matrix1[12]*matrix2[15];
	result[1]=matrix1[1]*matrix2[0]+
		matrix1[5]*matrix2[1]+
		matrix1[9]*matrix2[2]+
		matrix1[13]*matrix2[3];
	result[5]=matrix1[1]*matrix2[4]+
		matrix1[5]*matrix2[5]+
		matrix1[9]*matrix2[6]+
		matrix1[13]*matrix2[7];
	result[9]=matrix1[1]*matrix2[8]+
		matrix1[5]*matrix2[9]+
		matrix1[9]*matrix2[10]+
		matrix1[13]*matrix2[11];
	result[13]=matrix1[1]*matrix2[12]+
		matrix1[5]*matrix2[13]+
		matrix1[9]*matrix2[14]+
		matrix1[13]*matrix2[15];
	result[2]=matrix1[2]*matrix2[0]+
		matrix1[6]*matrix2[1]+
		matrix1[10]*matrix2[2]+
		matrix1[14]*matrix2[3];
	result[6]=matrix1[2]*matrix2[4]+
		matrix1[6]*matrix2[5]+
		matrix1[10]*matrix2[6]+
		matrix1[14]*matrix2[7];
	result[10]=matrix1[2]*matrix2[8]+
		matrix1[6]*matrix2[9]+
		matrix1[10]*matrix2[10]+
		matrix1[14]*matrix2[11];
	result[14]=matrix1[2]*matrix2[12]+
		matrix1[6]*matrix2[13]+
		matrix1[10]*matrix2[14]+
		matrix1[14]*matrix2[15];
	result[3]=matrix1[3]*matrix2[0]+
		matrix1[7]*matrix2[1]+
		matrix1[11]*matrix2[2]+
		matrix1[15]*matrix2[3];
	result[7]=matrix1[3]*matrix2[4]+
		matrix1[7]*matrix2[5]+
		matrix1[11]*matrix2[6]+
		matrix1[15]*matrix2[7];
	result[11]=matrix1[3]*matrix2[8]+
		matrix1[7]*matrix2[9]+
		matrix1[11]*matrix2[10]+
		matrix1[15]*matrix2[11];
	result[15]=matrix1[3]*matrix2[12]+
		matrix1[7]*matrix2[13]+
		matrix1[11]*matrix2[14]+
		matrix1[15]*matrix2[15];
}

void MultiplyMatrixByVector4by4OpenGL_FLOAT(float *resultvector, const float *matrix, const float *pvector)
{
	resultvector[0]=matrix[0]*pvector[0]+matrix[4]*pvector[1]+matrix[8]*pvector[2]+matrix[12]*pvector[3];
	resultvector[1]=matrix[1]*pvector[0]+matrix[5]*pvector[1]+matrix[9]*pvector[2]+matrix[13]*pvector[3];
	resultvector[2]=matrix[2]*pvector[0]+matrix[6]*pvector[1]+matrix[10]*pvector[2]+matrix[14]*pvector[3];
	resultvector[3]=matrix[3]*pvector[0]+matrix[7]*pvector[1]+matrix[11]*pvector[2]+matrix[15]*pvector[3];
}

#define SWAP_ROWS_DOUBLE(a, b) { double *_tmp = a; (a)=(b); (b)=_tmp; }
#define SWAP_ROWS_FLOAT(a, b) { float *_tmp = a; (a)=(b); (b)=_tmp; }
#define MAT(m,r,c) (m)[(c)*4+(r)]

//This code comes directly from GLU except that it is for float
int glhInvertMatrixf2(float *m, float *out)
{
	float wtmp[4][8];
	float m0, m1, m2, m3, s;
	float *r0, *r1, *r2, *r3;
	r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];
	r0[0] = MAT(m, 0, 0), r0[1] = MAT(m, 0, 1),
		r0[2] = MAT(m, 0, 2), r0[3] = MAT(m, 0, 3),
		r0[4] = 1.0f, r0[5] = r0[6] = r0[7] = 0.0f,
		r1[0] = MAT(m, 1, 0), r1[1] = MAT(m, 1, 1),
		r1[2] = MAT(m, 1, 2), r1[3] = MAT(m, 1, 3),
		r1[5] = 1.0f, r1[4] = r1[6] = r1[7] = 0.0f,
		r2[0] = MAT(m, 2, 0), r2[1] = MAT(m, 2, 1),
		r2[2] = MAT(m, 2, 2), r2[3] = MAT(m, 2, 3),
		r2[6] = 1.0f, r2[4] = r2[5] = r2[7] = 0.0f,
		r3[0] = MAT(m, 3, 0), r3[1] = MAT(m, 3, 1),
		r3[2] = MAT(m, 3, 2), r3[3] = MAT(m, 3, 3),
		r3[7] = 1.0f, r3[4] = r3[5] = r3[6] = 0.0f;
	/* choose pivot - or die */
	if (fabsf(r3[0]) > fabsf(r2[0]))
		SWAP_ROWS_FLOAT(r3, r2);
	if (fabsf(r2[0]) > fabsf(r1[0]))
		SWAP_ROWS_FLOAT(r2, r1);
	if (fabsf(r1[0]) > fabsf(r0[0]))
		SWAP_ROWS_FLOAT(r1, r0);
	if (0.0f == r0[0])
		return 0;
	/* eliminate first variable     */
	m1 = r1[0] / r0[0];
	m2 = r2[0] / r0[0];
	m3 = r3[0] / r0[0];
	s = r0[1];
	r1[1] -= m1 * s;
	r2[1] -= m2 * s;
	r3[1] -= m3 * s;
	s = r0[2];
	r1[2] -= m1 * s;
	r2[2] -= m2 * s;
	r3[2] -= m3 * s;
	s = r0[3];
	r1[3] -= m1 * s;
	r2[3] -= m2 * s;
	r3[3] -= m3 * s;
	s = r0[4];
	if (s != 0.0f) {
		r1[4] -= m1 * s;
		r2[4] -= m2 * s;
		r3[4] -= m3 * s;
	}
	s = r0[5];
	if (s != 0.0f) {
		r1[5] -= m1 * s;
		r2[5] -= m2 * s;
		r3[5] -= m3 * s;
	}
	s = r0[6];
	if (s != 0.0f) {
		r1[6] -= m1 * s;
		r2[6] -= m2 * s;
		r3[6] -= m3 * s;
	}
	s = r0[7];
	if (s != 0.0f) {
		r1[7] -= m1 * s;
		r2[7] -= m2 * s;
		r3[7] -= m3 * s;
	}
	/* choose pivot - or die */
	if (fabsf(r3[1]) > fabsf(r2[1]))
		SWAP_ROWS_FLOAT(r3, r2);
	if (fabsf(r2[1]) > fabsf(r1[1]))
		SWAP_ROWS_FLOAT(r2, r1);
	if (0.0f == r1[1])
		return 0;
	/* eliminate second variable */
	m2 = r2[1] / r1[1];
	m3 = r3[1] / r1[1];
	r2[2] -= m2 * r1[2];
	r3[2] -= m3 * r1[2];
	r2[3] -= m2 * r1[3];
	r3[3] -= m3 * r1[3];
	s = r1[4];
	if (0.0f != s) {
		r2[4] -= m2 * s;
		r3[4] -= m3 * s;
	}
	s = r1[5];
	if (0.0f != s) {
		r2[5] -= m2 * s;
		r3[5] -= m3 * s;
	}
	s = r1[6];
	if (0.0f != s) {
		r2[6] -= m2 * s;
		r3[6] -= m3 * s;
	}
	s = r1[7];
	if (0.0f != s) {
		r2[7] -= m2 * s;
		r3[7] -= m3 * s;
	}
	/* choose pivot - or die */
	if (fabsf(r3[2]) > fabsf(r2[2]))
		SWAP_ROWS_FLOAT(r3, r2);
	if (0.0f == r2[2])
		return 0;
	/* eliminate third variable */
	m3 = r3[2] / r2[2];
	r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
		r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6], r3[7] -= m3 * r2[7];
	/* last check */
	if (0.0f == r3[3])
		return 0;
	s = 1.0f / r3[3];		/* now back substitute row 3 */
	r3[4] *= s;
	r3[5] *= s;
	r3[6] *= s;
	r3[7] *= s;
	m2 = r2[3];			/* now back substitute row 2 */
	s = 1.0f / r2[2];
	r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
		r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
	m1 = r1[3];
	r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
		r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
	m0 = r0[3];
	r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
		r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;
	m1 = r1[2];			/* now back substitute row 1 */
	s = 1.0f / r1[1];
	r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
		r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
	m0 = r0[2];
	r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
		r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;
	m0 = r0[1];			/* now back substitute row 0 */
	s = 1.0f / r0[0];
	r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
		r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);
	MAT(out, 0, 0) = r0[4];
	MAT(out, 0, 1) = r0[5], MAT(out, 0, 2) = r0[6];
	MAT(out, 0, 3) = r0[7], MAT(out, 1, 0) = r1[4];
	MAT(out, 1, 1) = r1[5], MAT(out, 1, 2) = r1[6];
	MAT(out, 1, 3) = r1[7], MAT(out, 2, 0) = r2[4];
	MAT(out, 2, 1) = r2[5], MAT(out, 2, 2) = r2[6];
	MAT(out, 2, 3) = r2[7], MAT(out, 3, 0) = r3[4];
	MAT(out, 3, 1) = r3[5], MAT(out, 3, 2) = r3[6];
	MAT(out, 3, 3) = r3[7];
	return 1;
}

int glhUnProjectf(	float winx, float winy, float winz,
					float *modelview, float *projection,
					int *viewport, float *objectCoordinate)
{
	//Transformation matrices
	float m[16], A[16];
	float in[4], out[4];
	MultiplyMatrices4by4OpenGL_FLOAT(A, projection, modelview);
	//Now compute the inverse of matrix A
	if(glhInvertMatrixf2(A, m)==0)
		return 0;
	//Transformation of normalized coordinates between -1 and 1
	in[0]=(winx-(float)viewport[0])/(float)viewport[2]*2.0f-1.0f;
	in[1]=(winy-(float)viewport[1])/(float)viewport[3]*2.0f-1.0f;
	in[2]=2.0f*winz-1.0f;
	in[3]=1.0f;
	//Objects coordinates
	MultiplyMatrixByVector4by4OpenGL_FLOAT(out, m, in);
	if(out[3]==0.0f)
		return 0;
	out[3]=1.0f/out[3];
	objectCoordinate[0]=out[0]*out[3];
	objectCoordinate[1]=out[1]*out[3];
	objectCoordinate[2]=out[2]*out[3];
	return 1;
}

#undef glTranslatef
#undef glRotatef
#undef glScalef
#undef glPushMatrix
#undef glPopMatrix
#undef glLoadIdentity
#undef glMultMatrixf
#undef glMatrixMode

void mc_glTranslatef(float x, float y, float z) {
    if (currentStack) currentStack->translate(x, y, z);
}
void mc_glRotatef(float angle, float x, float y, float z) {
    if (currentStack) currentStack->rotate(angle, x, y, z);
}
void mc_glScalef(float x, float y, float z) {
    if (currentStack) currentStack->scale(x, y, z);
}
void mc_glPushMatrix() {
    if (currentStack) currentStack->push();
}
void mc_glPopMatrix() {
    if (currentStack) currentStack->pop();
}
void mc_glLoadIdentity() {
    if (currentStack) currentStack->identity();
}
void mc_glMultMatrixf(const GLfloat* m) {
    if (currentStack) currentStack->multMatrix(m);
}
void mc_glMatrixMode(GLenum mode) {
    if (mode == GL_PROJECTION) {
        currentStack = &projectionStack;
    } else if (mode == GL_MODELVIEW) {
        currentStack = &modelViewStack;
    }
}

void mc_glShadeModel(GLenum mode) {
    renderState.shadeModel = mode;
}

void mc_glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar) {
    if (currentStack) currentStack->ortho(left, right, bottom, top, zNear, zFar);
}

RenderState renderState = { false, 0, 0.0f, 1.0f, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f}, false, 0.1f, GL_SMOOTH, false, {1.0f, 1.0f, 1.0f, 1.0f} };

#undef glFogf
#undef glFogfv
#undef glFogx
#undef glEnable
#undef glDisable
#undef glAlphaFunc

void mc_glFogf(GLenum pname, GLfloat param) {
    if (pname == GL_FOG_START) renderState.fogStart = param;
    else if (pname == GL_FOG_END) renderState.fogEnd = param;
    else if (pname == GL_FOG_DENSITY) renderState.fogDensity = param;
    else if (pname == GL_FOG_MODE) renderState.fogMode = (int)param;
}

void mc_glFogfv(GLenum pname, const GLfloat *params) {
    if (pname == GL_FOG_COLOR) {
        renderState.fogColor[0] = params[0];
        renderState.fogColor[1] = params[1];
        renderState.fogColor[2] = params[2];
        renderState.fogColor[3] = params[3];
    }
}

void mc_glFogx(GLenum pname, GLint param) {
    if (pname == GL_FOG_MODE) renderState.fogMode = param;
}

void mc_glEnable(GLenum cap) {
    if (cap == GL_FOG) renderState.fogEnabled = true;
    else if (cap == GL_ALPHA_TEST) renderState.alphaTestEnabled = true;
    else if (cap == GL_TEXTURE_2D) renderState.texture2DEnabled = true;
    else if (cap == GL_LIGHTING) {} 
    else if (cap == GL_COLOR_MATERIAL) {} 
    else glEnable(cap); 
}

void mc_glDisable(GLenum cap) {
    if (cap == GL_FOG) renderState.fogEnabled = false;
    else if (cap == GL_ALPHA_TEST) renderState.alphaTestEnabled = false;
    else if (cap == GL_TEXTURE_2D) renderState.texture2DEnabled = false;
    else if (cap == GL_LIGHTING) {} 
    else if (cap == GL_COLOR_MATERIAL) {} 
    else glDisable(cap); 
}

void mc_glAlphaFunc(GLenum func, GLclampf ref) {
    renderState.alphaTestRef = ref;
}

#undef glColor4f
#undef glEnableClientState
#undef glDisableClientState
#undef glVertexPointer
#undef glColorPointer
#undef glTexCoordPointer
#undef glNormalPointer
#undef glDrawArrays

static const GLvoid* vPtr = NULL;
static const GLvoid* cPtr = NULL;
static const GLvoid* tPtr = NULL;

static GLint vSize = 3, cSize = 4, tSize = 2;
static GLenum vType = GL_FLOAT, cType = GL_UNSIGNED_BYTE, tType = GL_FLOAT;
static GLsizei vStride = 0, cStride = 0, tStride = 0;

static bool vEnabled = false;
static bool cEnabled = false;
static bool tEnabled = false;

void mc_glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
    renderState.color[0] = r;
    renderState.color[1] = g;
    renderState.color[2] = b;
    renderState.color[3] = a;
}

void mc_glEnableClientState(GLenum array) {
    if (array == GL_VERTEX_ARRAY) vEnabled = true;
    else if (array == GL_COLOR_ARRAY) cEnabled = true;
    else if (array == GL_TEXTURE_COORD_ARRAY) tEnabled = true;
}

void mc_glDisableClientState(GLenum array) {
    if (array == GL_VERTEX_ARRAY) vEnabled = false;
    else if (array == GL_COLOR_ARRAY) cEnabled = false;
    else if (array == GL_TEXTURE_COORD_ARRAY) tEnabled = false;
}

void mc_glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {
    vSize = size; vType = type; vStride = stride; vPtr = pointer;
}

void mc_glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {
    cSize = size; cType = type; cStride = stride; cPtr = pointer;
}

void mc_glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {
    tSize = size; tType = type; tStride = stride; tPtr = pointer;
}

void mc_glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer) {}

#undef glClear
#undef glClearColor
#undef glViewport

void mc_glClear(GLbitfield mask) {
    ensureShaders();
    glClear(mask);
}
void mc_glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
    glClearColor(red, green, blue, alpha);
}
void mc_glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
    ensureShaders();
    glViewport(x, y, width, height);
}

void mc_glDrawArrays(GLenum mode, GLint first, GLsizei count) {
    ensureShaders();
    static int drawCount = 0;
    if (drawCount < 10) {
    }

    if (currentShader) {
        currentShader->setUniformMatrix4("u_modelView", currentStack->getTop().m);
        currentShader->setUniformMatrix4("u_projection", projectionStack.getTop().m);
        
        currentShader->setUniform1i("u_useTexture", renderState.texture2DEnabled ? 1 : 0);
        currentShader->setUniform1i("u_alphaTest", renderState.alphaTestEnabled ? 1 : 0);
        currentShader->setUniform4f("u_color", renderState.color[0], renderState.color[1], renderState.color[2], renderState.color[3]);
        
        currentShader->setUniform1i("u_texture", 0);
        
        // Fog uniforms
        currentShader->setUniform1i("u_fogEnabled", renderState.fogEnabled ? 1 : 0);
        currentShader->setUniform4f("u_fogColor", renderState.fogColor[0], renderState.fogColor[1], renderState.fogColor[2], renderState.fogColor[3]);
        currentShader->setUniform1f("u_fogStart", renderState.fogStart);
        currentShader->setUniform1f("u_fogEnd", renderState.fogEnd);
        currentShader->setUniform1f("u_fogDensity", renderState.fogDensity);
        currentShader->setUniform1i("u_fogMode", renderState.fogMode);
        
        GLint posLoc = currentShader->getAttribLocation("a_position");
        GLint texLoc = currentShader->getAttribLocation("a_texCoord");
        GLint colLoc = currentShader->getAttribLocation("a_color");
        
        bool forceEnable = renderState.texture2DEnabled && currentShader;
        if (forceEnable) {
            vEnabled = true;
            tEnabled = true; 
            cEnabled = true;
            
            vSize = 3; vType = GL_FLOAT; vStride = 24; vPtr = 0;
            tSize = 2; tType = GL_FLOAT; tStride = 24; tPtr = (void*)12;
            cSize = 4; cType = GL_UNSIGNED_BYTE; cStride = 24; cPtr = (void*)20;
            
        }
        
        if (vEnabled && posLoc != -1 && p_glEnableVertexAttribArray && p_glVertexAttribPointer) {
            p_glEnableVertexAttribArray(posLoc);
            p_glVertexAttribPointer(posLoc, vSize, vType, GL_FALSE, vStride, vPtr);
        }
        if (tEnabled && texLoc != -1 && p_glEnableVertexAttribArray && p_glVertexAttribPointer) {
            p_glEnableVertexAttribArray(texLoc);
            p_glVertexAttribPointer(texLoc, tSize, tType, GL_FALSE, tStride, tPtr);
        } else if (texLoc != -1 && p_glDisableVertexAttribArray) {
            p_glDisableVertexAttribArray(texLoc);
        }

        if (cEnabled && colLoc != -1 && p_glEnableVertexAttribArray && p_glVertexAttribPointer) {
            p_glEnableVertexAttribArray(colLoc);
            p_glVertexAttribPointer(colLoc, cSize, cType, cType == GL_UNSIGNED_BYTE ? GL_TRUE : GL_FALSE, vStride, cPtr);
        } else if (colLoc != -1) {
            if (p_glDisableVertexAttribArray) p_glDisableVertexAttribArray(colLoc);
            if (p_glVertexAttrib4f) p_glVertexAttrib4f(colLoc, 1.0f, 1.0f, 1.0f, 1.0f);
        }
        
        glDrawArrays(mode, first, count);
        
        if (vEnabled && posLoc != -1 && p_glDisableVertexAttribArray) p_glDisableVertexAttribArray(posLoc);
        if (tEnabled && texLoc != -1 && p_glDisableVertexAttribArray) p_glDisableVertexAttribArray(texLoc);
        if (cEnabled && colLoc != -1 && p_glDisableVertexAttribArray) p_glDisableVertexAttribArray(colLoc);
    } else {
        glDrawArrays(mode, first, count);
    }

    GLenum err = glGetError();
    if (err != GL_NO_ERROR && drawCount < 50) {
        LOGE("DIAGNOSTIC: GLES Error after draw call: 0x%x\n", err);
    }
}

void mc_glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz) {
}

#undef glHint
void mc_glHint(GLenum target, GLenum mode) {
}

#undef glGetFloatv
void mc_glGetFloatv(GLenum pname, GLfloat *params) {
    if (pname == GL_MODELVIEW_MATRIX) {
        for (int i = 0; i < 16; i++) params[i] = modelViewStack.getTop().m[i];
    } else if (pname == GL_PROJECTION_MATRIX) {
        for (int i = 0; i < 16; i++) params[i] = projectionStack.getTop().m[i];
    } else {
        glGetFloatv(pname, params);
    }
}
