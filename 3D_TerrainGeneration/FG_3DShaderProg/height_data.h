#pragma once
#ifndef HEIGHT_DATA_H
#define HEIGHT_DATA_H

#include <vector>

struct Height_Data
{
	////m_ - member
	////p - pointer
	////uc - unsigned char
	//unsigned char* m_pucData; //the height data
	//int m_iSize; //the height size (power of 2)

	std::vector<std::uint8_t> m_Data;
	int m_iSize;
};

#endif