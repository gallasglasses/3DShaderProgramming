#version 330 core
out vec4 FragColor;
//uniform vec4 ourColor;
in vec3 ourColor;
//in vec3 ourPos;

//in vec2 ourTexCoord;
//uniform sampler2D texture1;
//uniform sampler2D texture2;
//uniform float mixValue;
void main()
{
    //FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    //FragColor = ourColor;
    //FragColor = vec4(ourColor, 1.0f);
    //FragColor = vec4(ourPos, 1.0f); // bottom left side is black because of negative float 

    //FragColor = texture(ourTexture, ourTexCoord);
    //FragColor = texture(ourTexture, ourTexCoord) * vec4(ourColor, 1.0f);
    //FragColor = mix(texture(texture1, ourTexCoord), texture(texture2,ourTexCoord), mixValue);
    //FragColor = mix(texture(texture1, ourTexCoord), texture(texture2,ourTexCoord), 0.2);
    FragColor = vec4(0.35, 0.65, 0.35, 1.0) * vec4(ourColor, 1.0f);
}