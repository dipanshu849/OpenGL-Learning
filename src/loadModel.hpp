#include "../glm/ext/vector_float2.hpp"
#include "../glm/ext/vector_float3.hpp"


bool loadObj(const char* path,
             std::vector<float> &outVertices,
             std::vector<glm::vec2> &outUvs,
             std::vector<glm::vec3> &outNormals);
