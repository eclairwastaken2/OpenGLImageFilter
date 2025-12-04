#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D image;

void main()
{
    vec3 color = texture(image, TexCoords).rgb;
    float dist = distance(TexCoords, vec2(0.5));
    float vig = smoothstep(0.9, 0.4, dist);
    color *= vig;
    FragColor = vec4(color, 1.0);
}