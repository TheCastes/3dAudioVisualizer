#pragma once

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:
    Shader(const GLchar* vertexPath, const GLchar* fragmentPath) {
        // Step 1: we retrieve shaders source code from provided filepaths
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;

        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        try {
            // Open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            // Read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            // Convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        } catch (std::ifstream::failure const&) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ\n";
        }

        // Convert strings to char pointers
        const GLchar* vShaderCode = vertexCode.c_str();
        const GLchar* fShaderCode = fragmentCode.c_str();
        // Step 2: we compile the shaders

        // Vertex Shader
        const GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        // check compilation errors
        checkCompileErrors(vertex, "VERTEX");
        // Fragment Shader
        const GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        // check compilation errors
        checkCompileErrors(fragment, "FRAGMENT");

        // Step 3: Shader Program creation
        this->program = glCreateProgram();
        glAttachShader(this->program, vertex);
        glAttachShader(this->program, fragment);
        glLinkProgram(this->program);
        // check linking errors
        checkCompileErrors(this->program, "PROGRAM");

        // Step 4: we delete the shaders because they are linked to the Shader Program, and we do not need them anymore
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }


    ~Shader() {
        glDeleteProgram(this->program);
    }

    Shader(const Shader& copy) = delete;
    Shader& operator=(const Shader&) = delete;

    void use() {
        glUseProgram(this->program);
    }

    void set(const std::string& name, int value) {
        glUniform1i(location(name), value);
    }
    void set(const std::string& name, float value) {
        glUniform1f(location(name), value);
    }
    void set(const std::string& name, const glm::mat4& value) {
        glUniformMatrix4fv(location(name), 1, GL_FALSE, glm::value_ptr(value));
    }
    void set(const std::string& name, const float* values, int count) {
        glUniform1fv(location(name), count, values);
    }
    void set(const std::string& name, const glm::vec3* values, int count) {
        glUniform3fv(location(name), count, glm::value_ptr(values[0]));
    }

private:
    GLuint program = 0;

    GLint location(const std::string& name) {
        return glGetUniformLocation(program, name.c_str());
    }



    // Check compilation and linking errors
    void checkCompileErrors(GLuint shader, std::string type) {
        GLint success;
        GLchar infoLog[1024];
        if(type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if(!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "| ERROR::::SHADER-COMPILATION-ERROR of type: " << type << "|\n" << infoLog << "\n| -- --------------------------------------------------- -- |\n";
            }
        }
        else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if(!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "| ERROR::::PROGRAM-LINKING-ERROR of type: " << type << "|\n" << infoLog << "\n| -- --------------------------------------------------- -- |\n";
            }
        }
    }
};
