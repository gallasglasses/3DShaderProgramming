#include "Terrain.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>

#include <glm/glm.hpp>
#include <glad/glad.h>

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

float Terrain::RegionPercent(const TextureTile& tile, float h)
{
    /*if (tile.tileType == ETileType::DIRT && h < tile.region.optimalHeight)
    {
        return 1.f;
    }
    else if (tile.tileType == ETileType::GRASS && h < tile.region.optimalHeight)
    {
        return 1.f;
    }
    else if (tile.tileType == ETileType::ROCK && h < tile.region.optimalHeight)
    {
        return 1.f;
    }
    else if (tile.tileType == ETileType::SNOW_TIP && h < tile.region.optimalHeight)
    {
        return 1.f;
    }*/

    if (h <= tile.region.lowHeight && tile.region.lowHeight <= 0)
    {
        return 1.f;
    }
    else if (h >= tile.region.highHeight && tile.region.highHeight >= 255)
    {
        return 1.f;
    }
    if (h <= tile.region.lowHeight)
    {
        return 0.f;
    }
    else if (h >= tile.region.highHeight)
    {
        return 0.f;
    }

    if (h < tile.region.optimalHeight)
    {
        float num1 = static_cast<float>(h - tile.region.lowHeight);
        float num2 = static_cast<float>(tile.region.optimalHeight - tile.region.lowHeight);
        return num1 / num2;
    }
    else if (h == tile.region.optimalHeight)
    {
        return 1.f;
    }
    else if(h > tile.region.optimalHeight)
    {
        float num = static_cast<float>(tile.region.highHeight - tile.region.optimalHeight);
            return static_cast<float>(tile.region.highHeight - h) / num;
    }

    return 0.f;
}

float Terrain::InterpolateHeight(int x, int z, float heightToTexRatio)
{
    float scaledX = x * heightToTexRatio;
    float scaledZ = z * heightToTexRatio;

    int x0 = static_cast<int>(std::floor(scaledX));
    int z0 = static_cast<int>(std::floor(scaledZ));

    float x1, z1;
    x1 = (x0 + 1 < m_iSize) ? (x0 + 1) : x0;
    z1 = (z0 + 1 < m_iSize) ? (z0 + 1) : z0;

    float fx = scaledX - x0; // divide by (x1 - x0) = 1
    float fy = scaledZ - z0; // divide by (z1 - z0) = 1

    float h00 = GetHeightAtPoint(x0, z0);
    float h10 = GetHeightAtPoint(x1, z0);
    float h01 = GetHeightAtPoint(x0, z1);
    float h11 = GetHeightAtPoint(x1, z1);

    float hx0 = h00 * (1.0f - fx) + h10 * fx;
    float hx1 = h01 * (1.0f - fx) + h11 * fx;
    float interpolation = hx0 * (1.0f - fy) + hx1 * fy;

    //same thing just like from Wikipedia
    /*float w00 = ((x1 - scaledX) * (z1 - scaledZ)) / ((x1 - x0) * (z1 - z0));
    float w01 = ((x1 - scaledX) * (scaledZ - z0)) / ((x1 - x0) * (z1 - z0));
    float w10 = ((scaledX - x0) * (z1 - scaledZ)) / ((x1 - x0) * (z1 - z0));
    float w11 = ((scaledX - x0) * (scaledZ - z0)) / ((x1 - x0) * (z1 - z0));

    float interpolation = GetHeightAtPoint(x0, z0) * w00 + GetHeightAtPoint(x1, z0) * w01 +
        GetHeightAtPoint(x0, z1) * w10 + GetHeightAtPoint(x1, z1) * w11;*/

    return interpolation;

    /*unsigned char low, highX, highZ;
    float interpX, interpZ;

    float scaledX = x * heightToTexRatio;
    float scaledZ = z * heightToTexRatio;
    float interpolation;

    low = GetHeightAtPoint(static_cast<int>(scaledX), static_cast<int>(scaledZ));
    if(scaledX + 1 > m_iSize - 1)
        return low;
    else
        highX = GetHeightAtPoint(static_cast<int>(scaledX + 1), static_cast<int>(scaledZ));

    interpolation = scaledX - static_cast<int>(scaledX);
    interpX = (highX - low) * interpolation + low; //(percentage - percentage increase) * difference + minimum value

    if (scaledZ + 1 > m_iSize - 1)
        return low;
    else
        highZ = GetHeightAtPoint(static_cast<int>(scaledX), static_cast<int>(scaledZ + 1));

    interpolation = scaledZ - static_cast<int>(scaledZ);
    interpZ = (highZ - low) * interpolation + low;

    return (interpX + interpZ) / 2;*/
}

bool Terrain::UploadToGL()
{
    if(mapPixels.empty() || m_textureSize <=0 ) return false;

    if (texture)
    {
        glDeleteTextures(1, &texture);
    }
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, m_textureSize, m_textureSize, 0, GL_RGB, GL_UNSIGNED_BYTE, mapPixels.data()); //GL_SRGB8
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

bool Terrain::LoadTile(ETileType type, const std::string& path)
{
    ImageTile imageTile;

    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels = 0;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (!data)
    {
        std::cout << "Failed to load tile '" << path << "'" << std::endl;
        return false;
    }

    imageTile.width = width;
    imageTile.height = height;
    imageTile.channels = nrChannels;
    std::cout << "nrChannels '" << nrChannels << "'" << std::endl;
    imageTile.pixels.assign(data, data + width * height * nrChannels);

    stbi_image_free(data);

    tiles[type].image = std::move(imageTile);
    tiles[type].isEnabled = true;
    tiles[type].tileType = type;

    countTiles = 0;

    for (const TextureTile& tile : tiles)
    {
        if (tile.isEnabled && tile.image.IsLoaded())
            countTiles++;
    }
    return true;
}

void Terrain::SetRegion(ETileType type, int low, int opt, int high)
{
    int nlow = std::max(0, std::min(255, low));
    std::cout << "nlow '" << nlow << "'" << std::endl;
    int nopt = std::max(0, std::min(255, opt));
    std::cout << "nopt '" << nopt << "'" << std::endl;
    int nhigh = std::max(0, std::min(255, high));
    std::cout << "nhigh '" << nhigh << "'" << std::endl;

    tiles[type].region = { nlow, nopt, nhigh };
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

bool Terrain::GenerateTextureMap(int textureSize, float tileRepeat)
{
    if (m_iSize <= 0) return false;
    mapPixels.clear();

    countTiles = 0;
    for (const TextureTile& tile : tiles)
    {
        if(tile.isEnabled && tile.image.IsLoaded())
            countTiles++;
    }

    if(countTiles == 0)
        return false;
    m_textureSize = textureSize;
    mapPixels.assign(m_textureSize * m_textureSize * 3, 0);


    const float mapRatio = static_cast<float>(m_iSize - 1) / static_cast<float>(m_textureSize - 1);

    for (int z = 0; z < m_textureSize; z++)
    {
        for (int x = 0; x < m_textureSize; x++)
        {
            float totalRed = 0.f;
            float totalGreen = 0.f;
            float totalBlue = 0.f;
            float totalBlend = 0.f;

            float interpHeight = std::clamp(InterpolateHeight(x, z, mapRatio), 0.f, 255.f);
            //float interpHeight = InterpolateHeight(x, z, mapRatio);

            for (const TextureTile& tile : tiles)
            {
                if (tile.isEnabled && tile.image.IsLoaded())
                {
                    float blend = RegionPercent(tile, interpHeight);
                    
                    //if (blend <= 0.f) continue;

                    float fx = x * tileRepeat;
                    float fz = z * tileRepeat;

                    unsigned tx = (unsigned)fx % (unsigned)tile.image.width;
                    unsigned ty = (unsigned)fz % (unsigned)tile.image.height;

                    //get color
                    size_t idx = static_cast<size_t>(ty * tile.image.width + tx) * tile.image.channels;
                    unsigned char r = tile.image.pixels[idx + 0];
                    unsigned char g = tile.image.pixels[idx + 1];
                    unsigned char b = tile.image.pixels[idx + 2];

                    totalRed += blend * (float)r;
                    totalGreen += blend * (float)g;
                    totalBlue += blend * (float)b;
                    //std::cout << "interpHeight = " << interpHeight << " | Blend = " << blend << " | r " << (float)r << " | g " << (float)g << " | b " << (float)b << " | z " << z << " | x " << x << " | idx " << idx << std::endl;

                    totalBlend += blend;
                }
            }

            if (totalBlend > 0.f)
            {
                totalRed /= totalBlend;
                totalGreen /= totalBlend;
                totalBlue /= totalBlend;
            }

            totalRed = std::clamp(totalRed, 0.f, 255.f);
            totalGreen = std::clamp(totalGreen, 0.f, 255.f);
            totalBlue = std::clamp(totalBlue, 0.f, 255.f);

            size_t index = static_cast<size_t>(z * textureSize + x) * 3;
            mapPixels[index + 0] = static_cast<unsigned char>(totalRed);
            mapPixels[index + 1] = static_cast<unsigned char>(totalGreen);
            mapPixels[index + 2] = static_cast<unsigned char>(totalBlue);
        }
    }

    return UploadToGL();
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
