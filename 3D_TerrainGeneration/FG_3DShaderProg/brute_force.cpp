#include "brute_force.h"
#include <vector>
#include <iostream>
#include <stb_image.h>

Brute_Force::Brute_Force()
{
	//BuildTerrain();
}

Brute_Force::~Brute_Force()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &texture);
}

void Brute_Force::Render()
{
	if (VAO == 0 && m_iSize > 1 && !m_heightData.m_Data.empty()) BuildTerrain();

	//std::cout << "Brute_Force::Render()" << std::endl;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Brute_Force::BuildTerrain()
{
	if(m_iSize <= 1)
	{
		std::cout << "Brute_Force::BuildTerrain() m_iSize <= 1 return;" << std::endl;
		return;
	}

	std::cout << "Brute_Force::BuildTerrain()" << std::endl;
	//unsigned char color;

	std::vector<BFVertex> vertices;
	vertices.reserve(m_iSize * m_iSize);
    float texCoordX, texCoordY;


	for (int z = 0; z < m_iSize; z++)
	{
		for (int x = 0; x < m_iSize; x++)
		{
			//// Height-based coloring. High-points are light, and low points are dark.
			//const auto hColor = GetHeightAtPoint(x, z);
			//glm::vec3 color = glm::vec3(static_cast<float>(hColor) / 255.0f);
			//std::cout << "Brute_Force::BuildTerrain(): color = " << static_cast<float>(hColor) / 255.0f << " | z = " << z << " | x = " << x << std::endl;
            glm::vec3 color = glm::vec3(1.0f);

			glm::vec3 vertex = glm::vec3(static_cast<float>(x), GetScaledHeightAtPoint(x, z), static_cast<float>(z));
			//std::cout << "Brute_Force::BuildTerrain(): vertex x = " << vertex.x << " | y = " << vertex.y << " | z = " << vertex.z << std::endl;

            texCoordX = static_cast<float>(x) / static_cast<float>(m_iSize - 1);
            texCoordY = static_cast<float>(z) / static_cast<float>(m_iSize - 1);
            glm::vec2 texCoord = glm::vec2(texCoordX, texCoordY);

			vertices.push_back({ vertex, color, texCoord});
		}
	}

	std::vector<unsigned int> indices;
	indices.reserve((m_iSize - 1) * (m_iSize - 1) * 6);

	for (int z = 0; z < m_iSize - 1; z++)
	{
		for (int x = 0; x < m_iSize - 1; x++)
		{
			indices.push_back(z * m_iSize + x);
			indices.push_back(z * m_iSize + x + 1);
			indices.push_back((z + 1) * m_iSize + x);
			indices.push_back(z * m_iSize + x + 1);
			indices.push_back((z + 1) * m_iSize + x + 1);
			indices.push_back((z + 1) * m_iSize + x);
		}
	}

	std::cout << "indices size " << indices.size() << std::endl;
	indexCount = indices.size();
	
	if (VAO)
	{
		glDeleteVertexArrays(1, &VAO);
	}
	
	if (VBO)
	{
		glDeleteBuffers(1, &VBO);
	}
	if (EBO)
	{
		glDeleteBuffers(1, &EBO);
	}

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(BFVertex), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BFVertex), (void*)offsetof(BFVertex, pos));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(BFVertex), (void*)offsetof(BFVertex, color));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(BFVertex), (void*)offsetof(BFVertex, texCoord));
	glEnableVertexAttribArray(2);


	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	std::cout << "Built terrain: " << m_iSize << "*" << m_iSize << ", vertices=" << vertices.size() << ", indices=" << indices.size() << std::endl;


    LoadTile(ETileType::DIRT, "T_Dirt.png");
    LoadTile(ETileType::GRASS, "T_Grass.png");
    LoadTile(ETileType::ROCK, "T_Rock.png");
    LoadTile(ETileType::SNOW_TIP, "T_snow_mountain.png");

    /*LoadTile(ETileType::DIRT, "T_Dirt_10.png");
    LoadTile(ETileType::GRASS, "T_Grass_10.png");
    LoadTile(ETileType::ROCK, "T_Rock_10.png");
    LoadTile(ETileType::SNOW_TIP, "T_snow_mountain_10.png");*/

    SetRegion(ETileType::DIRT, 0, 40, 80);
    SetRegion(ETileType::GRASS, 50, 90, 130);
    SetRegion(ETileType::ROCK, 120, 160, 200);
    SetRegion(ETileType::SNOW_TIP, 180, 220, 255);

    if(!GenerateTextureMap(4096, 8.0f))
    //if(!GenerateTextureMap(10, 8.0f))
    {
        std::cout << "Brute_Force::GenerateTextureMap() failed" << std::endl;
        return;
    }

    //SetRegion(1, lowG, optG, highG);

    /*if (texture)
    {
        glDeleteTextures(1, &texture);
    }
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;

    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load("T_Grass.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);*/
}
