#include "Renderer.h"
#include <stdio.h>

namespace coda {

bool Renderer::Initialize(int width, int height) {
    m_Width = width;
    m_Height = height;

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Set viewport
    glViewport(0, 0, width, height);

    // Create vertex buffer
    glGenBuffers(1, &m_VertexBuffer);

    return true;
}

void Renderer::Cleanup() {
    if (m_VertexBuffer) {
        glDeleteBuffers(1, &m_VertexBuffer);
    }
    if (m_ShaderProgram) {
        glDeleteProgram(m_ShaderProgram);
    }
}

void Renderer::SetViewport(int width, int height) {
    m_Width = width;
    m_Height = height;
    glViewport(0, 0, width, height);
}

void Renderer::SetClearColor(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
}

void Renderer::Clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

GLuint Renderer::CreateShaderProgram(const char* vertexSource, const char* fragmentSource) {
    // Delete existing program if any
    DeleteShaderProgram();
    
    // Create and compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);
    if (!CompileShader(vertexShader)) {
        glDeleteShader(vertexShader);
        return 0;
    }
    
    // Create and compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);
    if (!CompileShader(fragmentShader)) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }
    
    // Create program and attach shaders
    m_ShaderProgram = glCreateProgram();
    glAttachShader(m_ShaderProgram, vertexShader);
    glAttachShader(m_ShaderProgram, fragmentShader);
    
    // Link program
    glLinkProgram(m_ShaderProgram);
    if (!LinkProgram(m_ShaderProgram)) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(m_ShaderProgram);
        m_ShaderProgram = 0;
        return 0;
    }
    
    // Clean up shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return m_ShaderProgram;
}

void Renderer::UseShaderProgram(GLuint program) {
    m_CurrentProgram = program;
    glUseProgram(program);
}

void Renderer::DeleteShaderProgram() {
    if (m_ShaderProgram) {
        glDeleteProgram(m_ShaderProgram);
        m_ShaderProgram = 0;
    }
}

void Renderer::SetMatrix4f(const char* name, const float* matrix) {
    GLint location = glGetUniformLocation(m_CurrentProgram, name);
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, matrix);
    }
}

void Renderer::SetVector4f(const char* name, float x, float y, float z, float w) {
    GLint location = glGetUniformLocation(m_CurrentProgram, name);
    if (location != -1) {
        glUniform4f(location, x, y, z, w);
    }
}

void Renderer::SetFloat(const char* name, float value) {
    GLint location = glGetUniformLocation(m_CurrentProgram, name);
    if (location != -1) {
        glUniform1f(location, value);
    }
}

bool Renderer::CompileShader(GLuint shader) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Shader compilation error: %s\n", infoLog);
        return false;
    }
    return true;
}

bool Renderer::LinkProgram(GLuint program) {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        printf("Program linking error: %s\n", infoLog);
        return false;
    }
    return true;
}

void Renderer::CreateVertexBuffer() {
    if (m_VertexBuffer == 0) {
        glGenBuffers(1, &m_VertexBuffer);
    }
}

void Renderer::DeleteVertexBuffer() {
    if (m_VertexBuffer != 0) {
        glDeleteBuffers(1, &m_VertexBuffer);
        m_VertexBuffer = 0;
    }
}

void Renderer::BindVertexBuffer() {
    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
}

void Renderer::UpdateVertexBuffer(const float* data, size_t size) {
    BindVertexBuffer();
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
}

void Renderer::SetVertexAttribute(int location, int size, int stride, int offset) {
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, size, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offset));
}

void Renderer::DrawArrays(int mode, int first, int count) {
    glDrawArrays(mode, first, count);
}

void Renderer::DisableVertexAttribArray(int location) {
    glDisableVertexAttribArray(location);
}

void Renderer::SetVector3f(const char* name, float x, float y, float z) {
    GLint location = glGetUniformLocation(m_CurrentProgram, name);
    if (location != -1) {
        glUniform3f(location, x, y, z);
    }
}

} // namespace coda 