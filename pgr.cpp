#include "geGL/Buffer.h"
#include "geGL/VertexArray.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include<SDL.h>
#include<string.h>
#include<geGL/geGL.h>
#include<geGL/StaticCalls.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtx/transform.hpp>
#include<Timer.hpp>
#include<stb_image/stb_image.h>


#include<cube.hpp>
#include<drawObject.hpp>
#include<shaders/compareShader.hpp>
#include<shaders/objShader.hpp>
#include<shaders/lightShader.hpp>


#ifndef CMAKE_ROOT_DIR
#define CMAKE_ROOT_DIR "."
#endif

#ifndef TEX_FOLDERS
#define TEX_FOLDERS 7
#endif


using namespace ge::gl;

// Folder names of materials
const char* textures[TEX_FOLDERS] = {
  "brick",
  "scifi",
  "stone",
  "harsh",
  "slab",
  "stonework",
  "metal"
};

// Names of separate maps
const char* maps[6] = {
"/color.png",
"/normal.png",
"/ao.png",
"/roughness.png",
"/metallic.png",
"/height.png"
};

bool objLoaded = false;

// Testing function for printing Vector data
void pritnVec3(glm::vec3 vec, const char* name){
  std::cout << name << " | " << vec.x << " | " << vec.y << " | " << vec.z << std::endl;
}

// Function for loading vertexes, normals and uvs form Wavefront (.obj) files
bool loadObjFile(int mode, const char* path, std::vector<glm::vec3>& out_verts, std::vector<glm::vec2>& out_uvs,
  std::vector<glm::vec3>& out_normals, std::vector<glm::vec3>& out_tangent, std::vector<glm::vec3>& out_bitangent){

    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    FILE * file = fopen(path, "r");
    if(file == NULL){
      std::cout << "Unreadable file!" << std::endl;
      return false;
    }

    while (true)
    {
      char lineHeader[256];
      int res = fscanf(file, "%s\n", lineHeader);
      if(res == EOF)break;

      if(strcmp(lineHeader, "v") == 0){

        glm::vec3 vertex;
        fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
        temp_vertices.push_back(vertex);

      }else if(strcmp(lineHeader, "vt") == 0){

        glm::vec2 uv;
        fscanf(file, "%f %f\n", &uv.x, &uv.y);
        temp_uvs.push_back(uv);

      }else if(strcmp(lineHeader, "vn") == 0){

        glm::vec3 normal;
        fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
        temp_normals.push_back(normal);

      }else if(strcmp(lineHeader, "f") == 0){

        //std::string vertex1, vertex2, vertex3;
        unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
        int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
        //std::cout << matches << std::endl;
        if (matches != 9){
            
              std::cout << "File can't be read!" << matches << std::endl;
              return false;
            }
        vertexIndices.push_back(vertexIndex[0]);
        vertexIndices.push_back(vertexIndex[1]);
        vertexIndices.push_back(vertexIndex[2]);
        uvIndices    .push_back(uvIndex[0]);
        uvIndices    .push_back(uvIndex[1]);
        uvIndices    .push_back(uvIndex[2]);
        normalIndices.push_back(normalIndex[0]);
        normalIndices.push_back(normalIndex[1]);
        normalIndices.push_back(normalIndex[2]);

      }

    }

    for(unsigned int i = 0; i < vertexIndices.size(); i++){
      unsigned int vIndex = vertexIndices[i];
      glm::vec3 vertex = temp_vertices[vIndex - 1];
      out_verts.push_back(vertex); 
    }

    for(unsigned int i = 0; i < uvIndices.size(); i++){
      unsigned int uvUndex = uvIndices[i];
      glm::vec2 uv = temp_uvs[uvUndex - 1];
      out_uvs.push_back(uv);
    }

    for(unsigned int i = 0; i < normalIndices.size(); i++){
      unsigned int nIndex = normalIndices[i];
      glm::vec3 normal = temp_normals[nIndex - 1];
      out_normals.push_back(normal);
    }
    if(mode == 1){
      for(int i = 0; i < out_verts.size(); i+=3){

      glm::vec3 deltaPos1 = out_verts[i+1] - out_verts[i];
      glm::vec3 deltaPos2 = out_verts[i+2] - out_verts[i];

      glm::vec2 deltaUV1 = out_uvs[i+1] - out_uvs[i];
      glm::vec2 deltaUV2 = out_uvs[i+2] - out_uvs[i];

      float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
      glm::vec3 tangent = glm::normalize((deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r);
      glm::vec3 bitangent = -glm::normalize((deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r);

      out_tangent.push_back(tangent);
      out_tangent.push_back(tangent);
      out_tangent.push_back(tangent);

      out_bitangent.push_back(bitangent);
      out_bitangent.push_back(bitangent);
      out_bitangent.push_back(bitangent);
      }
    }
    
    return true;

  }

// Load and bind specific texture
void loadBindTex(GLuint tex, uint32_t texID, char* texPath){
  int x,y,channels;
  auto data = stbi_load(texPath,&x,&y,&channels,0);


  if(data)std::cerr << "mame: "<< texPath << "\nx: " << x << " | y: " << y << " | channels: " << channels << std::endl;


  glCreateTextures(GL_TEXTURE_2D,1,&tex);
  if(channels == 4)
    glTextureStorage2D(tex,1,GL_RGBA8,x,y);
  if(channels == 3)
    glTextureStorage2D(tex,1,GL_RGB8,x,y);
  if(channels == 2)
    glTextureStorage2D(tex,1,GL_RG8,x,y);
  if(channels == 1)
    glTextureStorage2D(tex,1,GL_R8,x,y);

  glTextureParameteri(tex,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTextureParameteri(tex,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

  glPixelStorei(GL_UNPACK_ROW_LENGTH,x);
  glPixelStorei(GL_UNPACK_ALIGNMENT ,1);
  if(channels == 4)
    glTextureSubImage2D(tex,0,0,0,x,y,GL_RGBA,GL_UNSIGNED_BYTE,data);
  if(channels == 3)
    glTextureSubImage2D(tex,0,0,0,x,y,GL_RGB,GL_UNSIGNED_BYTE,data);
  if(channels == 2)
    glTextureSubImage2D(tex,0,0,0,x,x,GL_RG,GL_UNSIGNED_BYTE,data);
  if(channels == 1)
    glTextureSubImage2D(tex,0,0,0,x,x,GL_RED,GL_UNSIGNED_BYTE,data);
  

  stbi_image_free(data);

  glBindTextureUnit(texID,tex);
}

// Function for creation of square model for texture demonstration
bool loadPlane(std::vector<glm::vec3>& out_verts, std::vector<glm::vec2>& out_uvs,
  std::vector<glm::vec3>& out_normals, std::vector<glm::vec3>& out_tangent, std::vector<glm::vec3>& out_bitangent){

    glm::vec3 verts[4] = {
      glm::vec3(-1.f, -1.f, 0.f),
      glm::vec3(1.f, -1.f, 0.f),
      glm::vec3(-1.f, 1.f, 0.f),
      glm::vec3(1.f, 1.f, 0.f)
    };
    glm::vec3 normal = glm::vec3(0,0,1);
    glm::vec2 uvs[4] = {
      glm::vec2(1.f, 0.f),
      glm::vec2(0.f, 0.f),
      glm::vec2(1.f, 1.f),
      glm::vec2(0.f, 1.f),
    };


    out_verts.push_back(verts[0]);
    out_verts.push_back(verts[1]);
    out_verts.push_back(verts[3]);
    out_verts.push_back(verts[3]);
    out_verts.push_back(verts[2]);
    out_verts.push_back(verts[0]);

    out_uvs.push_back(uvs[0]);
    out_uvs.push_back(uvs[1]);
    out_uvs.push_back(uvs[3]);
    out_uvs.push_back(uvs[3]);
    out_uvs.push_back(uvs[2]);
    out_uvs.push_back(uvs[0]);

    for(int i = 0; i < out_verts.size(); i+=3){

      glm::vec3 deltaPos1 = out_verts[i+1] - out_verts[i];
      glm::vec3 deltaPos2 = out_verts[i+2] - out_verts[i];

      glm::vec2 deltaUV1 = out_uvs[i+1] - out_uvs[i];
      glm::vec2 deltaUV2 = out_uvs[i+2] - out_uvs[i];

      float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
      glm::vec3 tangent = glm::normalize((deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r);
      glm::vec3 bitangent = -glm::normalize((deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r);

      out_tangent.push_back(tangent);
      out_tangent.push_back(tangent);
      out_tangent.push_back(tangent);

      out_bitangent.push_back(bitangent);
      out_bitangent.push_back(bitangent);
      out_bitangent.push_back(bitangent);

      out_normals.push_back(normal);
      out_normals.push_back(normal);
      out_normals.push_back(normal);

      pritnVec3(normal, "n");      
      pritnVec3(tangent, "t");
      pritnVec3(bitangent, "b");

    }

    return true;
  }

// Function for binding arrays of textures
void loadTextures(GLuint* texs, int texs_num){
  if(texs_num < sizeof(textures)){
    std::cout << textures[texs_num] << std::endl;
  }

  char texpath[6][128];
  for(int i = 0; i < 6; i++){
    strcpy(texpath[i], CMAKE_ROOT_DIR);
    strcat(texpath[i], "/resources/");
    strcat(texpath[i], textures[texs_num]);
    strcat(texpath[i], maps[i]);
    loadBindTex(texs[i], i, texpath[i]); 
  }
}

//////////////////////////////////////////////////////////// MAIN ////////////////////////////////
int main(int argc, char **argv){

  // Program inicialization
  SDL_Window*window;
  SDL_GLContext context;
  uint32_t width = 1624;
  uint32_t height = 1000;
  window = SDL_CreateWindow("PGR2022",0,0,width,height,SDL_WINDOW_OPENGL);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,6);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);
  context = SDL_GL_CreateContext(window);

  ge::gl::init();


  // Loading Sphere object for lighst source
  std::vector<glm::vec3> sp_verts;
  std::vector<glm::vec2> sp_uvs;
  std::vector<glm::vec3> sp_normals;
  std::vector<glm::vec3> sp_tan;
  std::vector<glm::vec3> sp_bit;

  if(!loadObjFile( 0,CMAKE_ROOT_DIR "/resources/sphere.obj", sp_verts, sp_uvs, sp_normals, sp_tan, sp_bit)){
    return 0;
  }

  // Creating VAO of sphere object
  auto vbo = std::make_shared<ge::gl::Buffer>(sp_verts.size()*sizeof(float)*3, &sp_verts[0]);
  auto ubo = std::make_shared<ge::gl::Buffer>(sp_uvs.size()*sizeof(float)*2, &sp_uvs[0]);
  auto nbo = std::make_shared<ge::gl::Buffer>(sp_normals.size()*sizeof(float)*3, &sp_normals[0]);

  auto vao = std::make_shared<ge::gl::VertexArray>();
  vao->addAttrib(vbo, 0, 3, GL_FLOAT, 3*sizeof(float), 0);
  vao->addAttrib(ubo, 1, 2, GL_FLOAT, 2*sizeof(float), 0);
  vao->addAttrib(nbo, 2, 3, GL_FLOAT, 3*sizeof(float), 0);


  // Loading plane model for texture demonstration
  std::vector<glm::vec3> verts;
  std::vector<glm::vec2> uvs;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec3> tang;
  std::vector<glm::vec3> bitang;

  
  if(argc < 2 ){
    if(!loadPlane(verts, uvs, normals, tang, bitang)){
      return 0;
    }
  }
  else{
    objLoaded = true;
    if(!loadObjFile(1, argv[1], verts, uvs, normals, tang, bitang)){
      return 0;
    }
  }

  auto vbo2 = std::make_shared<ge::gl::Buffer>(verts.size()*sizeof(float)*3, &verts[0]);
  auto ubo2 = std::make_shared<ge::gl::Buffer>(uvs.size()*sizeof(float)*2, &uvs[0]);
  auto nbo2 = std::make_shared<ge::gl::Buffer>(normals.size()*sizeof(float)*3, &normals[0]);
  auto tbo2 = std::make_shared<ge::gl::Buffer>(tang.size()*sizeof(float)*3, &tang[0]);
  auto bbo2 = std::make_shared<ge::gl::Buffer>(bitang.size()*sizeof(float)*3, &bitang[0]);

  auto vao2 = std::make_shared<ge::gl::VertexArray>();
  vao2->addAttrib(vbo2, 0, 3, GL_FLOAT, 3*sizeof(float), 0);
  vao2->addAttrib(ubo2, 1, 2, GL_FLOAT, 2*sizeof(float), 0);
  vao2->addAttrib(nbo2, 2, 3, GL_FLOAT, 3*sizeof(float), 0);
  vao2->addAttrib(tbo2, 3, 3, GL_FLOAT, 3*sizeof(float), 0);
  vao2->addAttrib(bbo2, 4, 3, GL_FLOAT, 3*sizeof(float), 0);

 
  glm::vec2 cameraAngles = glm::vec2(0.f,0.f);
  glm::vec3 cameraPosition = glm::vec3(0.f, 0.f, -2.5f);

  glm::mat4 cameraRotationMatrix = 
    glm::rotate(cameraAngles.x,glm::vec3(1,0,0))*
    glm::rotate(cameraAngles.y,glm::vec3(0,1,0));

  auto view  = cameraRotationMatrix * glm::translate(cameraPosition);
  auto proj = glm::perspective(glm::half_pi<float>(),(float)width/(float)height,0.1f,1000.f);

  glEnable(GL_DEPTH_TEST);


  // Loading textures
  GLuint texs[6];

  ////// Control vars
  // Number of loaded material
  int texnum = 0;
  // Detail of current material
  int stage = 0;

  loadTextures(texs, texnum);

  // Swithing of moving/motionless lightsource
  Timer<float> timer;
  bool turning = false;

  bool running = true;

  
  // main loop
  while(running){

    //std::cout << stage << std::endl;

    SDL_Event event;
    while(SDL_PollEvent(&event)){//event loop
      auto t = glm::transpose(cameraRotationMatrix);
      auto left     = glm::vec3(t[0]);
      auto top      = glm::vec3(t[1]);
      auto backward = glm::vec3(t[2]);
      float cameraSpeed = 0.1*5;
      if(event.type == SDL_QUIT)
        running = false;
      if(event.type == SDL_KEYDOWN){
        

        if(event.key.keysym.sym == SDLK_a)cameraPosition += left*cameraSpeed;
        if(event.key.keysym.sym == SDLK_d)cameraPosition -= left*cameraSpeed;
        if(event.key.keysym.sym == SDLK_w)cameraPosition -= top*cameraSpeed;
        if(event.key.keysym.sym == SDLK_s)cameraPosition += top*cameraSpeed;
        if(event.key.keysym.sym == SDLK_p){
          if(turning){turning = false;}
          else{turning = true;}
        }
        if(event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_RIGHT){
          if(event.key.keysym.sym == SDLK_LEFT)texnum += TEX_FOLDERS-1;
          if(event.key.keysym.sym == SDLK_RIGHT)texnum += 1;

          texnum = texnum%TEX_FOLDERS;
          loadTextures(texs, texnum);
        }
        if(event.key.keysym.sym == SDLK_UP){
          if(objLoaded){
            stage = std::min(4, ++stage);
          }
          else{
            stage = std::min(5, ++stage);
          }
          std::cout << stage << std::endl;
        }
        if(event.key.keysym.sym == SDLK_DOWN)stage = std::max(0, --stage);
        

      }
      if(event.type == SDL_MOUSEWHEEL){
        // scroll up
        if(event.wheel.y > 0)cameraPosition += event.wheel.preciseY*backward*cameraSpeed;
        // scroll down
        if(event.wheel.y < 0)cameraPosition += event.wheel.preciseY*backward*cameraSpeed;

      }

      if(event.type == SDL_MOUSEMOTION){
        if(event.motion.state&SDL_BUTTON_LMASK){
          float sensitivity = 0.01;
          cameraAngles.y += sensitivity * event.motion.xrel;
          cameraAngles.x += sensitivity * event.motion.yrel;

          cameraRotationMatrix = 
            glm::rotate(cameraAngles.x,glm::vec3(1,0,0))*
            glm::rotate(cameraAngles.y,glm::vec3(0,1,0));
        }
      }

    }


    auto time_d = 0.f;
    if(turning){
      time_d = timer.elapsedFromStart();
    }

    // Light source position
    glm::vec3 light_pos = glm::vec3(
       7.f * (glm::cos(time_d + 1) - .5f),
       0.f,
       7.f * (glm::sin(time_d + 1) - .5f)
    ) + glm::vec3(3,0,2);

    //glm::vec3 light_vec = light_pos;

    //rgb(247, 242, 191)

    glm::vec3 light_color = glm::vec3(247/255.f, 244/255.f, 216/255.f);


    view  = cameraRotationMatrix * glm::translate(cameraPosition);
    
    
    //std::cout << light_vec.x << " | " << light_vec.y << " | " << light_vec.z << std::endl;

    glClearColor(0.1,0.1,0.1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // Final object
    auto translate = glm::vec3(2.5f,0.f,0.f);
    auto model =  glm::translate(translate) * glm::rotate(0.0f,glm::vec3(0,1,0)) *  glm::scale(glm::vec3(2.f, 2.f, 2.f));
    drawObject(objVsSrc, objFsSrc, vao2, model, view, proj, cameraPosition,
      0.f, light_pos, light_color, verts.size(), stage);

    // Compare object
    auto translate2 = glm::vec3(-2.5f,0.f,0.f);
    auto model2 =  glm::translate(translate2) * glm::rotate(0.0f,glm::vec3(0,1,0)) *  glm::scale(glm::vec3(2.f, 2.f, 2.f));
    drawObject(cmpVsSrc, cmpFsSrc, vao2, model2, view, proj, cameraPosition,
      0.f, light_pos, light_color, verts.size(), stage);

    // Light object
    auto translate_light = light_pos;
    auto model_light =  glm::translate(translate_light) * glm::rotate(time_d,glm::vec3(0,1,0)) * glm::scale(glm::vec3(.2f, .2f, .2f));
    drawObject(lightVsSrc, lightFsSrc, vao, model_light, view, proj, cameraPosition,
      0.f, light_pos, light_color, sp_verts.size(), stage);
    SDL_GL_SwapWindow(window);

  }
  
  for(int i = 0; i < 6; i++){
    glDeleteTextures(i,&texs[i]);
  }

  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);

  return 0;
}
