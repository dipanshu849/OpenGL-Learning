#version 410 core

in vec3 v_color;
out vec4 color;

void main() {
    color = vec4(v_color.r, v_color.g, v_color.b, 1.0f);
}