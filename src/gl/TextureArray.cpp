#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "TextureArray.h"
#include "util.h"
#include "globals.h"
#include "colors.h"

TextureArray::TextureArray() {
	memset(buckets, 0, sizeof(TextureBucket) * TEXARRAY_BUCKET_COUNT);

	uint32_t* ids = new uint32_t[TEXARRAY_BUCKET_COUNT];
	glGenTextures(TEXARRAY_BUCKET_COUNT, ids);

	for (int i = 0; i < TEXARRAY_BUCKET_COUNT; i++) {
		buckets[i].glArrayId = ids[i];
	}

	delete[] ids;
}

TextureArray::~TextureArray() {
	for (int i = 0; i < TEXARRAY_BUCKET_COUNT; i++) {
		if (buckets[i].count) {
			glDeleteTextures(1, &buckets[i].glArrayId);
			delete[] buckets[i].textures;
		}
	}
}

void TextureArray::clear() {
	for (int i = 0; i < TEXARRAY_BUCKET_COUNT; i++) {
		if (buckets[i].textures) {
			delete[] buckets[i].textures;
			buckets[i].textures = NULL;
		}
		buckets[i].count = 0;
	}
	numResize = 0;
}

void TextureArray::getBucketDimensions(int& width, int& height) {
	if (false)
		return;

	const bool aggressiveScaling = false; // reduce quality for more speed

	if (width != height) {
		// try to move this into a square bucket
		if (width % height == 0) {
			height = width;
		}
		else if (width % height == 0) {
			width = height;
		}
	}

	if (width == height && width < 64 && 64 % width == 0) {
		width = height = 64;
	}

	if (aggressiveScaling) {
		if (width < 128 || height < 128) {
			width = 128;
			height = 128;
		}
	}

	return;
}

TexArrayOffset TextureArray::tally(int width, int height) {
	TexArrayOffset offset;
	offset.arrayIdx = 0;
	offset.layer = 0;

	if ((width % 16) != 0 || (height % 16) != 0) {
		logf("Texture array got dimensions not divisble by 16");
		return offset;
	}
	if (width > 1024 || height > 1024 || width < 16 || height < 16) {
		logf("Texture array got invalid texture size\n");
		return offset;
	}

	getBucketDimensions(width, height);

	int bucketX = (width / 16) - 1;
	int bucketY = (height / 16) - 1;
	offset.arrayIdx = bucketY * TEXARRAY_BUCKET_DIM + bucketX;

	TextureBucket& bucket = buckets[offset.arrayIdx];

	bucket.count++;

	offset.layer = bucket.count - 1;
	return offset;
}

void TextureArray::add(Texture* tex) {
	if ((tex->width % 16) != 0 || (tex->height % 16) != 0) {
		logf("Texture array got dimensions not divisble by 16");
		return;
	}
	if (tex->width > 1024 || tex->height > 1024 || tex->width < 16 || tex->height < 16) {
		logf("Texture array got invalid texture size\n");
		return;
	}

	int width = tex->width;
	int height = tex->height;
	getBucketDimensions(width, height);

	if (width != tex->width || height != tex->height) {
		COLOR3* newData = new COLOR3[width * height];

		float xScale = width / (float)tex->width;
		float yScale = height / (float)tex->height;

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				int srcX = x / xScale;
				int srcY = y / yScale;
				newData[y * width + x] = ((COLOR3*)tex->data)[srcY * tex->width + srcX];
			}
		}

		tex->width = width;
		tex->height = height;
		delete[] tex->data;
		tex->data = (uint8_t*)newData;

		numResize++;
	}

	int bucketX = (width / 16) - 1;
	int bucketY = (height / 16) - 1;

	TextureBucket& bucket = buckets[bucketY * TEXARRAY_BUCKET_DIM + bucketX];

	if (!bucket.textures) {
		bucket.textures = new Texture*[1];
		bucket.count = 0; // reset after tally() calls
	}
	else {
		Texture** oldTextures = bucket.textures;
		bucket.textures = new Texture*[bucket.count + 1];
		memcpy(bucket.textures, oldTextures, sizeof(Texture*) * bucket.count);
		delete[] oldTextures;
	}

	bucket.textures[bucket.count] = tex;

	tex->arrayId = bucket.glArrayId;
	tex->layer = bucket.count;
	bucket.count += 1;
}

void TextureArray::upload() {
	int bucketCount = 0;
	int textureCount = 0;
	int texDataSz = 0;

	for (int i = 0; i < TEXARRAY_BUCKET_COUNT; i++) {
		int sizeX = ((i % TEXARRAY_BUCKET_DIM) + 1)*16;
		int sizeY = ((i / TEXARRAY_BUCKET_DIM) + 1)*16;

		if (buckets[i].count) {
			bucketCount++;
			textureCount += buckets[i].count;
			debugf("%d textures in bucket %dx%d\n", buckets[i].count, sizeX, sizeY);

			glBindTexture(GL_TEXTURE_2D_ARRAY, buckets[i].glArrayId);
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, sizeX, sizeY, buckets[i].count, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

			texDataSz += buckets[i].count * sizeX * sizeY * 3;

			if (buckets[i].textures) {
				for (int k = 0; k < buckets[i].count; k++) {
					glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, k, sizeX, sizeY, 1, GL_RGB, GL_UNSIGNED_BYTE, buckets[i].textures[k]->data);

					delete[] buckets[i].textures[k]->data;
					buckets[i].textures[k]->data = NULL;
				}
			}

			if (g_settings.texture_filtering) {
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			else {
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}
			glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
		}
	}

	debugf("uploaded %d textures as %d arrays (%d resized, %.2f MB)\n",
		textureCount, bucketCount, numResize, texDataSz / (1024.0f*1024.0f));
}