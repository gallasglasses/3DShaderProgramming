#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 texCoord;

uniform sampler2D terrainTex;

void main()
{
    // green color with height based brightness
    //FragColor = vec4(0.35, 0.65, 0.35, 1.0) * vec4(ourColor, 1.0f);
    FragColor = texture(terrainTex, texCoord);
}
