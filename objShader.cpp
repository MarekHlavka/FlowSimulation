#include<shaders/objShader.hpp>

const char* objVsSrc = R".(
  #version 460

  uniform mat4 model = mat4(1);
  uniform mat4 view  = mat4(1);
  uniform mat4 proj  = mat4(1);
  uniform vec3 light = vec3(2.f);
  uniform vec3 camera = vec3(0.f,0.f,4.f);

  layout(location=0)in vec3 position;
  layout(location=1)in vec2 texCoord;
  layout(location=2)in vec3 normal;
  layout(location=3)in vec3 tangent;
  layout(location=4)in vec3 bitangent;

  out vec2 tex;
  out vec3 vecToLight;
  out mat4 translate;
  out vec3 toCameraPos;
  out mat3 fromTangent;
  out vec3 normIn;
  out vec3 cameraPos;
  out vec2 posDiff;
  out vec3 pos;
 
  void main(){
    translate = model;
    vec4 worldPos = model * vec4(position,1.0);

    tex = texCoord;
    gl_Position = proj * view * worldPos;
    vecToLight = light - worldPos.xyz;
    toCameraPos = camera - worldPos.xyz;
    cameraPos = camera;
    vec3 modelCamera = (model * vec4(-camera, 1.0)).xyz;

    vec3 n = normalize((model * vec4(normal, 0.0)).xyz);
    vec3 t = normalize((model * vec4(tangent, 0.0)).xyz);
    vec3 b = normalize((model * vec4(bitangent, 0.0)).xyz);

    fromTangent = mat3(
        t,
        b,
        n
    );
    normIn = n;
    posDiff = vec2((modelCamera.x - gl_Position.x), (gl_Position.y - modelCamera.y));
    pos = worldPos.xyz;


  }
  ).";


//      [Tx Ty Tz]
//TBN = [Bx By Bz]
//      [Nx Ny Nz]
const char* objFsSrc  = R".(
  #version 460

  out vec4 fColor;

  layout(binding=0)uniform sampler2D img;
  layout(binding=1)uniform sampler2D normal;
  layout(binding=2)uniform sampler2D ao;
  layout(binding=3)uniform sampler2D shin;
  layout(binding=4)uniform sampler2D specular;
  layout(binding=5)uniform sampler2D displacement;

  uniform float iTime = 0.f;
  uniform vec3 light_color = vec3(1.f);
  uniform int stage = 0;

  in vec2 tex;
  in vec3 vecToLight;
  in mat4 translate;
  in vec3 toCameraPos;
  in mat3 fromTangent;
  in vec3 normIn;
  in vec3 cameraPos;
  in vec2 posDiff;
  in vec3 pos;

  vec3 phong(
    vec3 normal,
    vec3 toLightPosition,
    vec3 toCamera,
    vec3 lightColor,
    vec3 ambient,
    vec3 diffuseColor,
    float shininess,
    float spec
  ){
    vec3 L = normalize(toLightPosition);
    float Df = max(dot(normal, L),0);

    vec3 V = normalize(toCamera);
    vec3 R = -reflect(L,normal);
    float Sf = pow(max(dot(R,V),0),shininess);

    vec3 ambientLight = ambient*diffuseColor;
    vec3 diffuseLight = Df*diffuseColor*lightColor;
    vec3 specularLight = Sf*vec3(1,1,1)*lightColor*spec;

    return ambientLight + diffuseLight + specularLight;
  }

  void main(){
    vec2 UVs = tex;

    vec3 norm = normIn;
    vec3 worldNormal = normalize(fromTangent * norm); 
    

    vec3 ao_val = vec3(1,1,1);
    float shin_val = 1.0;
    float spec_val = 0.0;

    if(stage >=1){
      if(stage >= 5){
        vec3 directionToEye = normalize(vec3(-cameraPos.x, -cameraPos.y, cameraPos.z) - pos);


        // Parallax displacement
        float heightScale = 0.04f;
        const float minLayers = 8.0f;
        const float maxLayers = 128.0f;
        float numLayers = 2 * mix(maxLayers, minLayers, 1 - abs(dot(normIn, directionToEye)));
        float layerDepth = 1.0f / numLayers;
        float currentLayerDepth = 0.0f;
        
        vec2 S = directionToEye.xy / directionToEye.z * heightScale; 
        vec2 deltaUVs = S / numLayers;
        
        float currentDepthMapValue = 1.0f - texture(displacement, UVs).r;
        
        // Loop till the point on the heightmap is "hit"
        while(currentLayerDepth < currentDepthMapValue)
          {
              UVs.x -= deltaUVs.x;
              UVs.y += deltaUVs.y;
              currentDepthMapValue = 1.0f - texture(displacement, UVs).r;
              currentLayerDepth += layerDepth;
          }

        // Apply Occlusion (interpolation with prev value)
        vec2 prevTexCoords;
        prevTexCoords.x = UVs.x + deltaUVs.x;
        prevTexCoords.y = UVs.y - deltaUVs.y;
        float afterDepth  = currentDepthMapValue - currentLayerDepth;
        float beforeDepth = 1.0f - texture(displacement, prevTexCoords).r - currentLayerDepth + layerDepth;
        float weight = afterDepth / (afterDepth - beforeDepth);
        UVs = prevTexCoords * weight + UVs * (1.0f - weight);

        // Get rid of anything outside the normal range
        if(UVs.x > 1.0 || UVs.y > 1.0 || UVs.x < 0.0 || UVs.y < 0.0)
          discard;
      }

      norm = (2*texture(normal, UVs).xyz)-1;
      worldNormal = normalize(fromTangent * norm); 
      
      if(stage >= 2){
        ao_val = vec3(texture(ao, UVs).x);
      }
      if(stage >= 3){
        spec_val = texture(specular, UVs).x;
      }
      if(stage >= 4){
        shin_val = 1 - texture(shin, UVs).x;
      }
    }

    fColor = vec4(phong(worldNormal, vecToLight, toCameraPos, light_color,
      ao_val, texture(img, UVs).xyz, shin_val, spec_val), 1.0);
    //fColor = vec4(abs(normalize(UVs - tex)), 0.0, 1.0);
  }

  )."; 

  