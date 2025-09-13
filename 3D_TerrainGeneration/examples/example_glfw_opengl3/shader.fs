#version 330 core

in VS_OUT {
    vec3 WorldPos;
    vec3 Normal;
    vec2 TexCoord;
    vec3 Color;
} fs_in;

out vec4 FragColor;

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

//in vec3 ourColor;
//in vec2 texCoord;

uniform sampler2D terrainTex;
uniform Light light;
uniform Material material;
uniform vec3 viewPos;
uniform bool useBlinn;


void main()
{
    // green color with height based brightness
    //FragColor = vec4(0.35, 0.65, 0.35, 1.0) * vec4(ourColor, 1.0f);

    //FragColor = texture(terrainTex, texCoord);

    vec3 albedo = texture(terrainTex, fs_in.TexCoord).rgb;

    vec3 LightDir = normalize(light.position - fs_in.WorldPos);
    vec3 Normal = normalize(fs_in.Normal);
    float diff  = max(dot(LightDir, Normal), 0.0);

    vec3 ViewDir = normalize(viewPos - fs_in.WorldPos);

    float spec = 0.0;
    if (useBlinn)
    {
        vec3 HalfwayDir = normalize(LightDir + ViewDir);
        spec = pow(max(dot(Normal, HalfwayDir), 0.0), material.shininess * 4.0);
    }
    else
    {
        vec3 ReflectDir = reflect(-LightDir, Normal);
        spec = pow(max(dot(ViewDir, ReflectDir), 0.0), material.shininess);
    }

    vec3 ambient = light.ambient * material.ambient * albedo; // some light somewhere in the world
    vec3 diffuse = light.diffuse * diff * albedo * material.diffuse; // simulates the directional impact a light object has on an object
    vec3 specular = light.specular * spec * material.specular; // simulates the bright spot of a light that appears on shiny objects

    vec3 color = ambient + diffuse + specular;

    FragColor = vec4(color, 1.0);
}
