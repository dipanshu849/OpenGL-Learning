#version 410 core

in vec3 o_fragPos;
in vec3 o_normals;
in vec2 o_uv;
in vec3 o_gouraudShadingResult;

out vec4 FragColor;

uniform sampler2D u_texture;
uniform vec3 u_lightPos;
uniform vec3 u_lightColor;
uniform vec3 u_viewPos;
uniform int u_isPhong;

vec3 PhongShading() 
{
  // ambient
  float ambientStrength = 0.1;
  vec3 ambient = ambientStrength * u_lightColor;

  // diffuse [point light]
  vec3 norm = normalize(o_normals);
  vec3 lightDir = normalize(u_lightPos - o_fragPos); // both in world space
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = diff * u_lightColor;

  // specular 
  float specularStrength = 0.5;
  vec3 viewDir = normalize(u_viewPos - o_fragPos);
  vec3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 2);
  vec3 specular = specularStrength * spec * u_lightColor;


  vec3 result = (ambient + diffuse + specular);
  return result;
}

void main() 
{
  FragColor = texture(u_texture, o_uv);
  vec3 result = vec3(0.0, 0.0, 0.0); 
  if (u_isPhong == 1)
  {
    result = PhongShading() * vec3(FragColor);  
  }
  else
  {
    result = o_gouraudShadingResult * vec3(FragColor);
  }

  FragColor = vec4(result, 1.0);
}
