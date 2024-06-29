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
#include <unistd.h>
#include <cmath>

#include "shaderLoader.hpp"
#include "particles.hpp"
#include "grid.hpp"

#ifndef CMAKE_ROOT_DIR
#define CMAKE_ROOT_DIR "."
#endif

#define DUAL false
#define SHADER_FOLDER "../shaders/"

#define SUCCESS   0
#define ERROR     1

#define DEBUG   std::cout << "DEBUG  " << __LINE__ << std::endl;
#define VERSION std::cout << glGetString(GL_VERSION) << std::endl;
#define PRINT   false

/// VARIABLES
#define POWER_LOSS        0.95f
#define CAM_DST            -1.1f
#define PARTICLE_COUNT     4000
#define GRID_X              128
#define GRID_Y               41
#define ARROW_VERTS          27

////////////////////////////////////////////////////////////////////////////////////////////////
//                    NOTES
//       max dst = sqrt(8)
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
GLuint create_draw_program(const char* shaderName){

    GLuint program = glCreateProgram();

    char filenameVS[124];
    char filenameFS[124];
    strcpy(filenameVS, shaderName);
    strcpy(filenameFS, shaderName);
    strcat(filenameVS, "VertexShader.glsl");
    strcat(filenameFS, "FragmentShader.glsl");

    GLuint vertexShader = loadAndCompileShader(filenameVS, GL_VERTEX_SHADER);
    GLuint fragmentShader = loadAndCompileShader(filenameFS, GL_FRAGMENT_SHADER);

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    return program;

}

////////////////////////////////////////////////////////////////////////////////////////////////////
GLuint create_compute_program(const char* shaderName){
    
    GLuint program = glCreateProgram();

    char filename[124];
    strcpy(filename, shaderName);
    strcat(filename, "ComputeShader.glsl");

    GLuint shader = loadAndCompileShader(filename, GL_COMPUTE_SHADER);

    glAttachShader(program, shader);

    return program;

}

void print2DVectors(float *input, int cnt){
  for(int i = 0; i < cnt; i++){
    printf("%2.2f : %2.2f\n", input[i], input[i+1]);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv){
    /// ----------------------------------------------------------------------------
    /// ----------------------------------------------------------------------------
    /// ----------------------------------- INIT -----------------------------------
    SDL_Window *window;
    int width = 1000;
    int height = 1000;
    /// SDL Init
    window = SDL_CreateWindow("PGPa2023_Flow",3220,0,width,height*1.2,SDL_WINDOW_OPENGL);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GLContext context = SDL_GL_CreateContext(window);

    /// ImGui Init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init();

    /// GLEW Init
    GLenum err = glewInit();
    if (err != GLEW_OK)
      exit(1); // or handle the error in a nicer way
    if (!GLEW_VERSION_2_1)  // check that the machine supports the 2.1 API.
      exit(1);

    /// Random Init
    srand(time(NULL));

    /// ----------------------------------------------------------------------
    /// #######################################################################

    /// #######################################################################
    /// --------------- PARTICLE DATA --------------------
    float particle_radius = 0.005f;
    float allowed_distance = (1 - particle_radius);
    uint32_t particle_segments = 20;
    float particle_verts[particle_segments * 3 * 3];
    generateCircle(particle_segments, particle_verts, particle_radius);
    /// -----------------------------------

    /// ---------------- UNIFORM STATIC VARS -------------------
    glm::vec3 gravity = glm::vec3(0.f, -0.00098f, 0.f);
    /// -----------------------------------

    /// --------------- PARTICLE DRAW PROGRAM --------------------
    GLuint particleProgram = create_draw_program("particle");
    glLinkProgram(particleProgram);
    //glUseProgram(particleProgram);
    
    GLuint particle_vao;
    glGenVertexArrays(1, &particle_vao);

    GLuint particle_vbo;
    glGenBuffers(1, &particle_vbo);

    glBindVertexArray(particle_vao);

    glBindBuffer(GL_ARRAY_BUFFER, particle_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particle_verts), particle_verts, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    /// -----------------------------------

    /// ----------------- PARTICLE DRAW UNIFORMS ------------------
    GLuint particle_proj = glGetUniformLocation(particleProgram, "proj");
    GLuint particle_view = glGetUniformLocation(particleProgram, "view");
    GLuint particle_model = glGetUniformLocation(particleProgram, "model");
    GLuint particle_color_switch = glGetUniformLocation(particleProgram, "col");
    /// -----------------------------------

    /// ----------------- GENERATING TRANSPOSE ------------------
    float translates[PARTICLE_COUNT * 3];

    generateTransposeInBorders(
      glm::vec2(-allowed_distance, allowed_distance),
      glm::vec2(-allowed_distance, allowed_distance), PARTICLE_COUNT, translates);
    /// -----------------------------------

    /// ------------------ GENERATING DIRECTIONS -----------------
    float directions[PARTICLE_COUNT * 2];
    std::fill_n(directions, PARTICLE_COUNT*2, 0);
    //generateDirections(PARTICLE_COUNT, directions);

    float next_translates[PARTICLE_COUNT * 3];
    std::fill_n(next_translates, PARTICLE_COUNT * 2, 0);

    float particleDensities[PARTICLE_COUNT];
    std::fill_n(particleDensities, PARTICLE_COUNT, 0);

    float particleGradients[PARTICLE_COUNT * 2];
    std::fill_n(particleGradients, PARTICLE_COUNT * 2, 0);

    float particlePressureForces[PARTICLE_COUNT * 2];
    std::fill_n(particlePressureForces, PARTICLE_COUNT * 2, 0);
    /// -----------------------------------

    /// ------------------- PER INSTANCE 3V BUFFER ----------------
    GLuint transforms_buffer;
    glGenBuffers(1, &transforms_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, transforms_buffer);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(1);

    GLuint velocities_buffer;
    glGenBuffers(1, &velocities_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, velocities_buffer);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
    glVertexAttribDivisor(2, 1);
    glEnableVertexAttribArray(2);
    /// -----------------------------------
    /// #######################################################################

    /// #######################################################################

    /// --------------- GRID DRAW PROGRAM --------------------
    GLuint backgroundProgram = create_draw_program("background");
    glLinkProgram(backgroundProgram);
    //glUseProgram(backgroundProgram);
    
    GLuint back_vao;
    glGenVertexArrays(1, &back_vao);

    GLuint back_vbo;
    glGenBuffers(1, &back_vbo);

    glBindVertexArray(back_vao);
    
    float border2[] = {
          -.5f,-.5f,0.f,
          .5f,-.5f,0.f,
          .5f,.5f,0.f,
          -5.f,5.f,0.f,
        };

    glBindBuffer(GL_ARRAY_BUFFER, back_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(border2), border2, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    /// ----------------- GRID DRAW UNIFORMS ------------------
    GLint background_proj = glGetUniformLocation(backgroundProgram, "proj");
    GLint background_view = glGetUniformLocation(backgroundProgram, "view");
    GLint background_model = glGetUniformLocation(backgroundProgram, "model");

    /// -----------------------------------
    /// #######################################################################

    /// #######################################################################
    /// ------------------------ PREDICT NEXT POSITION ---------------------------
    GLuint predictPosProgram = create_compute_program("predictPos");
    glLinkProgram(predictPosProgram);
    glUseProgram(predictPosProgram);

    GLuint predictParticlePositions = 0;
    GLuint predictParticleDirections = 0;
    GLuint predictNextPositions = 0;

    glGenBuffers(1, &predictParticlePositions);
    glGenBuffers(1, &predictParticleDirections);
    glGenBuffers(1, &predictNextPositions);

    GLuint predictGravity = glGetUniformLocation(predictPosProgram, "gravityMult");
    /// ---------------------------------------------------

    /// ------------------------ PARTICLE DENSIES COMPUTE PROGRAM ---------------------------
    GLuint pDnstProgram = create_compute_program("particleDst");
    glLinkProgram(pDnstProgram);
    glUseProgram(pDnstProgram);

    GLuint pDnstParticlePositions = 0;
    GLuint pDnstParticleDensities = 0;

    glGenBuffers(1, &pDnstParticlePositions);
    glGenBuffers(1, &pDnstParticleDensities);

    GLuint pDnstParticleCnt = glGetUniformLocation(pDnstProgram, "particleCnt");
    GLuint pDnstRadius = glGetUniformLocation(pDnstProgram, "radius");
    /// ---------------------------------------------------------

    /// -------------------------- PARTICLE GRADIENTS VECTORS COMPUTE -------------------------------
    GLuint pGrdProgram = create_compute_program("particleGrad");
    glLinkProgram(pGrdProgram);
    glUseProgram(pGrdProgram);

    GLuint pGrdParticlePositions = 0;
    GLuint pGrdParticleDensities = 0;
    GLuint pGrdParticleGradients = 0;

    glGenBuffers(1, &pGrdParticlePositions);
    glGenBuffers(1, &pGrdParticleDensities);
    glGenBuffers(1, &pGrdParticleGradients);

    GLuint pGrdParticleCnt = glGetUniformLocation(pGrdProgram, "particleCnt");
    GLuint pGrdRadius = glGetUniformLocation(pGrdProgram, "radius");
    /// ---------------------------------------------------------

    /// -------------------------- PARTICLE GRADIENTS VECTORS COMPUTE -------------------------------
    GLuint pPFProgram = create_compute_program("particlePressureForce");
    glLinkProgram(pPFProgram);
    glUseProgram(pPFProgram);

    GLuint pPFParticlePositions = 0;
    GLuint pPFParticleDensities = 0;
    GLuint pPFParticlePressureForces = 0;

    glGenBuffers(1, &pPFParticlePositions);
    glGenBuffers(1, &pPFParticleDensities);
    glGenBuffers(1, &pPFParticlePressureForces);

    GLuint pPFParticleCnt = glGetUniformLocation(pPFProgram, "particleCnt");
    GLuint pPFRadius = glGetUniformLocation(pPFProgram, "radius");
    GLuint pPFTargetDensity = glGetUniformLocation(pPFProgram, "targetDensity");
    GLuint pPFPressureMult = glGetUniformLocation(pPFProgram, "pressureMult");
    /// ---------------------------------------------------------

    /// -------------------------- PARTICLE DIRECTIONS -------------------------------
    GLuint particleDirectionsProgram = create_compute_program("directions");
    glLinkProgram(particleDirectionsProgram);
    glUseProgram(particleDirectionsProgram);

    GLuint pdParticleDirections = 0;
    GLuint pdParticleDensities = 0;
    GLuint pdParticlePressureForces = 0;

    glGenBuffers(1, &pdParticleDirections);
    glGenBuffers(1, &pdParticleDensities);
    glGenBuffers(1, &pdParticlePressureForces);

    GLuint pdParticleCnt = glGetUniformLocation(particleDirectionsProgram, "particleCnt");
    GLuint pdRadius = glGetUniformLocation(particleDirectionsProgram, "radius");
    /// ---------------------------------------------------------

    /// -------------------------- MOUSE INFLUENCE -------------------------------
    GLuint mouseMoveProgram = create_compute_program("mouseMove");
    glLinkProgram(mouseMoveProgram);
    glUseProgram(mouseMoveProgram);

    GLuint mmParticleDirections = 0;
    GLuint mmParticlePositions = 0;

    glGenBuffers(1, &mmParticlePositions);
    glGenBuffers(1, &mmParticleDirections);

    GLuint mmParticleCnt = glGetUniformLocation(mouseMoveProgram, "particleCnt");
    GLuint mmRadius = glGetUniformLocation(mouseMoveProgram, "radius");
    GLuint mmMousePos = glGetUniformLocation(mouseMoveProgram, "mouse");
    /// ---------------------------------------------------------------

    /// -------------------------- PARTICLE MOVEMENT -------------------------------
    GLuint movementProgram = create_compute_program("movement");
    glLinkProgram(movementProgram);
    glUseProgram(movementProgram);

    GLuint mv_particlePositions = 0;
    GLuint mv_particleDirections = 0;

    glGenBuffers(1, &mv_particlePositions);
    glGenBuffers(1, &mv_particleDirections);

    GLuint loc_pull = glGetUniformLocation(movementProgram, "pull");
    GLuint loc_collisionDamp = glGetUniformLocation(movementProgram, "collisionDamp");
    GLuint mv_x_border = glGetUniformLocation(movementProgram, "x_border");
    GLuint mv_y_border = glGetUniformLocation(movementProgram, "y_border");
    /// ---------------------------------------------------------
    /// #######################################################################

    /// #######################################################################
    /// ---------------- CAMERA MATRICES -------------------
    glm::vec2 cameraAngles = glm::vec2(0.f, 0.f);
    glm::vec3 cameraPosition = glm::vec3(0.f, 0.f, CAM_DST);

    glm::mat4 cameraRotationMatrix = 
    glm::rotate(cameraAngles.x, glm::vec3(1,0,0))*
    glm::rotate(cameraAngles.y, glm::vec3(0,1,0));

    auto view = cameraRotationMatrix * glm::translate(cameraPosition);
    auto proj = glm::perspective(glm::half_pi<float>(), (float)width/(float)height, 0.1f, 1000.f);
    auto model =  glm::translate(glm::vec3(0.f,0.f,0.f)) * glm::rotate(0.0f,glm::vec3(0,1,0)) *  glm::scale(glm::vec3(1.f, 1.f, 1.f));
    /// -----------------------------------

    /// --------------------------------- SPEC VARS ---------------------------------------
    bool running = true;
    bool mouse_down = false;
    bool mouse_in_borders = false;

    bool movement = false;
    bool mouse_check = false;
    float in_radius = 0.47f;

    bool color_enabled = true;

    float targetDensity = 1.0f;
    float pressureMult = 500.0f;
    float collisionDamp = 0.8f;
    float gravityMult = 0.0f;

    float x_size = 1.0f;
    float y_size = 1.0f;

    int x, y = 0;
    /// ---------------------------------------------------------------------------------


    /// ----------------------------------- DRAW LOOP -----------------------------------
    /// ---------------------------------------------------------------------------------
    
    while (running){
      

      /// ------------------------ SDL EVENT CAPTURE ----------------------
      SDL_Event event;
      while(SDL_PollEvent(&event)){

        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_KEYDOWN){
          switch (event.key.keysym.sym)
          {
            case SDLK_ESCAPE:
              running = false;
              break;
            default:
              break;
          }
        }
        if (event.type == SDL_QUIT)
        {
          running = false;
        }
        
        if(event.type == SDL_MOUSEBUTTONDOWN){
          mouse_down = true;
          SDL_GetMouseState(&x, &y);
          
        }
        if(event.type == SDL_MOUSEBUTTONUP){
          mouse_down = false;
        }
        if(event.type == SDL_MOUSEMOTION && mouse_down){
          SDL_GetMouseState(&x, &y);
          
        }
      }

      ///////////////////////////////// MOUSE POSITIONS RELATIVE FOR THIS SPECIFIC SETUP
      //// Not iddealy computed, but it is fastest for this use
      float x_pos =  (((  x/(float)width) * 2.f - 1.f)        /(0.9f));
      float y_pos = -(((( y-200.f)/(float)height) * 2.f - 1.f)/(0.9f));
      if(x_pos <= 1.f && x_pos >= -1.f && y_pos <= 1.f && y_pos >= -1.f){
        mouse_in_borders = true;
      }
      else{
        mouse_in_borders = false;
      }
      // std::max(1.f, std::min(), -1.f)
      // std::max(1.f, std::min(), -1.f)
      
      /// ----------------------------------------------------------------------

      /// ----------------------- ImGUI GUI --------------------------
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplSDL2_NewFrame();
      ImGui::NewFrame();
      ImGui::Begin("Fluid simulation - PGPa 2023 - Marek Hlávka");
      ImGui::Text("Application average %.3f", 1000.0f / io.Framerate);
      ImGui::Text("Ms/Frame (%.1f FPS)", io.Framerate);
      ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.36f);
      ImGui::SliderFloat("X Borders", &x_size, 0.1f, 1.f);
      ImGui::SameLine();
      ImGui::SliderFloat("Y Borders", &y_size, 0.1f, 1.f);
      ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.8f);
      ImGui::SliderFloat("Radius", &in_radius, .05f, .6f);
      ImGui::SliderFloat("Pressure Multiplier", &pressureMult, 0.01f, 500.f);
      ImGui::SliderFloat("Target density", &targetDensity, 1.f, 10.f);
      ImGui::SliderFloat("Collision damp", &collisionDamp, 0.f, 1.f);
      ImGui::SliderFloat("Gravity strenght", &gravityMult, 0.f, .1f);
      ImGui::Checkbox("Particle movement", &movement);
      ImGui::SameLine();
      ImGui::Checkbox("Mouse movement", &mouse_check);
      ImGui::SameLine();
      ImGui::Checkbox("Color enabled", &color_enabled);
      ImGui::End();
      /// ----------------------------------------------------------------------

      /// ------------------------- VIEWPORT CLEAR ---------------------------------
      glViewport(0, 0, width, height);
      glClearColor(0.f, 0.f, 0.f, 1.00f);
      glClear(GL_COLOR_BUFFER_BIT);
      /// ----------------------------------------------------------------------

      
      float radius = in_radius / 10.0f;


      /// **********************************************************************
      /// ------------------------- COMPUTE SHADERS AND PHYSICS ----------------


      /// ------------------------- PREDICT POSITIONS ------------------
      if(movement){
        glUseProgram(predictPosProgram);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, predictParticlePositions);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(translates), &translates, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, predictParticlePositions);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, predictParticleDirections);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(directions), &directions, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, predictParticleDirections);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, predictNextPositions);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(next_translates), &next_translates, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, predictNextPositions);

        glUniform1f(predictGravity, gravityMult);

        glDispatchCompute(PARTICLE_COUNT, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        /// ------------------- Return data from GPU -----------------
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, predictNextPositions);
        memcpy(next_translates, (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE), PARTICLE_COUNT*3*sizeof(float));
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, predictParticleDirections);
        memcpy(directions, (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE), PARTICLE_COUNT*2*sizeof(float));
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        /// ----------------------------------------------------------------------



        /// ------------------------- CALCULATE DENSITIES ------------------

        glUseProgram(pDnstProgram);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pDnstParticlePositions);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(next_translates), &next_translates, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pDnstParticlePositions);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pDnstParticleDensities);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(particleDensities), &particleDensities, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pDnstParticleDensities);

        glUniform1ui(pDnstParticleCnt, PARTICLE_COUNT);
        glUniform1f(pDnstRadius, radius);

        glDispatchCompute(PARTICLE_COUNT, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        /// ------------------- Return data from GPU -----------------
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pDnstParticleDensities);
        memcpy(particleDensities, (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE), PARTICLE_COUNT*sizeof(float));
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        /// ----------------------------------------------------------------------

        /// ------------------------- CALCULATE GRADIENTS ------------------
        
        glUseProgram(pGrdProgram);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pGrdParticlePositions);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(next_translates), &next_translates, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pGrdParticlePositions);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pGrdParticleDensities);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(particleDensities), &particleDensities, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pGrdParticleDensities);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pGrdParticleGradients);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(particleGradients), &particleGradients, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, pGrdParticleGradients);

        glUniform1f(pGrdParticleCnt, PARTICLE_COUNT);
        glUniform1f(pGrdRadius, radius);

        glDispatchCompute(PARTICLE_COUNT, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pGrdParticleGradients);
        memcpy(particleGradients, (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE), PARTICLE_COUNT*2*sizeof(float));
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        


        /// ----------------------------------------------------------------------

        /// ------------------------- CALCULATE PRESSURE FORCES ------------------
        glUseProgram(pPFProgram);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pPFParticlePositions);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(next_translates), &next_translates, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pPFParticlePositions);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pPFParticleDensities);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(particleDensities), &particleDensities, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pGrdParticleDensities);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pPFParticlePressureForces);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(particlePressureForces), &particlePressureForces, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, pPFParticlePressureForces);

        glUniform1f(pPFParticleCnt, PARTICLE_COUNT);
        glUniform1f(pPFRadius, radius);
        glUniform1f(pPFTargetDensity, targetDensity);
        glUniform1f(pPFPressureMult, pressureMult);

        glDispatchCompute(PARTICLE_COUNT, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pPFParticlePressureForces);
        memcpy(particlePressureForces, (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE), PARTICLE_COUNT*2*sizeof(float));
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        /// ----------------------------------------------------------------------

        /// ------------------------- CALCULATE MOVE VECTOR FOR PARTICLES --------

        glUseProgram(particleDirectionsProgram);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pdParticleDirections);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(directions), &directions, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pdParticleDirections);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pdParticleDensities);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(particleDensities), &particleDensities, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pdParticleDensities);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pdParticlePressureForces);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(particlePressureForces), &particlePressureForces, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, pdParticlePressureForces);

        glUniform1ui(pdParticleCnt, PARTICLE_COUNT);
        glUniform1f(pdRadius, radius);
        glDispatchCompute(PARTICLE_COUNT, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pdParticleDirections);
        memcpy(directions, (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE), PARTICLE_COUNT*2*sizeof(float));
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        /// ----------------------------------------------------------------------

        /// ------------------------- MOUSE INFLUENCE ----------------------------
        if(mouse_check && mouse_down && mouse_in_borders){
          glUseProgram(mouseMoveProgram);
          glBindBuffer(GL_SHADER_STORAGE_BUFFER, mmParticlePositions);
          glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(translates), &translates, GL_STATIC_DRAW);
          glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mmParticlePositions);

          glBindBuffer(GL_SHADER_STORAGE_BUFFER, mmParticleDirections);
          glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(directions), &directions, GL_STATIC_DRAW);
          glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mmParticleDirections);

          glUniform1ui(mmParticleCnt, PARTICLE_COUNT);
          glUniform1f(mmRadius, radius);
          glUniform2f(mmMousePos, x_pos, y_pos);
          glDispatchCompute(PARTICLE_COUNT, 1, 1);
          glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

          glBindBuffer(GL_SHADER_STORAGE_BUFFER, mmParticleDirections);
          memcpy(directions, (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE), PARTICLE_COUNT*2*sizeof(float));
          glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        }
        /// ----------------------------------------------------------------------

      }

      if(true){
        
        float border[] = {
          (x_size + particle_radius),-(y_size + particle_radius),0.f,
          (x_size + particle_radius),(y_size + particle_radius),0.f,
          -(x_size + particle_radius),-(y_size + particle_radius),0.f,
          -(x_size + particle_radius),(y_size + particle_radius),0.f,
        };

        glUseProgram(backgroundProgram);

        glBindBuffer(GL_ARRAY_BUFFER, back_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(border), border, GL_STATIC_DRAW);

        glUniformMatrix4fv(background_proj, 1, false, (float*)&proj);
        glUniformMatrix4fv(background_view, 1, false, (float*)&view);
        glUniformMatrix4fv(background_model, 1, false, (float*)&model);

        glBindVertexArray(back_vao);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
      }



      /// ------------------------- PARTICLE DRAW CALLS ---------------------------------
      glUseProgram(particleProgram);

      glBindBuffer(GL_ARRAY_BUFFER, transforms_buffer);
      glBufferData(GL_ARRAY_BUFFER, sizeof(translates), translates, GL_DYNAMIC_DRAW);

      glBindBuffer(GL_ARRAY_BUFFER, velocities_buffer);
      glBufferData(GL_ARRAY_BUFFER, sizeof(directions), directions, GL_DYNAMIC_DRAW);

      glUniformMatrix4fv(particle_proj, 1, false, (float*)&proj);
      glUniformMatrix4fv(particle_view, 1, false, (float*)&view);
      glUniformMatrix4fv(particle_model, 1, false, (float*)&model);
      glUniform1ui(particle_color_switch, (color_enabled?1:0));

      glBindVertexArray(particle_vao);

      glDrawArraysInstanced(GL_TRIANGLES, 0, particle_segments*3, PARTICLE_COUNT);
      /// ----------------------------------------------------------------------

      /// ------------------------- WINDOW DRAW ---------------------------------



      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      SDL_GL_SwapWindow(window);
      /// ----------------------------------------------------------------------

      /// ------------------------- MOVE PARTICLES -----------------------------
      if(movement){
        glUseProgram(movementProgram);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mv_particlePositions);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(translates), &translates, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mv_particlePositions);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mv_particleDirections);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(directions), &directions, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mv_particleDirections);

        glUniform1f(loc_pull, 0.01f);
        glUniform1f(loc_collisionDamp, collisionDamp);
        glUniform1f(mv_x_border, x_size);
        glUniform1f(mv_y_border, y_size);

        glDispatchCompute(PARTICLE_COUNT, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mv_particlePositions);
        memcpy(translates, (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE), PARTICLE_COUNT*3*sizeof(float));
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mv_particleDirections);
        memcpy(directions, (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE), PARTICLE_COUNT*2*sizeof(float));
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

      }
      /// ----------------------------------------------------------------------

      /// ----------------------------------------------------------------------
      /// **********************************************************************
      
    }
    

    /// ----------------------------------------------------------------------
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);

    ///glDeleteBuffers(1, &transforms_buffer);
///
    ///glDeleteBuffers(1, &cd_gridPoints);
    ///glDeleteBuffers(1, &cd_gridVals);
    ///glDeleteBuffers(1, &cd_particlePoints);
///
    ///glDeleteBuffers(1, &cv_gridVals);
    ///glDeleteBuffers(1, &cv_gridVectors);
///
    ///glDeleteBuffers(1, &pd_particlePosition);
    ///glDeleteBuffers(1, &pd_particleDirections);
    ///glDeleteBuffers(1, &pd_newParticleDirections);
///
    ///glDeleteBuffers(1, &mv_particlePositions);
    ///glDeleteBuffers(1, &mv_particleDirections);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    return SUCCESS;


    ///////// Create Map for shaders
}