#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"
#include <GL/gl.h>
#include <GL/glut.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <fstream>
#include <stdlib.h>
#ifndef CMAKE_ROOT_DIR
#define CMAKE_ROOT_DIR "."
#endif

#define DUAL false
#define SHADER_FOLDER "../shaders/"

#define SUCCESS   0
#define ERROR     1

#define HALP    std::cout << "HALP " << __LINE__ << std::endl;
#define VERSION std::cout << glGetString(GL_VERSION) << std::endl;

#define VERTEX_LOC  0
#define MODEL_LOC   1
#define COLOR_LOC   2

/// VARIABLES
#define POWER_LOSS        0.95f
#define BORDER_DISTANCE     2.f
#define CAM_DST            -2.f
#define PARTICLE_COUNT        6
#define GRID_X               81
#define GRID_Y               41

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
/// @return GLuint compiled shader
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
  return shader;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void generateVec3DInBounds(float* transforms, int count, float* upBounds, float* downBouds){
  return;
}

void generateVec2DInBounds(float* transforms, int count, float* upBounds, float* downBouds){
  
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void generateCircle(uint radius_points, float* output, float radius){
  float center[] = {0.0f, 0.0f, 0.0f};

  float angle = (2 * M_PI) / radius_points;

  int iteration_verts = 9;

  for(uint i = 0; i < radius_points; i++){
    output[i * iteration_verts] = center[0];
    output[i * iteration_verts + 1] = center[1];
    output[i * iteration_verts + 2] = center[2];

    output[i * iteration_verts + 6] = radius * (float)glm::cos((i) * angle);
    output[i * iteration_verts + 7] = radius * (float)glm::sin((i) * angle);
    output[i * iteration_verts + 8] = 0.0f;

    output[i * iteration_verts + 3] = radius * (float)glm::cos((i + 1) * angle);
    output[i * iteration_verts + 4] = radius * (float)glm::sin((i + 1) * angle);
    output[i * iteration_verts + 5] = 0.0f;
  
  }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void computeDensity(float* gridPos, float* particlePos, float* resValues, int size, int particleCount){

  for(int i = 0; i < GRID_X*GRID_Y; i++){
    int current_index = i*3;
    glm::vec3 current_pos = glm::vec3(gridPos[current_index],gridPos[current_index+1]/2,gridPos[current_index+2]);
    int total_dnst = 0;

    for(int j = 0; j < particleCount*3; j+=3){
        glm::vec3 current_particle = glm::vec3(particlePos[j],particlePos[j+1]/2,particlePos[j+2]);
        total_dnst += glm::distance(current_pos, current_particle);
    }
    resValues[i] = glm::min(total_dnst/(float)10, 1.f);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////// MAIN ////////////////////////////////
int main(int argc, char **argv){

  ////////////////////////////// INIT ///////////////////////////
  // Program inicialization
  SDL_Window*window;
  SDL_GLContext context;
  uint32_t width = 1440;
  uint32_t height = 720;
  if(DUAL){
    window = SDL_CreateWindow("PGPa2023",1920,0,width,height,SDL_WINDOW_OPENGL);
  }
  else{
    window = SDL_CreateWindow("PGPa2023",0,0,width,height,SDL_WINDOW_OPENGL);
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,6);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);
  context = SDL_GL_CreateContext(window);

  bool running = true;

  ////// ImGui setup
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();

  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

  ImGui_ImplSDL2_InitForOpenGL(window, context);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
  ImGui_ImplOpenGL3_Init();

  // GLEW init
  GLenum err = glewInit();
  if (err != GLEW_OK)
    exit(1); // or handle the error in a nicer way
  if (!GLEW_VERSION_2_1)  // check that the machine supports the 2.1 API.
    exit(1);

  // random init
  srand(time(NULL));
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  //////////////////////////////////// COMPUTE SHADER TEST /////////////////

  ////////////// Input data
  uint32_t test_input_data[512];
  std::fill_n(test_input_data, 512, 1);

  ///////////// Load shader from file and compile it
  GLuint computeShader = loadAndCompileShader("computeShader.shader", GL_COMPUTE_SHADER);
  HALP
  if(checkShaderCompilation(computeShader) == ERROR){
    return -1;
  }
  HALP
  //////////// Create program
  GLuint computeProgram = glCreateProgram();
  glAttachShader(computeProgram, computeShader);
  glLinkProgram(computeProgram);


  //////////// Use program
  glUseProgram(computeProgram);

  //////////// Creating Shader storage buffer object
  GLuint ssbo = 0;
  glGenBuffers(1, &ssbo);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(test_input_data), &test_input_data, GL_STATIC_DRAW);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);


  ///////////// Add uniform variable
  uint wow = 2;
  GLuint test_location = glGetUniformLocation(computeProgram, "wow");
  glUniform1ui(test_location, wow);


  /////////// Running shader program
  std::cout << "Running Shader Program" << std::endl;
  glDispatchCompute(512,1,1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

  //////////// Return data from GPU
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
  uint32_t* ptr = (uint32_t*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
  for(int i = 0; i < 512; i++){
    /// std::cout << ptr[i] << std::endl;
    continue;
  }
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

  for(uint32_t line : test_input_data){
    //std::cout << line << std::endl;
    continue;
  }
  std::cout << "Done" << std::endl;
  //////////////////////////////////// COMPUTE SHADER TEST  END /////////////////



  GLuint drawProgram = glCreateProgram();
  GLuint borderProgram = glCreateProgram();
  GLuint gridProgram = glCreateProgram();


  /////////////////////////////// DRAWING SHADERS ///////////////////////////
  GLuint particleVertexShader = loadAndCompileShader("particleVertexShader.shader", GL_VERTEX_SHADER);
  if(checkShaderCompilation(particleVertexShader) == ERROR){
    return ERROR;
  }

  GLuint particleFragmentShader = loadAndCompileShader("particleFragmentShader.shader", GL_FRAGMENT_SHADER);
  if(checkShaderCompilation(particleFragmentShader) == ERROR){
    return ERROR;
  }

  GLuint borderVS = loadAndCompileShader("borderVertexShader.shader", GL_VERTEX_SHADER);
  if(checkShaderCompilation(borderVS) == ERROR){
    return ERROR;
  }

  GLuint borderFS = loadAndCompileShader("borderFragmentShader.shader", GL_FRAGMENT_SHADER);
  if(checkShaderCompilation(borderFS) == ERROR){
    return ERROR;
  }

  GLuint gridVS = loadAndCompileShader("gridVertexShader.shader", GL_VERTEX_SHADER);
  if(checkShaderCompilation(gridVS) == ERROR){
    return ERROR;
  }

  GLuint gridFS = loadAndCompileShader("gridFragmentShader.shader", GL_FRAGMENT_SHADER);
  if(checkShaderCompilation(gridFS) == ERROR){
    return ERROR;
  }

  //////////////////////////////////////////////////////////////////  Background
  ///////// Create triangle

  float borders[] = {
    2.f * -BORDER_DISTANCE, -BORDER_DISTANCE, 0.f,
    2.f *  BORDER_DISTANCE, -BORDER_DISTANCE, 0.f,
    2.f * -BORDER_DISTANCE,  BORDER_DISTANCE, 0.f,
    2.f *  BORDER_DISTANCE,  BORDER_DISTANCE, 0.f,
  }; 

  GLuint border_vbo;
  glGenBuffers(1, &border_vbo);

  GLuint border_vao;
  glGenVertexArrays(1, &border_vao);
  glBindVertexArray(border_vao);

  glBindBuffer(GL_ARRAY_BUFFER, border_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(borders), borders, GL_STATIC_DRAW);
  glVertexAttribPointer(VERTEX_LOC, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
  glEnableVertexAttribArray(VERTEX_LOC);

  glAttachShader(borderProgram, borderVS);
  glAttachShader(borderProgram, borderFS);
  glLinkProgram(borderProgram);

  //// Start test input data
  float radius = .1f;
  float allowed_distance = (BORDER_DISTANCE - radius);
  u_int radius_points = 20;
  float idk[radius_points * 3 * 3];
  generateCircle(radius_points, idk, radius);

  int velocity = 1;

  glm::vec3 gravity = glm::vec3(0.f, -0.00098f, 0.f);

  ////////////////////////////////////////////////////////////////// VAO OF CELLS

  GLuint vbo;
  glGenBuffers(1, &vbo);

  GLuint vao;
  glGenVertexArrays(1, &vao);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(idk), idk, GL_STATIC_DRAW);

  glVertexAttribPointer(VERTEX_LOC, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(VERTEX_LOC);


  glAttachShader(drawProgram, particleVertexShader);
  glAttachShader(drawProgram, particleFragmentShader);
  glLinkProgram(drawProgram);


  /////////////////////////////////////////////////////////////////////////
  int gridWidth = GRID_X;
  int gridWidthHalf = glm::floor(gridWidth/2.f);
  int gridHeight = GRID_Y;
  int gridHeightHalf = glm::floor(gridHeight/2.f);
  float densityGrid[gridWidth*gridHeight*3] = {0.f};
  float gridValues[gridWidth*gridHeight] = {0.f};
  float xDiff = 2*BORDER_DISTANCE/(float)gridWidth;
  float yDiff = BORDER_DISTANCE/(float)gridHeight;
  float xPos = -gridWidthHalf*xDiff;
  for(int i = 0; i <gridWidth ; i++){
    float yPos = gridHeightHalf*yDiff;
    for(int j = 0; j <gridHeight*3; j+=3){
      densityGrid[(i*gridHeight * 3) + j]     = xPos;
      densityGrid[(i*gridHeight * 3) + j + 1] = yPos*2;
      yPos -= yDiff;
    }
    xPos += xDiff;
  }
  

  //for(int i = 0; i < 41*21*3; i+=3){
  //  std::cout << "X: " << densityGrid[i] << "  Y: " << densityGrid[i+1] << "  Z: " << densityGrid[i+2] << std::endl;
  //}

  glAttachShader(gridProgram, gridVS);
  glAttachShader(gridProgram, gridFS);
  glLinkProgram(gridProgram);

  
  /////////////////////////////////////////////////////////////////////////



  //// Matrices
  glm::vec2 cameraAngles = glm::vec2(0.f, 0.f);
  glm::vec3 cameraPosition = glm::vec3(0.f, 0.f, CAM_DST);

  glm::mat4 cameraRotationMatrix = 
    glm::rotate(cameraAngles.x, glm::vec3(1,0,0))*
    glm::rotate(cameraAngles.y, glm::vec3(0,1,0));

  auto view = cameraRotationMatrix * glm::translate(cameraPosition);
  auto proj = glm::perspective(glm::half_pi<float>(), (float)width/(float)height, 0.1f, 1000.f);


  GLint _loc_proj = glGetUniformLocation(drawProgram, "proj");
  GLint _loc_view = glGetUniformLocation(drawProgram, "view");
  GLint _loc_model = glGetUniformLocation(drawProgram, "model");

  GLint _border_proj = glGetUniformLocation(drawProgram, "proj");
  GLint _border_view = glGetUniformLocation(drawProgram, "view");
  GLint _border_model = glGetUniformLocation(drawProgram, "model");

  GLint _grid_proj = glGetUniformLocation(drawProgram, "proj");
  GLint _grid_view = glGetUniformLocation(drawProgram, "view");
  GLint _grid_model = glGetUniformLocation(drawProgram, "model");


  GLint uniform_particles[PARTICLE_COUNT];
  for(int i = 0; i < PARTICLE_COUNT; i++){
    char text[20] = "particle";
    char number[10];
    sprintf(number, "%d", i);
    strcat(text, number);
    uniform_particles[i] = glGetUniformLocation(borderProgram, text);
  }

  ////////////// Test of instanced model matrices
  /*
  GLuint translate_buffer;
  glBindBuffer(GL_ARRAY_BUFFER, translate_buffer);


  for (uint i = 0; i < 4; i++){
    glEnableVertexAttribArray(MODEL_LOC + i);
    glVertexAttribPointer(MODEL_LOC + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
          (const GLvoid*)(sizeof(GLfloat)*i*4));
    glVertexAttribDivisor(MODEL_LOC + i, 1);
  }*/
  GLuint test_buffer;
  glGenBuffers(1, &test_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, test_buffer);
  glVertexAttribPointer(MODEL_LOC, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
  glVertexAttribDivisor(MODEL_LOC, 1);
  glEnableVertexAttribArray(MODEL_LOC);

  GLuint color_buffer;
  glGenBuffers(1, &color_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
  glVertexAttribPointer(COLOR_LOC, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
  glVertexAttribDivisor(COLOR_LOC, 1);
  glEnableVertexAttribArray(COLOR_LOC);

  float test[3] = {1.f, -1.f, 0.f};

  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  bool reset = false;
  bool clamped = false;
  
  float translates[PARTICLE_COUNT*3] = {
    -1.f, 0.5f, 0.f,
    1.f, -0.5f, 0.0f,
    0.f, 0.f, 0.0f,
    -1.5f, 0.8f, 0.f,
    0.8f, -0.9f, 0.0f,
    1.f, 1.f, 0.0f
  };

  float directions[PARTICLE_COUNT*3] = {
     0.02f,  0.f,0.f,
    -0.05f,  0.f,0.f,
     0.01f,0.02f,0.f,
     0.02f,  0.f,0.f,
    -0.05f,  0.f,0.f,
     0.01f,0.02f,0.f
  };



  float mouseX = 0;
  float mouseY = 0;
  float mouseFactor = .1f;
  float powerLoss = 1.f;
  float gravityFactor = 1.f;
  bool drag = false;
  auto model =  glm::translate(glm::vec3(0.f,0.f,0.f)) * glm::rotate(0.0f,glm::vec3(0,1,0)) *  glm::scale(glm::vec3(1.f, 1.f, 1.f));
  //////////////////////////// MAIN LOOP /////////////////////////////////
  while (running)
  { 

    // FIXME
    //computeDensity(densityGrid, translates, gridValues, gridHeight*gridWidth, PARTICLE_COUNT);

    /////////////////////// EVENT CATCH ///////////////////////
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_KEYDOWN)
      {
        switch (event.key.keysym.sym)
        {
          case SDLK_ESCAPE:
            running = false;
            break;
          case SDLK_SPACE:
            drag = !drag;
          default:
            break;
        }
      }
      if(event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT){
        int X, Y;
        SDL_GetMouseState(&X, &Y);
        mouseX = (((float)X / (float)width) * 4.f) - 2.f;
        mouseY = -((((float)Y / (float)height) *2.f) - 1.f);
      }
      if (event.type == SDL_QUIT)
      {
        running = 0;
      }
    }

    /////////////////////// IMGUI ///////////////////////
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    /// ImGui::ShowDemoWindow(); 

    //// ------------------------------------ ////
    ImGui::Begin("Hello world!");
    ImGui::ColorEdit3("clear color", (float*)&clear_color);
    ImGui::SameLine();
    if(ImGui::Button("Reset color")){
      reset = true;
    }
    else{reset = false;}
    ImGui::Text("MouseX: %.4f\n Mouse Y: %.4f", mouseX, mouseY);
    ImGui::SliderFloat("Mouse Factor", &mouseFactor, 0.f, 0.1f);
    ImGui::SliderFloat("Power Loss", &powerLoss, 0.9f, 1.f);
    ImGui::SliderFloat("Gravity factor", &gravityFactor, 0.0f, 1.f);
    ImGui::Text("Allowed Distance: %.4f", allowed_distance);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

    ImGui::End();
    //// ------------------------------------ ////

    ////////////////////////// Binding buffers



    /////////////////////// FRAME RENDERING ///////////////////////
    glViewport(0, 0, width, height);
    if(reset){
      clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    }
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    // std::cout << direction[0] << " | " << direction[1] << std::endl;
    //////////////////////////////////////// Border ////////////////////////////////////////
    glUseProgram(borderProgram);

    glUniformMatrix4fv(_border_proj, 1, false, (float*)&proj);
    glUniformMatrix4fv(_border_view, 1, false, (float*)&view);
    glUniformMatrix4fv(_border_model, 1, false, (float*)&model);


    // FIXME: 6 more uniforms
    for(int i = 0; i < PARTICLE_COUNT; i++){
      int t_index = 3*i;
      glUniform3f(uniform_particles[i], (translates[t_index]*2), (translates[t_index+1]), translates[t_index+2]);
    }


    glBindVertexArray(border_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    //////////////////////////////////////// GRID ////////////////////////////////////////
    /*
    glUseProgram(gridProgram);

    glUniformMatrix4fv(_grid_proj, 1, false, (float*)&proj);
    glUniformMatrix4fv(_grid_view, 1, false, (float*)&view);
    glUniformMatrix4fv(_grid_model, 1, false, (float*)&model);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gridValues), gridValues, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, test_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(densityGrid), densityGrid, GL_DYNAMIC_DRAW);
    glDrawArraysInstanced(GL_TRIANGLES, 0, radius_points*3, GRID_X*GRID_Y);
    */

    ///////////////////////////////////////// Particles ////////////////////////////////////////
    glUseProgram(drawProgram);

    
    //glBindBuffer(GL_ARRAY_BUFFER, test_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, test_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(translates), translates, GL_DYNAMIC_DRAW);   

    glUniformMatrix4fv(_loc_proj, 1, false, (float*)&proj);
    glUniformMatrix4fv(_loc_view, 1, false, (float*)&view);
    glUniformMatrix4fv(_loc_model, 1, false, (float*)&model);

    
    glBindVertexArray(vao);

    glDrawArraysInstanced(GL_TRIANGLES, 0, radius_points*3, PARTICLE_COUNT);

    /////////////////////////////////////////////////////////////////////////////////

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window);

    //////////////// Update data
    
    velocity++;
    for(int i = 0; i < 18; i+=3){
        glm::vec3 direction = glm::make_vec3(&directions[i]);
        glm::vec3 translate = glm::make_vec3(&translates[i]);
        direction += gravityFactor * gravity;
        //std::cout << translate.x << " | " << translate.y << " | " << translate.z << std::endl;
        

        if(drag){
          glm::vec3 toMouseDirection = glm::vec3(mouseX, mouseY*2, 0.f) - translate;
          //toMouseDirection.y *= -1.f;
          direction += (toMouseDirection * mouseFactor);
        }

        translate += direction;

        if(translate.y <= -allowed_distance || translate.y >= allowed_distance){
          translate.y = (direction.y >= 0 ? allowed_distance : -allowed_distance);
          direction.y *= -1.f;
          direction *= POWER_LOSS;
        }

        if(translate.x <= -allowed_distance || translate.x >= allowed_distance){
          translate.x = (direction.x >= 0 ? allowed_distance : -allowed_distance);
          direction.x *= -1.f;
          direction *= POWER_LOSS;
        }
        direction *= powerLoss;
        directions[i] = direction.x;
        directions[i+1] = direction.y;
        directions[i+2] = direction.z;

        translates[i] = translate.x;
        translates[i+1] = translate.y;
        translates[i+2] = translate.z;
    }
  }

  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  return 0;
}
