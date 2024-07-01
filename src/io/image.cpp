#include <cassert>
#include <cstdint>
#include <cstdlib>

#include "image.h"

uint32_t Image::width() const {
	return m_width;
}

uint32_t Image::height() const {
	return m_height;
}

uint8_t *Image::data() const {
	return m_data;
}

size_t Image::size() const {
	return m_width * m_height * 4;
}

Image::Image(uint32_t width, uint32_t height, void *data, size_t size) {
	assert(size == width * height * 4);

	m_width = width;
	m_height = height;
	m_data = reinterpret_cast<uint8_t *>(data);
}

Image::~Image() {
	if (m_data == nullptr)
		return;

	free(m_data);
}
