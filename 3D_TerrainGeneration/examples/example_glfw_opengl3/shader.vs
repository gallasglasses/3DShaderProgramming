#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aNormal;

out VS_OUT {
    vec3 WorldPos;
    vec3 Normal;
    vec2 TexCoord;
    vec3 Color;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool inverse_normals;

void main()
{
    vec4 world = model * vec4(aPos, 1.0);
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    vec3 n = inverse_normals ? -aNormal : aNormal;

    vs_out.WorldPos = world.xyz;
    vs_out.Normal = normalize(normalMatrix * n);
    vs_out.TexCoord = aTexCoord;
    vs_out.Color = aColor;

    gl_Position = projection * view * world;
};
