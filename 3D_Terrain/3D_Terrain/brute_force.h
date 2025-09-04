#pragma once
#ifndef BRUTE_FORCE_H
#define BRUTE_FORCE_H

#include <vector>
#include "Terrain.h"
#include <glad/glad.h>
#include <glm/glm.hpp>


struct BFVertex
{
	glm::vec3 pos;
	glm::vec3 color;
};

class Brute_Force: public Terrain
{
public:
	unsigned int VAO = 0;
	unsigned int VBO = 0;
	unsigned int EBO = 0;

	Brute_Force();
	~Brute_Force();

	void Render();
	void BuildTerrain();

private:
	

	size_t indexCount;
};

#endif