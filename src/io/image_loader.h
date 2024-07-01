#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

#include <cstddef>

class Image;

Image *imageLoad(const char *filename);
Image *imageLoadFromMemory(void *buffer, size_t bufferSize);

#endif // !IMAGE_LOADER_H
