#version 460

layout(location = 0) in vec3 FragColor;

layout(location = 0) out vec4 PixelColor;

void main() {
    PixelColor = vec4(FragColor, 1.0);
}