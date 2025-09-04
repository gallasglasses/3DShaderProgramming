#pragma once

#include <string>
#include "height_data.h"

class Terrain
{
protected: 
	
	Height_Data m_heightData;
	float m_fHeightScale;


	void FilterHeightBand(float* band, int stride, int count, float filter);
	void FilterHeightField(std::vector<float>& heightData, float filter);
	void NormalizeHeight(std::vector<float>& heightData);

public:

	int m_iSize;

	Terrain() : m_fHeightScale(1.0f), m_iSize(0) {};
	~Terrain();

	virtual void Render () = 0;

	bool GenerateFaultFormation(int size, int iterations, int minDelta, int maxDelta, float filter);

	bool LoadHeightMap(const std::string& filename, int iSize);
	bool SaveHeightMap(const std::string& filename);
	bool UnloadHeightMap();

	inline void SetHeightScale(float fScale) { m_fHeightScale = fScale; }
	inline void SetHeightAtPoint(unsigned char ucHeight, int iX, int iZ) { m_heightData.m_Data[(iZ * m_iSize) + iX] = ucHeight; }
	inline unsigned char GetHeightAtPoint(int iX, int iZ) const { return m_heightData.m_Data[(iZ * m_iSize) + iX];  }
	inline float GetScaledHeightAtPoint(int iX, int iZ) const { return m_heightData.m_Data[(iZ * m_iSize) + iX] * m_fHeightScale; }

};