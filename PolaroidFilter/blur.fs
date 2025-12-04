#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D image;

void main()
{
    float offset = 1.0 / 300.0;
    vec3 color = vec3(0.0);

    color += texture(image, TexCoords + vec2(-offset,  offset)).rgb * 0.111;
    color += texture(image, TexCoords + vec2( 0.0f,    offset)).rgb * 0.111;
    color += texture(image, TexCoords + vec2( offset,  offset)).rgb * 0.111;

    color += texture(image, TexCoords + vec2(-offset,  0.0f)).rgb * 0.111;
    color += texture(image, TexCoords).rgb * 0.111;
    color += texture(image, TexCoords + vec2( offset,  0.0f)).rgb * 0.111;

    color += texture(image, TexCoords + vec2(-offset, -offset)).rgb * 0.111;
    color += texture(image, TexCoords + vec2( 0.0f,   -offset)).rgb * 0.111;
    color += texture(image, TexCoords + vec2( offset, -offset)).rgb * 0.111;

    FragColor = vec4(color, 1.0);
}