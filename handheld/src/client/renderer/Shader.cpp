#include "Shader.h"
#include "GLESLoader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "../../platform/log.h"

Shader* currentShader = nullptr;

GLuint Shader::compileShader(GLenum type, const std::string& source) {
    LOGI("DIAGNOSTIC: Compiling %s shader...\n", type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");

    if (!p_glCreateShader) {
        LOGE("DIAGNOSTIC: p_glCreateShader is NULL! Loader might have failed.\n");
        return 0;
    }

    GLuint shader = p_glCreateShader(type);
    GLenum err = glGetError();
    LOGI("DIAGNOSTIC: p_glCreateShader returned %u, error: 0x%x\n", shader, err);

    if (shader == 0) return 0;

    const char* src = source.c_str();
    p_glShaderSource(shader, 1, &src, nullptr);
    p_glCompileShader(shader);
    err = glGetError();
    LOGI("DIAGNOSTIC: p_glCompileShader called, error: 0x%x\n", err);

    GLint success;
    p_glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        p_glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        LOGE("DIAGNOSTIC: Shader compilation failed! Log:\n%s\n", infoLog);
    } else {
        LOGI("DIAGNOSTIC: Shader compiled successfully.\n");
    }
    return shader;
}

Shader::Shader(const std::string& vertexFile, const std::string& fragmentFile) {
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile(vertexFile);
    std::ifstream fShaderFile(fragmentFile);

    if (vShaderFile.is_open()) {
        std::stringstream vShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        vertexCode = vShaderStream.str();
    } else {
        LOGE("Failed to open vertex shader file: %s", vertexFile.c_str());
    }

    if (fShaderFile.is_open()) {
        std::stringstream fShaderStream;
        fShaderStream << fShaderFile.rdbuf();
        fragmentCode = fShaderStream.str();
    } else {
        LOGE("Failed to open fragment shader file: %s", fragmentFile.c_str());
    }

    GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexCode);
    GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentCode);

    if (!p_glCreateProgram) {
        LOGE("DIAGNOSTIC: p_glCreateProgram is NULL!\n");
        programId = 0;
        return;
    }

    programId = p_glCreateProgram();
    GLenum err = glGetError();
    LOGI("DIAGNOSTIC: p_glCreateProgram returned %u, error: 0x%x\n", programId, err);

    if (programId != 0) {
        p_glAttachShader(programId, vertex);
        p_glAttachShader(programId, fragment);
        p_glLinkProgram(programId);
        err = glGetError();
        LOGI("DIAGNOSTIC: p_glLinkProgram err: 0x%x\n", err);

        GLint success;
        p_glGetProgramiv(programId, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            p_glGetProgramInfoLog(programId, 512, nullptr, infoLog);
            LOGE("DIAGNOSTIC: Shader linking failed! Log:\n%s\n", infoLog);
            p_glDeleteProgram(programId);
            programId = 0;
        } else {
            LOGI("DIAGNOSTIC: Shader program linked successfully. ID: %u\n", programId);
        }
    }

    p_glDeleteShader(vertex);
    p_glDeleteShader(fragment);
}

Shader::~Shader() {
    if (programId && p_glDeleteProgram) {
        p_glDeleteProgram(programId);
    }
}

void Shader::bind() {
    if (programId && p_glUseProgram) {
        p_glUseProgram(programId);
        currentShader = this;
    }
}

void Shader::unbind() {
    if (p_glUseProgram) {
        p_glUseProgram(0);
        currentShader = nullptr;
    }
}

GLint Shader::getUniformLocation(const std::string& name) {
    if (programId && p_glGetUniformLocation) {
        return p_glGetUniformLocation(programId, name.c_str());
    }
    return -1;
}

GLint Shader::getAttribLocation(const std::string& name) {
    if (programId && p_glGetAttribLocation) {
        return p_glGetAttribLocation(programId, name.c_str());
    }
    return -1;
}

void Shader::setUniformMatrix4(const std::string& name, const float* matrix) {
    if (!programId || !p_glUniformMatrix4fv) return;
    GLint loc = getUniformLocation(name);
    if (loc != -1) p_glUniformMatrix4fv(loc, 1, GL_FALSE, matrix);
}

void Shader::setUniform1i(const std::string& name, int value) {
    if (!programId || !p_glUniform1i) return;
    GLint loc = getUniformLocation(name);
    if (loc != -1) p_glUniform1i(loc, value);
}

void Shader::setUniform1f(const std::string& name, float value) {
    if (!programId || !p_glUniform1f) return;
    GLint loc = getUniformLocation(name);
    if (loc != -1) p_glUniform1f(loc, value);
}

void Shader::setUniform4f(const std::string& name, float x, float y, float z, float w) {
    if (!programId || !p_glUniform4f) return;
    GLint loc = getUniformLocation(name);
    if (loc != -1) p_glUniform4f(loc, x, y, z, w);
}
