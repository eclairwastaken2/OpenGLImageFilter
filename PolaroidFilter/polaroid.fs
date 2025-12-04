#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D image;

void main()
{
    vec3 c = texture(image, TexCoords).rgb;

    // small color grade
    c = pow(c, vec3(0.95));

    // warm highlights
    c.r += 0.03;

    // tiny film fade
    c = mix(vec3(0.95), c, 0.92);

    // polaroid-like white border
    float border = 0.06; // relative
    float bottomExtra = 0.05;
    vec3 paper = vec3(1.0);
    if (TexCoords.x < border || TexCoords.x > 1.0 - border ||
        TexCoords.y < border || TexCoords.y > 1.0 - (border + bottomExtra))
    {
        FragColor = vec4(paper, 1.0);
        return;
    }

    FragColor = vec4(c, 1.0);
}