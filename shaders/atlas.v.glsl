#version 330 core

layout (location = 0) in vec4 pos_uv;
out vec2 uv;

uniform mat4 view;

void main(void) {
  gl_Position = view * vec4(pos_uv.xy, 0, 1);
  uv = pos_uv.zw;
}