#version 330 core
out vec4 FragColor;

// dont really need specular lighting for our usecase
in vec2 texCoord;
in vec3 Normal;
in vec3 currentPos;

uniform sampler2D tex0;
uniform vec4 lightC;
uniform vec3 lightPos;
uniform bool use_texture;

void main() {
    float ambient = 0.2f;
    vec3 N = normalize(Normal);

    vec3 L = normalize(
        lightPos - currentPos
    );

    float diff = max(dot(N, L), 0.0);

    vec4 texColor;
    if (use_texture)
    {
        texColor = texture(tex0, texCoord);
    } else {
        texColor = vec4(1.0, 1.0, 1.0, 1.0);
    }

    FragColor = texColor * (diff + ambient) * lightC;
}