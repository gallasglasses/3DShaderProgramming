#include "brute_force.h"
#include <vector>
#include <iostream>

Brute_Force::Brute_Force()
{
	//BuildTerrain();
}

Brute_Force::~Brute_Force()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

void Brute_Force::Render()
{
	if (VAO == 0 && m_iSize > 1 && !m_heightData.m_Data.empty()) BuildTerrain();

	//std::cout << "Brute_Force::Render()" << std::endl;
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

	for (int z = 0; z < m_iSize; z++)
	{
		for (int x = 0; x < m_iSize; x++)
		{
			// Height-based coloring. High-points are light, and low points are dark.
			const auto hColor = GetHeightAtPoint(x, z);
			glm::vec3 color = glm::vec3(static_cast<float>(hColor) / 255.0f);
			std::cout << "Brute_Force::BuildTerrain(): color = " << static_cast<float>(hColor) / 255.0f << " | z = " << z << " | x = " << x << std::endl;

			glm::vec3 vertex = glm::vec3(static_cast<float>(x), GetScaledHeightAtPoint(x, z), static_cast<float>(z));
			std::cout << "Brute_Force::BuildTerrain(): vertex x = " << vertex.x << " | y = " << vertex.y << " | z = " << vertex.z << std::endl;

			vertices.push_back({ vertex, color});
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


	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	std::cout << "Built terrain: " << m_iSize << "*" << m_iSize << ", vertices=" << vertices.size() << ", indices=" << indices.size() << std::endl;
}
