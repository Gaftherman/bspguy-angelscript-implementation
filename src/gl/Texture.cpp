#include <GL/glew.h>
#include "colors.h"
#include "Texture.h"
#include "globals.h"

Texture::Texture(int width, int height) {
	this->width = width;
	this->height = height;
	this->depth = 1;
	this->nearFilter = GL_LINEAR;
	this->farFilter = GL_LINEAR_MIPMAP_LINEAR;
	this->data = new uint8_t[width*height*sizeof(COLOR4)];
	arrayId = -1;
	layer = 0;
	is3d = false;
}

Texture::Texture(int width, int height, int depth) {
	this->width = width;
	this->height = height;
	this->depth = depth;
	this->nearFilter = GL_LINEAR;
	this->farFilter = GL_LINEAR_MIPMAP_LINEAR;
	this->data = new uint8_t[width * height * depth * sizeof(COLOR4)];
	arrayId = -1;
	layer = 0;
	is3d = true;
}

Texture::Texture( int width, int height, void * data )
{
	this->width = width;
	this->height = height;
	this->depth = 1;
	this->nearFilter = GL_LINEAR;
	this->farFilter = GL_LINEAR_MIPMAP_LINEAR;
	this->data = (uint8_t*)data;
	arrayId = -1;
	layer = 0;
	is3d = false;
}

Texture::~Texture()
{
	if (uploaded)
		glDeleteTextures(1, &id);
	if (data)
		delete[] data;
}

void Texture::upload(int format, bool lightmap)
{
	if (!data) {
		return;
	}
	this->isLightmap = lightmap;
	if (uploaded) {
		glDeleteTextures(1, &id);
	}
	glGenTextures(1, &id);

	if (is3d) {
		int glParam3d = g_opengl_texture_array_support ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_3D;

		glBindTexture(glParam3d, id); // Binds this texture handle so we can load the data into it

		glTexParameteri(glParam3d, GL_TEXTURE_WRAP_S, GL_REPEAT); // Note: GL_CLAMP is significantly slower
		glTexParameteri(glParam3d, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Set up filters
		glTexParameteri(glParam3d, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(glParam3d, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		if (format == GL_RGB)
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		}

		glTexImage3D(glParam3d, 0, format, width, height, depth, 0, format, GL_UNSIGNED_BYTE, data);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, id); // Binds this texture handle so we can load the data into it

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Note: GL_CLAMP is significantly slower
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Set up filters
		if (lightmap)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

		if (format == GL_RGB)
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

		if (!lightmap && width % 16 == 0 && height % 16 == 0 && format == GL_RGBA) {
			const int mipLevels = 3;
			COLOR4* texdata = (COLOR4*)data;

			for (int m = 1; m <= mipLevels; m++) {
				int mipWidth = width >> m;
				int mipHeight = height >> m;
				int scale = 1 << m;

				COLOR4* mipData = new COLOR4[mipWidth * mipHeight];
				for (int y = 0; y < mipHeight; y++) {
					for (int x = 0; x < mipWidth; x++) {
						int srcX = x * scale;
						int srcY = y * scale;
						mipData[y * mipWidth + x] = texdata[y * width * scale + x * scale];
					}
				}

				glTexImage2D(GL_TEXTURE_2D, m, format, mipWidth, mipHeight, 0, format, GL_UNSIGNED_BYTE, mipData);
				delete[] mipData;
			}

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipLevels);
		}
		else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		}
	}

	uploaded = true;
}

void Texture::bind()
{	
	int glParam3d = g_opengl_texture_array_support ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_3D;
	int glFilter3d = g_opengl_texture_array_support ? GL_NEAREST_MIPMAP_LINEAR : GL_NEAREST;

	if (arrayId != -1) {
		// 3D textures break when using any interpolation or mip-mapping.
		// A new shader is needed for bilinear filtering without blending the Z axis.
		glBindTexture(glParam3d, arrayId);
		glTexParameteri(glParam3d, GL_TEXTURE_MIN_FILTER, glFilter3d);
		glTexParameteri(glParam3d, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		return;
	}

	if (is3d) {
		glBindTexture(glParam3d, id);
		glTexParameteri(glParam3d, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(glParam3d, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		return;
	}

	glBindTexture(GL_TEXTURE_2D, id);

	if (isLightmap) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
}
