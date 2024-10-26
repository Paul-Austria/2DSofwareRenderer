#include "PixelConverter.h"

namespace Renderer2D
{
    PixelConverter::ConvertFunc PixelConverter::GetConversionFunction(PixelFormat from, PixelFormat to) const {
        for (const auto& conversion : defaultConversions) {
            if (conversion.from == from && conversion.to == to) {
                return conversion.func;
            }
        }
        throw std::runtime_error("Conversion function not found between these formats.");
    }

    void PixelConverter::Convert(PixelFormat from, PixelFormat to, const uint8_t* src, uint8_t* dst, size_t count) const {
        ConvertFunc func = GetConversionFunction(from, to);
        func(src, dst, count);
    }

    void PixelConverter::RGB24ToARGB8888(const uint8_t* src, uint8_t* dst, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            dst[i * 4 + 1] = src[i * 3 + 0];  // R
            dst[i * 4 + 2] = src[i * 3 + 1];  // G
            dst[i * 4 + 3] = src[i * 3 + 2];  // B
            dst[i * 4 + 0] = 255;             // A
        }
    }

    void PixelConverter::ARGB8888ToRGB24(const uint8_t* src, uint8_t* dst, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            dst[i * 3 + 0] = src[i * 4 + 1];  // R
            dst[i * 3 + 1] = src[i * 4 + 2];  // G
            dst[i * 3 + 2] = src[i * 4 + 3];  // B
        }
    }

    void PixelConverter::RGB565ToARGB8888(const uint8_t* src, uint8_t* dst, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            uint16_t pixel = reinterpret_cast<const uint16_t*>(src)[i];
            dst[i * 4 + 1] = (pixel & 0xF800) >> 8; // R
            dst[i * 4 + 2] = (pixel & 0x07E0) >> 3; // G
            dst[i * 4 + 3] = (pixel & 0x001F) << 3; // B
            dst[i * 4 + 0] = 255;                  // A
        }
    }

    void PixelConverter::ARGB8888ToRGB565(const uint8_t* src, uint8_t* dst, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            uint16_t r = (src[i * 4 + 1] >> 3) & 0x1F;
            uint16_t g = (src[i * 4 + 2] >> 2) & 0x3F;
            uint16_t b = (src[i * 4 + 3] >> 3) & 0x1F;
            reinterpret_cast<uint16_t*>(dst)[i] = (r << 11) | (g << 5) | b;
        }
    }

    void PixelConverter::RGBA1555ToARGB8888(const uint8_t* src, uint8_t* dst, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            uint16_t pixel = reinterpret_cast<const uint16_t*>(src)[i];
            dst[i * 4 + 1] = (pixel & 0x7C00) >> 7; // R
            dst[i * 4 + 2] = (pixel & 0x03E0) >> 2; // G
            dst[i * 4 + 3] = (pixel & 0x001F) << 3; // B
            dst[i * 4 + 0] = (pixel & 0x8000) ? 255 : 0; // A
        }
    }

    void PixelConverter::ARGB8888ToRGBA1555(const uint8_t* src, uint8_t* dst, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            uint16_t r = (src[i * 4 + 1] >> 3) & 0x1F;
            uint16_t g = (src[i * 4 + 2] >> 3) & 0x1F;
            uint16_t b = (src[i * 4 + 3] >> 3) & 0x1F;
            uint16_t a = (src[i * 4 + 0] >= 128) ? 0x8000 : 0;
            reinterpret_cast<uint16_t*>(dst)[i] = a | (r << 10) | (g << 5) | b;
        }
    }

    void PixelConverter::RGBA4444ToARGB8888(const uint8_t* src, uint8_t* dst, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            uint16_t pixel = reinterpret_cast<const uint16_t*>(src)[i];
            dst[i * 4 + 1] = (pixel & 0xF000) >> 8;  // R
            dst[i * 4 + 2] = (pixel & 0x0F00) >> 4;  // G
            dst[i * 4 + 3] = (pixel & 0x00F0);       // B
            dst[i * 4 + 0] = (pixel & 0x000F) << 4;  // A
        }
    }

    void PixelConverter::ARGB8888ToRGBA4444(const uint8_t* src, uint8_t* dst, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            uint16_t r = (src[i * 4 + 1] >> 4) & 0x0F;
            uint16_t g = (src[i * 4 + 2] >> 4) & 0x0F;
            uint16_t b = (src[i * 4 + 3] >> 4) & 0x0F;
            uint16_t a = (src[i * 4 + 0] >> 4) & 0x0F;
            reinterpret_cast<uint16_t*>(dst)[i] = (r << 12) | (g << 8) | (b << 4) | a;
        }
    }

    void PixelConverter::Grayscale8ToARGB8888(const uint8_t* src, uint8_t* dst, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            dst[i * 4 + 1] = src[i];  // R
            dst[i * 4 + 2] = src[i];  // G
            dst[i * 4 + 3] = src[i];  // B
            dst[i * 4 + 0] = 255;     // A
        }
    }

    void PixelConverter::ARGB8888ToGrayscale8(const uint8_t* src, uint8_t* dst, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            dst[i] = static_cast<uint8_t>(0.299f * src[i * 4 + 1] +
                                          0.587f * src[i * 4 + 2] +
                                          0.114f * src[i * 4 + 3]);
        }
    }

    void PixelConverter::RGB24ToGrayscale8(const uint8_t* src, uint8_t* dst, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            dst[i] = static_cast<uint8_t>(0.299f * src[i * 3 + 1] +
                                          0.587f * src[i * 3 + 2] +
                                          0.114f * src[i * 3 + 3]);
        }
    }

    void PixelConverter::RGB565ToGrayscale8(const uint8_t* src, uint8_t* dst, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            uint16_t pixel = reinterpret_cast<const uint16_t*>(src)[i];
            uint8_t r = (pixel & 0xF800) >> 8;
            uint8_t g = (pixel & 0x07E0) >> 3;
            uint8_t b = (pixel & 0x001F) << 3;
            dst[i] = static_cast<uint8_t>(0.299f * r + 0.587f * g + 0.114f * b);
        }
    }

} // namespace Renderer2D
