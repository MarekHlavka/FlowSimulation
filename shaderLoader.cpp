#include "shaderLoader.hpp"
#include <GL/gl.h>
#include <GL/glut.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string.h>

#include <vector>

#define SUCCESS   0
#define ERROR     1

#define SHADER_FOLDER "../shaders/"

/// @brief Checking if shader is compiled correctly
/// @param shader shader object
/// @return 0 if OK, 1 otherwise
uint checkShaderCompilation(GLuint shader){
  GLint compileSuccess = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compileSuccess);

  if(compileSuccess == GL_FALSE)
  {
    GLint maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

    // The maxLength includes the NULL character
    std::vector<GLchar> errorLog(maxLength);
    glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

    // Provide the infolog in whatever manor you deem best.
    for(GLchar c : errorLog){
      std::cout << c;
    }
    std::cout << std::endl;
    // Exit with failure.
    glDeleteShader(shader); // Don't leak the shader.
    return ERROR;
  }
  return SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Loadign shader file from source and compile it
/// @param filename path to the shader file
/// @param type     shader type (compute, vertex, fragment)
/// @return GLuint compiled shader or -1 as error
GLuint loadAndCompileShader(const char* filename, uint16_t type){
  std::string shaderStr;
  std::ifstream shader_file;
  char filepath[256] = SHADER_FOLDER;
  strcat(filepath, filename);
  std::cout << filepath << std::endl;
  shader_file.open(filepath);
  if(shader_file.is_open()){
    while(shader_file){
      shaderStr += shader_file.get();
    }
  }
  else{
    std::cerr << "Compute shader not openned!" << std::endl;
  }
  shader_file.close();
  
  //////////// Compiling compute shader
  GLuint shader = glCreateShader(type);
  const char * source = shaderStr.c_str();
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  if(checkShaderCompilation(shader) == ERROR){
    return -1;
  }  
  return shader;
}