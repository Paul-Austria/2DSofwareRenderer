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

    void PixelConverter::RGB24ToRGBA32(const uint8_t* src, uint8_t* dst, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            dst[i * 4 + 0] = src[i * 3 + 0];  // R
            dst[i * 4 + 1] = src[i * 3 + 1];  // G
            dst[i * 4 + 2] = src[i * 3 + 2];  // B
            dst[i * 4 + 3] = 255;               // A
        }
    }

    void PixelConverter::RGBA32ToRGB24(const uint8_t* src, uint8_t* dst, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            dst[i * 3 + 0] = src[i * 4 + 0];  // R
            dst[i * 3 + 1] = src[i * 4 + 1];  // G
            dst[i * 3 + 2] = src[i * 4 + 2];  // B
        }
    }

    void PixelConverter::BGR24ToRGBA32(const uint8_t* src, uint8_t* dst, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            dst[i * 4 + 0] = src[i * 3 + 2];  // R
            dst[i * 4 + 1] = src[i * 3 + 1];  // G
            dst[i * 4 + 2] = src[i * 3 + 0];  // B
            dst[i * 4 + 3] = 255;               // A
        }
    }

    void PixelConverter::BGRA32ToRGB24(const uint8_t* src, uint8_t* dst, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            dst[i * 3 + 0] = src[i * 4 + 2];  // R
            dst[i * 3 + 1] = src[i * 4 + 1];  // G
            dst[i * 3 + 2] = src[i * 4 + 0];  // B
        }
    }

    void PixelConverter::Grayscale8ToRGBA32(const uint8_t* src, uint8_t* dst, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            dst[i * 4 + 0] = src[i];  // R
            dst[i * 4 + 1] = src[i];  // G
            dst[i * 4 + 2] = src[i];  // B
            dst[i * 4 + 3] = 255;      // A
        }
    }

    void PixelConverter::Grayscale8ToRGB24(const uint8_t* src, uint8_t* dst, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            dst[i * 3 + 0] = src[i];  // R
            dst[i * 3 + 1] = src[i];  // G
            dst[i * 3 + 2] = src[i];  // B
        }
    }

    void PixelConverter::RGBA32ToGrayscale8(const uint8_t* src, uint8_t* dst, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            dst[i] = static_cast<uint8_t>(0.299f * src[i * 4 + 0] +
                                           0.587f * src[i * 4 + 1] +
                                           0.114f * src[i * 4 + 2]);
        }
    }

    void PixelConverter::RGB24ToGrayscale8(const uint8_t* src, uint8_t* dst, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            dst[i] = static_cast<uint8_t>(0.299f * src[i * 3 + 0] +
                                           0.587f * src[i * 3 + 1] +
                                           0.114f * src[i * 3 + 2]);
        }
    }

} // namespace Renderer2D
