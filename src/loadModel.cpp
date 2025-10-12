#include <cstdio>
#include <vector>
#include <cstring>
#include <iostream>

#include "../glm/ext/vector_float2.hpp"
#include "../glm/ext/vector_float3.hpp"


bool loadObj(const char* path,
            std::vector<float> &outVertices,
            std::vector<float> &outUvs,
            std::vector<float> &outNormals)
{
  FILE* fp = fopen(path, "r");
  if (fp == NULL)
  {
    printf("Can't even open the file\n");
    return false;
  }

  std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
  std::vector<glm::vec3> temp_vertices;
  std::vector<glm::vec3> temp_uvs;
  std::vector<glm::vec3> temp_normals;

  // Formatting data
  while (1)
  {
    char lineHeader[128];
    // Read first word of the line
    if (fscanf(fp, "%s", lineHeader) == EOF)
      break;

    if (strcmp(lineHeader, "v") == 0)
    {
      glm::vec3 vertex;
      fscanf(fp, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
      temp_vertices.push_back(vertex);
    }
    else if (strcmp(lineHeader, "vn") == 0)
    {
      glm::vec3 normal;
      fscanf(fp, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
      temp_normals.push_back(normal);
    }
    else if (strcmp(lineHeader, "vt") == 0)
    {
      glm::vec3 uv;
      fscanf(fp, "%f %f\n", &uv.x, &uv.y);
      temp_uvs.push_back(uv);
    }
    else if (strcmp(lineHeader,"f") == 0)
    {
      unsigned int vertex[3], uv[3], normal[3]; 
      int matches = fscanf(fp, "%u/%u/%u %u/%u/%u %u/%u/%u\n",
                                &vertex[0], &uv[0], &normal[0],
                                &vertex[1], &uv[1], &normal[1],
                                &vertex[2], &uv[2], &normal[2]);

      if (matches != 9)
      {
        printf("Create a better praser\n");
        return false;
      }
      
      vertexIndices.push_back(vertex[0]);
      vertexIndices.push_back(vertex[1]);
      vertexIndices.push_back(vertex[2]);
      uvIndices.push_back(uv[0]);
      uvIndices.push_back(uv[1]);
      uvIndices.push_back(uv[2]);
      normalIndices.push_back(normal[0]);
      normalIndices.push_back(normal[1]);
      normalIndices.push_back(normal[2]);
    }
  }
  
  fclose(fp);

  // Processing data
  for (unsigned int i = 0; i < vertexIndices.size(); i++)
  {
    unsigned int index = vertexIndices[i];
    glm::vec3 vertex = temp_vertices[index - 1];
    outVertices.push_back(vertex.x);
    outVertices.push_back(vertex.y);
    outVertices.push_back(vertex.z);
  }

  for (unsigned int i = 0; i < uvIndices.size(); i++)
  {
    unsigned int index = uvIndices[i];
    glm::vec2 uv = temp_uvs[index - 1];
    outUvs.push_back(uv.x);
    outUvs.push_back(uv.y);
  }

  for (unsigned int i = 0; i < normalIndices.size(); i++)
  {
    unsigned int index = normalIndices[i];
    glm::vec3 normal = temp_normals[index - 1];
   
    outNormals.push_back(normal.x);
    outNormals.push_back(normal.y);
    outNormals.push_back(normal.z);
  }
    
  std::cout << "In load model file" << std::endl;
  std::cout << "Size of vertexIndex: " << vertexIndices.size() << std::endl;
  std::cout << "Size of uvIndex: " << uvIndices.size() << std::endl;

  return true;
}
