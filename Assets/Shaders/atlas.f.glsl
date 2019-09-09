#version 330 core

in vec2 uv;
uniform sampler2D texture;
uniform vec4 inputColor;

out vec4 color;

void main(void) {
  color = vec4(1.0f, 1.0f, 1.0f, texture(texture, uv).r) * inputColor;
}