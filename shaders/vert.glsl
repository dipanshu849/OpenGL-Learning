#version 410 core

layout(location=0) in vec3 position;
layout(location=1) in vec2 texCoordinates;

out vec2 uv;

uniform mat4 u_model; // Local to world
uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
    vec4 newPosition = u_projection * u_view * u_model * vec4(position, 1.0f);
    gl_Position = newPosition;
    uv = texCoordinates;
}
