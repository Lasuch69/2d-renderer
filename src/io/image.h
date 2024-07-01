#ifndef IMAGE_H
#define IMAGE_H

#include <cstddef>
#include <cstdint>

class Image {
private:
	uint32_t m_width = 0;
	uint32_t m_height = 0;
	uint8_t *m_data = nullptr;

public:
	uint32_t width() const;
	uint32_t height() const;
	uint8_t *data() const;
	size_t size() const;

	Image(uint32_t width, uint32_t height, void *data, size_t size);
	~Image();
};

#endif // !IMAGE_H
