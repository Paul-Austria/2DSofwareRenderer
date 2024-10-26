#ifndef COLOR_H
#define COLOR_H

#include "PixelFormat/PixelFormat.h"
#include <cstdint>
#include <cstring>

namespace Renderer2D {

class Color {
public:
    // Constructors for different formats
    Color(uint8_t r, uint8_t g, uint8_t b);                 // RGB24 or BGR24
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);       // ARGB8888
    Color(uint8_t grayscale);                               // GRAYSCALE8
    Color() = default;

    // Get color components
    void GetColor(PixelFormat format, uint8_t* outColor) const;

    // Set color components
    void SetColor(PixelFormat format, const uint8_t* colorData);

    uint8_t GetAlpha();

    PixelFormat GetFormat();

    // Converters
    void ConvertTo(PixelFormat targetFormat, uint8_t* outColor) const;

    alignas(16) uint8_t data[4] = {0, 0, 0, 0}; // Color data storage (max 4 bytes for ARGB8888)

private:
    PixelFormat format;  // The current format of the color data
};

} // namespace Renderer2D

#endif // COLOR_H
