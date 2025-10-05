#version 410 core

layout(location=0) in vec3 position;

uniform mat4 u_model; // Local to world
uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
    vec4 newPosition = u_projection * u_view * u_model * vec4(position, 1.0f);
    gl_Position = newPosition;
}
