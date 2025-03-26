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
	this->data = new uint8_t[width * height * sizeof(COLOR4)];
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
		glBindTexture(GL_TEXTURE_3D, id); // Binds this texture handle so we can load the data into it

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Note: GL_CLAMP is significantly slower
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Set up filters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		if (format == GL_RGB)
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		}

		glTexImage3D(GL_TEXTURE_3D, 0, format, width, height, depth, 0, format, GL_UNSIGNED_BYTE, data);
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
			glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		}

		if (format == GL_RGB)
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	}

	uploaded = true;
}

void Texture::bind()
{	
	if (arrayId != -1) {
		// 3D textures break when using any interpolation or mip-mapping.
		// A new shader is needed for bilinear filtering without blending the Z axis.
		glBindTexture(GL_TEXTURE_3D, arrayId);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		return;
	}

	if (is3d) {
		glBindTexture(GL_TEXTURE_3D, id);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
