#pragma once

#include <string>
#include "height_data.h"
#include <random>

struct MidpointDisplacement
{
    int N;
    float startAmplitude;
    float amplitudeDecline;
    int EdgeLength;
    std::vector<float> heights;
};

class Terrain
{
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
    void ApplySquareStep(int x0, int x1, int y0, int y1, MidpointDisplacement& data);
    void ApplyDiamondStep(int x0, int x1, int y0, int y1, MidpointDisplacement& data);
    void CalculateMidpointDisplacement(MidpointDisplacement& data, int size);

public:

	int m_iSize;

	Terrain() : m_fHeightScale(1.0f), m_iSize(0) {};
	~Terrain();

	virtual void Render () = 0;

	bool GenerateFaultFormation(int size, int iterations, int minDelta, int maxDelta, float filter);
	bool GenerateMidpointDisplacement(int size, int seed, float amplitude, float factor);

	bool LoadHeightMap(const std::string& filename, int iSize);
	bool SaveHeightMap(const std::string& filename);
	bool UnloadHeightMap();

	inline void SetHeightScale(float fScale) { m_fHeightScale = fScale; }
	inline void SetHeightAtPoint(unsigned char ucHeight, int iX, int iZ) { m_heightData.m_Data[(iZ * m_iSize) + iX] = ucHeight; }
	inline unsigned char GetHeightAtPoint(int iX, int iZ) const { return m_heightData.m_Data[(iZ * m_iSize) + iX];  }
	inline float GetScaledHeightAtPoint(int iX, int iZ) const { return m_heightData.m_Data[(iZ * m_iSize) + iX] * m_fHeightScale; }

};
