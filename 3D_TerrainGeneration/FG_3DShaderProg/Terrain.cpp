#include "Terrain.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>

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

    for (i = 0; i < m_iSize; i++)
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

int Terrain::CalculateLength(int n)
{
    return static_cast<int>(std::pow(2, n) + 1);
}

float Terrain::GetRandom(float amp)
{
    std::uniform_real_distribution<float> dis(-amp, amp);
    return dis(rng);
}

float Terrain::GetOffset(float amp, float decline)
{
    return GetRandom(amp) * decline;
}

int Terrain::GetMidpoint(int a, int b)
{
    return a + (b - a) / 2;
}

int Terrain::GetIndex(int x, int y, int size)
{
    return y * size + x;
}

float Terrain::GetAverageOf2(float a, float b)
{
    return (a + b) / 2.0f;
}

float Terrain::GetAverageOf4(float a, float b, float c, float d)
{
    return (a + b + c + d) / 4.0f;
}

float Terrain::GetAverageOfCount(float sum, int count)
{
    if (count > 0)
    {
        return sum / static_cast<float>(count);
    }
    else
        return 0.f;
}

bool Terrain::IsWithinRange(int x, int y, int edge)
{
    if (x >= 0 && y >= 0 && x < edge && y < edge)
        return true;
    else
        return false;
}

//void Terrain::ApplySquareStep(int x0, int x1, int y0, int y1, MidpointDisplacement& data)
void Terrain::ApplySquareStep(int x, int y, int half, MidpointDisplacement& data)
{
    float sum = 0.0f;
    int count = 0;

    if (IsWithinRange(x, y + half, data.EdgeLength))
    {
        sum += data.heights[GetIndex(x, y + half, data.EdgeLength)]; // bottom
        count++;
    }
    if (IsWithinRange(x, y - half, data.EdgeLength))
    {
        sum += data.heights[GetIndex(x, y - half, data.EdgeLength)]; // top
        count++;
    }
    if (IsWithinRange(x - half, y, data.EdgeLength))
    {
        sum += data.heights[GetIndex(x - half, y, data.EdgeLength)]; // left
        count++;
    }
    if (IsWithinRange(x + half, y, data.EdgeLength))
    {
        sum += data.heights[GetIndex(x + half, y, data.EdgeLength)]; // right
        count++;
    }

    data.heights[GetIndex(x, y, data.EdgeLength)] = GetAverageOfCount(sum, count) + GetRandom(data.startAmplitude);

    //int cx = GetMidpoint(x0, x1);
    //int cy = GetMidpoint(y0, y1);

    //float bottom, top, left, right;

    //data.heights[GetIndex(cx, y0, data.EdgeLength)] = GetAverageOf2(data.heights[GetIndex(x0, y0, data.EdgeLength)], data.heights[GetIndex(x1, y0, data.EdgeLength)]) + GetRandom(data.startAmplitude); //GetOffset(data.startAmplitude, data.amplitudeDecline) 
    //bottom = data.heights[GetIndex(cx, y0, data.EdgeLength)];

    //data.heights[GetIndex(cx, y1, data.EdgeLength)] = GetAverageOf2(data.heights[GetIndex(x0, y1, data.EdgeLength)], data.heights[GetIndex(x1, y1, data.EdgeLength)]) + GetRandom(data.startAmplitude);
    //top = data.heights[GetIndex(cx, y1, data.EdgeLength)];

    //data.heights[GetIndex(x0, cy, data.EdgeLength)] = GetAverageOf2(data.heights[GetIndex(x0, y0, data.EdgeLength)], data.heights[GetIndex(x0, y1, data.EdgeLength)]) + GetRandom(data.startAmplitude);
    //left = data.heights[GetIndex(x0, cy, data.EdgeLength)];

    //data.heights[GetIndex(x1, cy, data.EdgeLength)] = GetAverageOf2(data.heights[GetIndex(x1, y0, data.EdgeLength)], data.heights[GetIndex(x1, y1, data.EdgeLength)]) + GetRandom(data.startAmplitude);
    //right = data.heights[GetIndex(x1, cy, data.EdgeLength)];
}

void Terrain::ApplyDiamondStep(int x0, int x1, int y0, int y1, MidpointDisplacement& data)
{
    int cx = GetMidpoint(x0, x1);
    int cy = GetMidpoint(y0, y1);

    float a = data.heights[GetIndex(x0, y0, data.EdgeLength)];
    float b = data.heights[GetIndex(x1, y0, data.EdgeLength)];
    float c = data.heights[GetIndex(x0, y1, data.EdgeLength)];
    float d = data.heights[GetIndex(x1, y1, data.EdgeLength)];

    data.heights[GetIndex(cx, cy, data.EdgeLength)] = GetAverageOf4(a, b, c, d) + GetRandom(data.startAmplitude);
}

void Terrain::CalculateMidpointDisplacement(MidpointDisplacement& data, int step)
{
    if (step < 2) return;
    int half = step / 2;

    for (int y = 0; y < data.EdgeLength - 1; y += step)
    {
        for (int x = 0; x < data.EdgeLength - 1; x += step)
        {
            ApplyDiamondStep(x, x + step, y, y + step, data);
        }
    }

    for (int y = 0; y < data.EdgeLength; y += half)
    {
        int row = y / half;
        int xStart = (row % 2 == 0) ? half : 0;
        for (int x = xStart; x < data.EdgeLength; x += step)
        {
            ApplySquareStep(x, y, half, data);
        }
    }

    data.startAmplitude *= data.amplitudeDecline;
    CalculateMidpointDisplacement(data, step / 2);
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
        float height = maxDelta - static_cast<float>((maxDelta - minDelta) * currentIteration) / static_cast<float>(iterations);

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

bool Terrain::GenerateMidpointDisplacement(int size, int seed, float amplitude, float factor)
{
    /*int N = n;
    float Spread;
    float spreadReductionRate;
    int edgeLength;*/

    MidpointDisplacement algorithm;
    algorithm.N = size;
    algorithm.startAmplitude = amplitude;
    algorithm.amplitudeDecline = std::pow(2.0f, -factor);
    algorithm.EdgeLength = CalculateLength(size);

    //edgeLength = CalculateLength(n);

    rng.seed(seed);

    m_heightData.m_iSize = algorithm.EdgeLength;
    m_iSize = algorithm.EdgeLength;
    m_heightData.m_Data.resize(m_iSize * m_iSize);
    //std::vector<float> heights(m_iSize * m_iSize, 0.f);
    algorithm.heights.assign(m_iSize * m_iSize, 0.f);

    //corner step
    algorithm.heights[0] = GetRandom(algorithm.startAmplitude);
    algorithm.heights[GetIndex(algorithm.EdgeLength - 1, 0, algorithm.EdgeLength)] = GetRandom(algorithm.startAmplitude);
    algorithm.heights[GetIndex(0, algorithm.EdgeLength - 1, algorithm.EdgeLength)] = GetRandom(algorithm.startAmplitude);
    algorithm.heights[GetIndex(algorithm.EdgeLength - 1, algorithm.EdgeLength - 1, algorithm.EdgeLength)] = GetRandom(algorithm.startAmplitude);

    //square step (average of corners + rando value) -> diamond step -> square step -> diamond step...
    CalculateMidpointDisplacement(algorithm, algorithm.EdgeLength - 1);

    NormalizeHeight(algorithm.heights);

    for (int z = 0; z < m_iSize; z++)
    {
        for (int x = 0; x < m_iSize; x++)
        {
            SetHeightAtPoint(static_cast<unsigned char>(algorithm.heights[(z * m_iSize) + x]), x, z);
        }
    }

    if (!algorithm.heights.empty())
    {
        algorithm.heights.clear();
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
