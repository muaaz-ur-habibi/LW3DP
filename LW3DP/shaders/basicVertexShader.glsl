#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in vec3 aNormal;

out vec2 texCoord;
out vec3 Normal;
out vec3 currentPos;

uniform mat4 camMat;
uniform mat4 model;

void main()
{
    vec4 v4Pos = model * vec4(aPos, 1.0f);
    currentPos = v4Pos.xyz;  

    gl_Position = camMat * v4Pos;

    texCoord = aTex;
    Normal = mat3(transpose(inverse(model))) * aNormal;
}