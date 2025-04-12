#pragma once
#include <fstream>
#include <vector>
#include "Wad.h"

#pragma pack(push, 1)
struct BMPHeader {
	uint16_t bfType;
	uint32_t bfSize;
	uint16_t bfReserved1;
	uint16_t bfReserved2;
	uint32_t bfOffBits;
};

struct DIBHeader {
	uint32_t biSize;
	int32_t biWidth;
	int32_t biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	int32_t biXPelsPerMeter;
	int32_t biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
};

struct RGBQuad {
	uint8_t b, g, r, reserved;
};
#pragma pack(pop)

void save8BitBMP(const char* filename, const WADTEX& tex);

WADTEX load8BitBMP(const char* filename);