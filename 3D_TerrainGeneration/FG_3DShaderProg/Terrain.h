#pragma once

#include <string>
#include "height_data.h"
#include <random>
#include <array>
#include <stb_image.h>

struct MidpointDisplacement
{
    int N;
    float startAmplitude;
    float amplitudeDecline;
    int EdgeLength;
    std::vector<float> heights;
};

enum ETileType
{
    DIRT,
    GRASS,
    ROCK,
    SNOW_TIP
};

struct TextureRegion
{
    int lowHeight = 0;
    int optimalHeight = 0;
    int highHeight = 0;
};

struct ImageTile
{
    int width = 0;
    int height = 0;
    int channels = 0;
    std::vector<unsigned char> pixels;

    bool IsLoaded() const { return width > 0 && height > 0 && !pixels.empty(); }
};

struct TextureTile
{
    ImageTile image;
    ETileType tileType;
    TextureRegion region;
    bool isEnabled = false;
};

class Terrain
{
private:

    std::array<TextureTile, 4> tiles{};
    int countTiles = 0;
    std::vector<unsigned char> mapPixels;
    int m_textureSize = 0;

protected: 
	
	Height_Data m_heightData;
	float m_fHeightScale;
    std::minstd_rand rng;


	void FilterHeightBand(float* band, int stride, int count, float filter);
	void FilterHeightField(std::vector<float>& heightData, float filter);

	void NormalizeHeight(std::vector<float>& heightData);

    int CalculateLength(int n);
    float GetRandom(float amp);
    float GetOffset(float amp, float decline);
    int GetMidpoint(int a, int b);
    int GetIndex(int x, int y, int size);
    float GetAverageOf2(float a, float b);
    float GetAverageOf4(float a, float b, float c, float d);
    float GetAverageOfCount(float sum, int count);
    bool IsWithinRange(int x, int y, int edge);
    //void ApplySquareStep(int x0, int x1, int y0, int y1, MidpointDisplacement& data);
    void ApplySquareStep(int x, int y, int half, MidpointDisplacement& data);
    void ApplyDiamondStep(int x0, int x1, int y0, int y1, MidpointDisplacement& data);
    void CalculateMidpointDisplacement(MidpointDisplacement& data, int step);

    //texture generation
    float RegionPercent(const TextureTile& tile, float h);
    float InterpolateHeight(int x, int z, float heightToTexRatio);
    bool UploadToGL();
    bool LoadTile(ETileType type, const std::string& path);
    void SetRegion(ETileType type, int low, int opt, int high);


public:

	int m_iSize;
    unsigned int texture = 0;

	Terrain() : m_fHeightScale(1.0f), m_iSize(0) {};
	~Terrain();

	virtual void Render () = 0;

	bool GenerateFaultFormation(int size, int iterations, int minDelta, int maxDelta, float filter);
	bool GenerateMidpointDisplacement(int size, int seed, float amplitude, float factor);
    bool GenerateTextureMap(int textureSize = 4096, float tileRepeat = 8.f);

	bool LoadHeightMap(const std::string& filename, int iSize);
	bool SaveHeightMap(const std::string& filename);
	bool UnloadHeightMap();

	inline void SetHeightScale(float fScale) { m_fHeightScale = fScale; }
	inline void SetHeightAtPoint(unsigned char ucHeight, int iX, int iZ) { m_heightData.m_Data[(iZ * m_iSize) + iX] = ucHeight; }
	inline unsigned char GetHeightAtPoint(int iX, int iZ) const { return m_heightData.m_Data[(iZ * m_iSize) + iX];  }
	inline float GetScaledHeightAtPoint(int iX, int iZ) const { return m_heightData.m_Data[(iZ * m_iSize) + iX] * m_fHeightScale; }

};
