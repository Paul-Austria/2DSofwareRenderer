#include "../PixelConverter.h"

using namespace Renderer2D;

void PixelConverter::RGB24ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i * 4 + 1] = src[i * 3 + 0]; // R
        dst[i * 4 + 2] = src[i * 3 + 1]; // G
        dst[i * 4 + 3] = src[i * 3 + 2]; // B
        dst[i * 4 + 0] = 255;            // A
    }
}

void PixelConverter::ARGB8888ToRGB24(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i * 3 + 0] = src[i * 4 + 1]; // R
        dst[i * 3 + 1] = src[i * 4 + 2]; // G
        dst[i * 3 + 2] = src[i * 4 + 3]; // B
    }
}

void PixelConverter::RGB565ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint16_t pixel = reinterpret_cast<const uint16_t *>(src)[i];

        // Extract and scale the red channel (5 bits to 8 bits)
        dst[i * 4 + 1] = (pixel & 0xF800) >> 8;       // R (5 bits -> 8 bits)
        dst[i * 4 + 1] = (dst[i * 4 + 1] * 255) / 31; // Scale to 0-255

        // Extract and scale the green channel (6 bits to 8 bits)
        dst[i * 4 + 2] = (pixel & 0x07E0) >> 3;       // G (6 bits -> 8 bits)
        dst[i * 4 + 2] = (dst[i * 4 + 2] * 255) / 63; // Scale to 0-255

        // Extract and scale the blue channel (5 bits to 8 bits)
        dst[i * 4 + 3] = (pixel & 0x001F) << 3;       // B (5 bits -> 8 bits)
        dst[i * 4 + 3] = (dst[i * 4 + 3] * 255) / 31; // Scale to 0-255

        // Set the alpha channel to 255 (fully opaque)
        dst[i * 4 + 0] = 255; // A
    }
}

void PixelConverter::ARGB8888ToRGB565(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint16_t r = (src[i * 4 + 1] >> 3) & 0x1F;
        uint16_t g = (src[i * 4 + 2] >> 2) & 0x3F;
        uint16_t b = (src[i * 4 + 3] >> 3) & 0x1F;
        reinterpret_cast<uint16_t *>(dst)[i] = (r << 11) | (g << 5) | b;
    }
}

void PixelConverter::RGBA1555ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint16_t pixel = reinterpret_cast<const uint16_t *>(src)[i];
        dst[i * 4 + 1] = (pixel & 0x7C00) >> 7;      // R
        dst[i * 4 + 2] = (pixel & 0x03E0) >> 2;      // G
        dst[i * 4 + 3] = (pixel & 0x001F) << 3;      // B
        dst[i * 4 + 0] = (pixel & 0x8000) ? 255 : 0; // A
    }
}

void PixelConverter::RGB24ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i * 4 + 0] = src[i * 3 + 0]; // R
        dst[i * 4 + 1] = src[i * 3 + 1]; // G
        dst[i * 4 + 2] = src[i * 3 + 2]; // B
        dst[i * 4 + 3] = 255;            // A
    }
}

void PixelConverter::RGB565ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint16_t pixel = reinterpret_cast<const uint16_t *>(src)[i];
        dst[i * 4 + 0] = (pixel & 0xF800) >> 8; // R
        dst[i * 4 + 1] = (pixel & 0x07E0) >> 3; // G
        dst[i * 4 + 2] = (pixel & 0x001F) << 3; // B
        dst[i * 4 + 3] = 255;                   // A
    }
}

void PixelConverter::RGBA1555ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint16_t pixel = reinterpret_cast<const uint16_t *>(src)[i];
        dst[i * 4 + 0] = (pixel & 0x7C00) >> 7;      // R
        dst[i * 4 + 1] = (pixel & 0x03E0) >> 2;      // G
        dst[i * 4 + 2] = (pixel & 0x001F) << 3;      // B
        dst[i * 4 + 3] = (pixel & 0x8000) ? 255 : 0; // A
    }
}

void PixelConverter::RGBA4444ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint16_t pixel = reinterpret_cast<const uint16_t *>(src)[i];
        dst[i * 4 + 0] = (pixel & 0xF000) >> 8; // R
        dst[i * 4 + 1] = (pixel & 0x0F00) >> 4; // G
        dst[i * 4 + 2] = (pixel & 0x00F0);      // B
        dst[i * 4 + 3] = (pixel & 0x000F) << 4; // A
    }
}

void PixelConverter::Grayscale8ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i * 4 + 0] = src[i]; // R
        dst[i * 4 + 1] = src[i]; // G
        dst[i * 4 + 2] = src[i]; // B
        dst[i * 4 + 3] = 255;    // A
    }
}

void PixelConverter::ARGB8888ToRGBA1555(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint16_t r = (src[i * 4 + 1] >> 3) & 0x1F;
        uint16_t g = (src[i * 4 + 2] >> 3) & 0x1F;
        uint16_t b = (src[i * 4 + 3] >> 3) & 0x1F;
        uint16_t a = (src[i * 4 + 0] >= 128) ? 0x8000 : 0;
        reinterpret_cast<uint16_t *>(dst)[i] = a | (r << 10) | (g << 5) | b;
    }
}

void PixelConverter::RGBA4444ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint16_t pixel = reinterpret_cast<const uint16_t *>(src)[i];
        dst[i * 4 + 1] = (pixel & 0xF000) >> 8; // R
        dst[i * 4 + 2] = (pixel & 0x0F00) >> 4; // G
        dst[i * 4 + 3] = (pixel & 0x00F0);      // B
        dst[i * 4 + 0] = (pixel & 0x000F) << 4; // A
    }
}

void PixelConverter::ARGB8888ToRGBA4444(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint16_t r = (src[i * 4 + 1] >> 4) & 0x0F;
        uint16_t g = (src[i * 4 + 2] >> 4) & 0x0F;
        uint16_t b = (src[i * 4 + 3] >> 4) & 0x0F;
        uint16_t a = (src[i * 4 + 0] >> 4) & 0x0F;
        reinterpret_cast<uint16_t *>(dst)[i] = (r << 12) | (g << 8) | (b << 4) | a;
    }
}

void PixelConverter::Grayscale8ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i * 4 + 1] = src[i]; // R
        dst[i * 4 + 2] = src[i]; // G
        dst[i * 4 + 3] = src[i]; // B
        dst[i * 4 + 0] = 255;    // A
    }
}

void PixelConverter::ARGB8888ToGrayscale8(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i] = static_cast<uint8_t>(0.299f * src[i * 4 + 1] +
                                      0.587f * src[i * 4 + 2] +
                                      0.114f * src[i * 4 + 3]);
    }
}

void PixelConverter::RGB24ToGrayscale8(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i] = static_cast<uint8_t>(0.299f * src[i * 3 + 1] +
                                      0.587f * src[i * 3 + 2] +
                                      0.114f * src[i * 3 + 3]);
    }
}

void PixelConverter::RGB565ToGrayscale8(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        uint16_t pixel = reinterpret_cast<const uint16_t *>(src)[i];
        uint8_t r = (pixel & 0xF800) >> 8;
        uint8_t g = (pixel & 0x07E0) >> 3;
        uint8_t b = (pixel & 0x001F) << 3;
        dst[i] = static_cast<uint8_t>(0.299f * r + 0.587f * g + 0.114f * b);
    }
}

void Renderer2D::PixelConverter::RGBA8888ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i * 3 + 0] = src[i * 4 + 3]; // A
        dst[i * 3 + 1] = src[i * 4 + 0]; // R
        dst[i * 3 + 2] = src[i * 4 + 1]; // G
        dst[i * 4 + 3] = src[i * 4 + 2]; // B
    }
}
void Renderer2D::PixelConverter::ARGB8888ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i * 3 + 3] = src[i * 4 + 0]; // A
        dst[i * 3 + 0] = src[i * 4 + 1]; // R
        dst[i * 3 + 1] = src[i * 4 + 2]; // G
        dst[i * 4 + 2] = src[i * 4 + 3]; // B
    }
}
void PixelConverter::RGBA8888ToRGB24(const uint8_t *src, uint8_t *dst, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        dst[i * 3 + 0] = src[i * 4 + 0]; // R
        dst[i * 3 + 1] = src[i * 4 + 1]; // G
        dst[i * 3 + 2] = src[i * 4 + 2]; // B
    }
}
