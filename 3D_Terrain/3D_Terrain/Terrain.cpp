#include "Terrain.h"
#include <iostream>
#include <fstream>
#include <sstream>

void Terrain::FilterHeightBand(float* band, int stride, int count, float filter)
{
    float heightValue = band[0];
    int j = stride;
    int i;

    for (i = 0; i < count - 1; i++)
    {
        band[j] = filter * heightValue + (1 - filter) * band[j];

        heightValue = band[j];
        j += stride;
    }
}

void Terrain::FilterHeightField(std::vector<float>& heightData, float filter)
{
    int i = 0;

    for (; i < m_iSize; i++)
        FilterHeightBand(&heightData[m_iSize * i], 1, m_iSize, filter);

    //erode right to left
    for (i = 0; i < m_iSize; i++)
        FilterHeightBand(&heightData[m_iSize * i + m_iSize - 1], -1, m_iSize, filter);

    //erode top to bottom
    for (i = 0; i < m_iSize; i++)
        FilterHeightBand(&heightData[i], m_iSize, m_iSize, filter);

    //erode from bottom to top
    for (i = 0; i < m_iSize; i++)
        FilterHeightBand(&heightData[m_iSize * (m_iSize - 1) + i], -m_iSize, m_iSize, filter);
}

void Terrain::NormalizeHeight(std::vector<float>& heightData)
{
    float fMin, fMax;
    float fHeight;
    int i;

    fMin = heightData[0];
    fMax = heightData[0];

    for (i = 1; i < m_iSize * m_iSize; i++)
    {
        if (heightData[i] > fMax)
        {
            fMax = heightData[i];
        }
        else if (heightData[i] < fMin)
        {
            fMin = heightData[i];
        }
    }

    if (fMax <= fMin) return;

    fHeight = fMax - fMin;

    for (i = 0; i < m_iSize * m_iSize; i++)
    {
        heightData[i] = ((heightData[i] - fMin) / fHeight) * 255.0f;
    }
}

Terrain::~Terrain()
{
    UnloadHeightMap();
}


bool Terrain::GenerateFaultFormation(int size, int iterations, int minDelta, int maxDelta, float filter)
{
    m_heightData.m_iSize = size;
    m_iSize = size;
    m_heightData.m_Data.resize(m_iSize * m_iSize);
    std::vector<float> heights(m_iSize * m_iSize, 0.f);

    int currentIteration;
    int randX1, randX2, randZ1, randZ2;

    for (currentIteration = 0; currentIteration < iterations; currentIteration++)
    {
        float height = maxDelta - ((maxDelta - minDelta) * currentIteration) / iterations;

        randX1 = rand() % m_iSize;
        randZ1 = rand() % m_iSize;

        do
        {
            randX2 = rand() % m_iSize;
            randZ2 = rand() % m_iSize;
        } while (randX2 == randX1 && randZ2 == randZ1);

        int dirX1 = randX2 - randX1;
        int dirZ1 = randZ2 - randZ1;

        for (int z = 0; z < m_iSize; z++)
        {
            for (int x = 0; x < m_iSize; x++)
            {
                int dirX2 = x - randX1;
                int dirZ2 = z - randZ1;

                if ((dirX2 * dirZ1 - dirX1 * dirZ2) > 0)
                {
                    heights[(z*m_iSize) + x] += height;
                }
            }
        }

        FilterHeightField(heights, filter);
    }
    NormalizeHeight(heights);

    for (int z = 0; z < m_iSize; z++)
    {
        for (int x = 0; x < m_iSize; x++)
        {
            SetHeightAtPoint(static_cast<unsigned char>(heights[(z * m_iSize) + x]), x, z);
        }
    }

    if(!heights.empty())
    {
        heights.clear();
    }

    return true;
}

bool Terrain::LoadHeightMap(const std::string& filename, int iSize)
{
    if(iSize <= 0) return false;

    if(!m_heightData.m_Data.empty()) UnloadHeightMap();

    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if(!file)
    {
        std::cout << "ERROR::TERRAIN::FAILED_TO_OPEN " << filename << std::endl;
        return false;
    }

    const std::streampos size = file.tellg();
    if (size < 0)
    {
        std::cout << "ERROR::TERRAIN::FAILED " << filename << std::endl;
        return false;
    }
    if (size != iSize * iSize)
    {
        std::cout << "ERROR::TERRAIN::FAILED " << filename << std::endl;
        return false;
    }

    file.seekg(0, std::ios::beg);
    m_heightData.m_Data.resize(iSize* iSize);

    std::cout << "m_heightData.m_Data size " << m_heightData.m_Data.size() << std::endl;

    if (!file.read(reinterpret_cast<char*>(m_heightData.m_Data.data()), size))
    {
        std::cout << "ERROR::TERRAIN::FAILED_TO_OPEN " << filename << std::endl;
        UnloadHeightMap();
        return false;
    }
    file.close();

    m_heightData.m_iSize = iSize;
    m_iSize = iSize;
    std::cout << "m_iSize = " << m_iSize << std::endl;

    return true;
}

bool Terrain::SaveHeightMap(const std::string& filename)
{
    if (m_iSize <= 0) return false;

    if (m_iSize * m_iSize != m_heightData.m_Data.size())
    {
        std::cout << "ERROR::TERRAIN::SIZE_MISMATCH " << std::endl;
        UnloadHeightMap();
        return false;
    }

    std::ofstream file(filename, std::ios::binary | std::ios::trunc);

    if (!file)
    {
        std::cout << "ERROR::TERRAIN::FAILED_TO_CREATE " << filename << std::endl;
        return false;
    }

    if (!file.write(reinterpret_cast<char*>(m_heightData.m_Data.data()), m_heightData.m_Data.size()))
    {
        std::cout << "ERROR::TERRAIN::FAILED_TO_OPEN " << filename << std::endl;
        UnloadHeightMap();
        return false;
    }

    return true;
}

bool Terrain::UnloadHeightMap()
{
    std::cout << "UnloadHeightMap" << std::endl;
    m_heightData.m_Data.clear();
    m_heightData.m_iSize = 0;
    m_iSize = 0;

    return true;
}
