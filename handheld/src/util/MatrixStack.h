#ifndef MCPE_UTIL_MATRIXSTACK_H
#define MCPE_UTIL_MATRIXSTACK_H

#include "Math.h"
#include <vector>

class MatrixStack {
    std::vector<Matrix4> stack;
public:
    MatrixStack() {
        stack.push_back(Matrix4());
    }

    void push() {
        stack.push_back(stack.back());
    }

    void pop() {
        if (stack.size() > 1) {
            stack.pop_back();
        }
    }

    void identity() {
        stack.back().identity();
    }

    void translate(float x, float y, float z) {
        stack.back().translate(x, y, z);
    }

    void rotate(float angle, float x, float y, float z) {
        stack.back().rotate(angle, x, y, z);
    }

    void scale(float x, float y, float z) {
        stack.back().scale(x, y, z);
    }

    void ortho(float left, float right, float bottom, float top, float zNear, float zFar) {
        stack.back().ortho(left, right, bottom, top, zNear, zFar);
    }

    void multMatrix(const float* m) {
        Matrix4 mat;
        for (int i = 0; i < 16; i++) mat.m[i] = m[i];
        stack.back().multiply(mat);
    }

    Matrix4& getTop() {
        return stack.back();
    }
};

extern MatrixStack modelViewStack;
extern MatrixStack projectionStack;
extern MatrixStack* currentStack;

#endif
