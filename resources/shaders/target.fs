#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
layout(location = 0) out vec4 color;

void main()
{    
    color = vec4(1.0, 1.0, 1.0, 1.0);
}