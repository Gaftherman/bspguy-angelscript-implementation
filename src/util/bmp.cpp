#include "bmp.h"
#include "util.h"

void save8BitBMP(const char* filename, const WADTEX& tex) {
	if (!tex.data) {
		logf("Failed to save empty BMP file\n");
		return;
	}

	std::ofstream file(filename, std::ios::binary);

	int rowSize = ((tex.nWidth + 3) / 4) * 4; // pad to 4 bytes
	int imageSize = rowSize * tex.nHeight;

	BMPHeader bmp;
	memset(&bmp, 0, sizeof(BMPHeader));
	bmp.bfType = 0x04D42; // 'BM'
	bmp.bfOffBits = 54 + 1024; // header + palette

	DIBHeader dib;
	memset(&dib, 0, sizeof(DIBHeader));
	bmp.bfSize = sizeof(BMPHeader) + sizeof(DIBHeader) + 256 * sizeof(RGBQuad) + imageSize;
	dib.biSize = sizeof(DIBHeader);
	dib.biPlanes = 1;
	dib.biBitCount = 8;
	dib.biClrUsed = 256;
	dib.biWidth = tex.nWidth;
	dib.biHeight = tex.nHeight;
	dib.biSizeImage = imageSize;

	file.write((char*)&bmp, sizeof(bmp));
	file.write((char*)&dib, sizeof(dib));

	COLOR3* palette = tex.getPalette();
	for (int i = 0; i < 256; ++i) {
		COLOR3& src = palette[i];
		RGBQuad dst;
		dst.r = src.r;
		dst.g = src.g;
		dst.b = src.b;
		dst.reserved = 0;
		file.write((char*)&dst, sizeof(RGBQuad));
	}

	uint8_t* data = tex.getMip(0);
	int rowPad = rowSize - tex.nWidth;

	for (int y = tex.nHeight - 1; y >= 0; --y) {
		file.write((char*)&data[y * tex.nWidth], tex.nWidth);

		for (int i = 0; i < rowPad; ++i)
			file.put(0);
	}
}


WADTEX load8BitBMP(const char* filename) {
	WADTEX tex;
	memset(&tex, 0, sizeof(WADTEX));

	std::ifstream file(filename, std::ios::binary);
	if (!file) {
		logf("Failed to open file: %s\n", filename);
		return tex;
	}

	BMPHeader bmp;
	DIBHeader dib;

	file.read((char*)&bmp, sizeof(BMPHeader));
	file.read((char*)&dib, sizeof(DIBHeader));

	if (bmp.bfType != 0x4D42) {
		logf("Invalid BMP file: %s\n", filename);
		return tex;
	}

	if (dib.biBitCount != 8 || dib.biCompression != 0) {
		logf("Only uncompressed 8-bit BMPs can be imported: %s\n", filename);
		return tex;
	}

	std::vector<RGBQuad> palette(256);
	file.read((char*)palette.data(), 256 * sizeof(RGBQuad));

	int rowSize = ((dib.biWidth + 3) / 4) * 4;
	
	tex.nWidth = dib.biWidth;
	tex.nHeight = dib.biHeight;
	tex.data = new uint8_t[tex.getDataSize()];

	// Read pixel data (bottom-up BMP format)
	for (int y = dib.biHeight - 1; y >= 0; --y) {
		file.read((char*)&tex.data[y * dib.biWidth], dib.biWidth);
		file.ignore(rowSize - dib.biWidth);
	}

	tex.generateMips();

	COLOR3* pal = tex.getPalette();
	for (int i = 0; i < 256; i++) {
		COLOR3& dst = pal[i];
		RGBQuad& src = palette[i];
		dst.r = src.r;
		dst.g = src.g;
		dst.b = src.b;
	}

	uint16_t* palCount = (uint16_t*)((uint8_t*)pal - 2);
	*palCount = 256;

	return tex;
}