#version 330 core

in vec3 fragmentColor;

out vec3 color;

void main()
{
    color = vec3(0.7,0.7,0.7) * fragmentColor;
}
