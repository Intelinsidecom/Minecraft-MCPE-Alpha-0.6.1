#ifndef MCPE_RENDERER_SHADER_H
#define MCPE_RENDERER_SHADER_H

#include "gles.h"
#include <string>

class Shader {
    GLuint programId;

    GLuint compileShader(GLenum type, const std::string& source);
public:
    Shader(const std::string& vertexFile, const std::string& fragmentFile);
    Shader(const std::string& vertexSource, const std::string& fragmentSource, bool fromSource);
    ~Shader();

    bool isLoaded() const { return programId != 0; }

    void bind();
    void unbind();

    GLuint getProgram() const { return programId; }

    GLint getUniformLocation(const std::string& name);
    GLint getAttribLocation(const std::string& name);

    void setUniformMatrix4(const std::string& name, const float* matrix);
    void setUniform1i(const std::string& name, int value);
    void setUniform1f(const std::string& name, float value);
    void setUniform4f(const std::string& name, float x, float y, float z, float w);
};

extern Shader* currentShader;

#endif
