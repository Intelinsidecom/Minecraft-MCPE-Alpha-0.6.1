#include "RenderList.h"

#include "gles.h"
#include "RenderChunk.h"
#include "Tesselator.h"
#include "Shader.h"



RenderList::RenderList()
	:	inited(false),
	rendered(false)
{
	lists = new int[MAX_NUM_OBJECTS];
	rlists = new RenderChunk[MAX_NUM_OBJECTS];

	for (int i = 0; i < MAX_NUM_OBJECTS; ++i)
		rlists[i].vboId = -1;
}

RenderList::~RenderList() {
	delete[] lists;
	delete[] rlists;
}

void RenderList::init(float xOff, float yOff, float zOff) {
	inited = true;
	listIndex = 0;

	this->xOff = (float) xOff;
	this->yOff = (float) yOff;
	this->zOff = (float) zOff;
}

void RenderList::add(int list) {
	lists[listIndex] = list;
	if (listIndex == MAX_NUM_OBJECTS) /*lists.remaining() == 0)*/ render();
}

void RenderList::addR(const RenderChunk& chunk) {
	rlists[listIndex] = chunk;
}

void RenderList::render() {

	if (!inited) return;
	if (!rendered) {
		bufferLimit = listIndex;
		listIndex = 0;
		rendered = true;
	}
	if (listIndex < bufferLimit) {
		glPushMatrix2();
		glTranslatef2(-xOff, -yOff, -zOff);

		#ifndef USE_VBO
			glCallLists(bufferLimit, GL_UNSIGNED_INT, lists);
		#else
			renderChunks();
		#endif/*!USE_VBO*/

		glPopMatrix2();
	}
}

void RenderList::renderChunks() {
	//glDisableClientState2(GL_NORMAL_ARRAY);
	glEnableClientState2(GL_VERTEX_ARRAY);
	glEnableClientState2(GL_COLOR_ARRAY);
	glEnableClientState2(GL_TEXTURE_COORD_ARRAY);

	const int Stride = VertexSizeBytes;

	// Set fog uniforms once before rendering all chunks
	if (currentShader) {
		currentShader->setUniform1i("u_fogEnabled", renderState.fogEnabled ? 1 : 0);
		currentShader->setUniform4f("u_fogColor", renderState.fogColor[0], renderState.fogColor[1], renderState.fogColor[2], renderState.fogColor[3]);
		currentShader->setUniform1f("u_fogStart", renderState.fogStart);
		currentShader->setUniform1f("u_fogEnd", renderState.fogEnd);
		currentShader->setUniform1f("u_fogDensity", renderState.fogDensity);
		currentShader->setUniform1i("u_fogMode", renderState.fogMode);
	}

	for (int i = 0; i < bufferLimit; ++i) {
		RenderChunk& rc = rlists[i];

		glPushMatrix2();
		glTranslatef2(rc.pos.x, rc.pos.y, rc.pos.z);
		glBindBuffer2(GL_ARRAY_BUFFER, rc.vboId);

		glVertexPointer2	(3, GL_FLOAT, Stride,  0);
		glTexCoordPointer2	(2, GL_FLOAT, Stride, (GLvoid*) (3 * 4));
		glColorPointer2		(4, GL_UNSIGNED_BYTE, Stride, (GLvoid*) (5 * 4));

		glDrawArrays2(GL_TRIANGLES, 0, rc.vertexCount);

		glPopMatrix2();
	}

	glDisableClientState2(GL_VERTEX_ARRAY);
	glDisableClientState2(GL_COLOR_ARRAY);
	glDisableClientState2(GL_TEXTURE_COORD_ARRAY);
}

void RenderList::clear() {
	inited = false;
	rendered = false;
}
