#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

//out vec3 ourPos;
out vec3 ourColor;
out vec2 ourTexCoord;
//uniform float horizontalOffset;
void main()
{
	//gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
	gl_Position = vec4(aPos, 1.0);
	//gl_Position = vec4(aPos.x + horizontalOffset, -aPos.y, aPos.z, 1.0);
	ourColor = aColor;
	//ourPos = aPos;

	ourTexCoord = vec2(aTexCoord.x, aTexCoord.y);
};