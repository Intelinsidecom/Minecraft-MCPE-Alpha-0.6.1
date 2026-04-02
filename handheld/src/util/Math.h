#ifndef MCPE_UTIL_MATH_H
#define MCPE_UTIL_MATH_H

#include <cmath>

class Vector3 {
public:
    float x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vector3 cross(const Vector3& o) const {
        return Vector3(y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x);
    }

    void normalize() {
        float len = std::sqrt(x * x + y * y + z * z);
        if (len > 0.0f) { x /= len; y /= len; z /= len; }
    }
};

class Matrix4 {
public:
    float m[16];

    Matrix4() { identity(); }

    void identity() {
        for (int i = 0; i < 16; i++) m[i] = 0.0f;
        m[0] = 1.0f; m[5] = 1.0f; m[10] = 1.0f; m[15] = 1.0f;
    }

    void translate(float x, float y, float z) {
        m[12] += m[0] * x + m[4] * y + m[8] * z;
        m[13] += m[1] * x + m[5] * y + m[9] * z;
        m[14] += m[2] * x + m[6] * y + m[10] * z;
        m[15] += m[3] * x + m[7] * y + m[11] * z;
    }

    void rotate(float angle, float x, float y, float z) {
        float rad = angle * (3.14159265f / 180.0f);
        float c = std::cos(rad);
        float s = std::sin(rad);

        Vector3 v(x, y, z);
        v.normalize();

        float nc = 1.0f - c;
        float xy = v.x * v.y;
        float yz = v.y * v.z;
        float zx = v.z * v.x;
        float xs = v.x * s;
        float ys = v.y * s;
        float zs = v.z * s;

        Matrix4 rot;
        rot.m[0] = v.x * v.x * nc + c;
        rot.m[1] = xy * nc + zs;
        rot.m[2] = zx * nc - ys;
        rot.m[3] = 0.0f;

        rot.m[4] = xy * nc - zs;
        rot.m[5] = v.y * v.y * nc + c;
        rot.m[6] = yz * nc + xs;
        rot.m[7] = 0.0f;

        rot.m[8] = zx * nc + ys;
        rot.m[9] = yz * nc - xs;
        rot.m[10] = v.z * v.z * nc + c;
        rot.m[11] = 0.0f;

        rot.m[12] = 0.0f;
        rot.m[13] = 0.0f;
        rot.m[14] = 0.0f;
        rot.m[15] = 1.0f;

        multiply(rot);
    }

    void scale(float x, float y, float z) {
        m[0] *= x; m[1] *= x; m[2] *= x; m[3] *= x;
        m[4] *= y; m[5] *= y; m[6] *= y; m[7] *= y;
        m[8] *= z; m[9] *= z; m[10] *= z; m[11] *= z;
    }

    void multiply(const Matrix4& o) {
        Matrix4 res;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                // Column-major order for OpenGL ES 2.0 compatibility
                res.m[j * 4 + i] = m[0 * 4 + i] * o.m[j * 4 + 0] +
                                   m[1 * 4 + i] * o.m[j * 4 + 1] +
                                   m[2 * 4 + i] * o.m[j * 4 + 2] +
                                   m[3 * 4 + i] * o.m[j * 4 + 3];
            }
        }
        for (int i = 0; i < 16; i++) m[i] = res.m[i];
    }

    void ortho(float left, float right, float bottom, float top, float zNear, float zFar) {
        Matrix4 o;
        o.m[0] = 2.0f / (right - left);
        o.m[5] = 2.0f / (top - bottom);
        o.m[10] = -2.0f / (zFar - zNear);
        o.m[12] = -(right + left) / (right - left);
        o.m[13] = -(top + bottom) / (top - bottom);
        o.m[14] = -(zFar + zNear) / (zFar - zNear);
        multiply(o);
    }
};

#endif
