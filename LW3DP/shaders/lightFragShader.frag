#version 330 core
out vec4 FragColor;

uniform vec4 lightC;

void main()
{
    FragColor = lightC;
}
