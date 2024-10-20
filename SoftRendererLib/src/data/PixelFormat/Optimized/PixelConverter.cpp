#include "../PixelConverter.h"
#include <immintrin.h>  // For SIMD (SSE/AVX or NEON on ARM devices)

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
        size_t i = 0;
        __m128i alpha = _mm_set1_epi32(0xFF000000); // Preload alpha value

        // Process in batches of 4 pixels (12 bytes input -> 16 bytes output)
        for (; i + 3 < count; i += 4) {
            __m128i rgb1 = _mm_loadu_si128((__m128i*)(src + i * 3));  // Load 12 bytes of RGB data
            __m128i rgb2 = _mm_srli_si128(rgb1, 12);                   // Shift to load remaining

            // Shuffle and combine with alpha
            __m128i rgba1 = _mm_or_si128(_mm_shuffle_epi8(rgb1, _mm_setr_epi8(
                0, 1, 2, -1, 3, 4, 5, -1, 6, 7, 8, -1, 9, 10, 11, -1)), alpha);

            __m128i rgba2 = _mm_or_si128(_mm_shuffle_epi8(rgb2, _mm_setr_epi8(
                0, 1, 2, -1, 3, 4, 5, -1, 6, 7, 8, -1, 9, 10, 11, -1)), alpha);

            _mm_storeu_si128((__m128i*)(dst + i * 4), rgba1);  // Store 16 bytes (RGBA)
            _mm_storeu_si128((__m128i*)(dst + i * 4 + 16), rgba2); // Next 16 bytes
        }

        // Tail processing for remaining pixels
        for (; i < count; ++i) {
            dst[i * 4 + 0] = src[i * 3 + 0];
            dst[i * 4 + 1] = src[i * 3 + 1];
            dst[i * 4 + 2] = src[i * 3 + 2];
            dst[i * 4 + 3] = 255;  // Set alpha to fully opaque
        }
    }

    void PixelConverter::RGBA32ToRGB24(const uint8_t* src, uint8_t* dst, size_t count) {
        size_t i = 0;

        // Process in batches of 4 pixels (16 bytes input -> 12 bytes output)
        for (; i + 3 < count; i += 4) {
            __m128i rgba = _mm_loadu_si128((__m128i*)(src + i * 4));  // Load 16 bytes (RGBA)

            // Shuffle to discard alpha and store RGB
            __m128i rgb = _mm_shuffle_epi8(rgba, _mm_setr_epi8(
                0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, -1, -1, -1, -1));

            _mm_storeu_si128((__m128i*)(dst + i * 3), rgb);  // Store 12 bytes (RGB)
        }

        // Tail processing for remaining pixels
        for (; i < count; ++i) {
            dst[i * 3 + 0] = src[i * 4 + 0];
            dst[i * 3 + 1] = src[i * 4 + 1];
            dst[i * 3 + 2] = src[i * 4 + 2];
        }
    }

    void PixelConverter::BGR24ToRGBA32(const uint8_t* src, uint8_t* dst, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            dst[i * 4 + 0] = src[i * 3 + 2];  // Swap R and B
            dst[i * 4 + 1] = src[i * 3 + 1];
            dst[i * 4 + 2] = src[i * 3 + 0];
            dst[i * 4 + 3] = 255;  // Set alpha to fully opaque
        }
    }

    void PixelConverter::BGRA32ToRGB24(const uint8_t* src, uint8_t* dst, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            dst[i * 3 + 0] = src[i * 4 + 2];  // Swap R and B
            dst[i * 3 + 1] = src[i * 4 + 1];
            dst[i * 3 + 2] = src[i * 4 + 0];
        }
    }

    void PixelConverter::Grayscale8ToRGBA32(const uint8_t* src, uint8_t* dst, size_t count) {
        size_t i = 0;
        __m128i alpha = _mm_set1_epi32(0xFF000000);  // Preload alpha value

        // Process in batches of 16 pixels (16 bytes input -> 64 bytes output)
        for (; i + 15 < count; i += 16) {
            __m128i grayscale = _mm_loadu_si128((__m128i*)(src + i));  // Load 16 bytes of grayscale

            // Duplicate grayscale value across R, G, B channels and add alpha
            __m128i rgba1 = _mm_or_si128(_mm_shuffle_epi8(grayscale, _mm_setr_epi8(
                0, 0, 0, -1, 1, 1, 1, -1, 2, 2, 2, -1, 3, 3, 3, -1)), alpha);

            __m128i rgba2 = _mm_or_si128(_mm_shuffle_epi8(grayscale, _mm_setr_epi8(
                4, 4, 4, -1, 5, 5, 5, -1, 6, 6, 6, -1, 7, 7, 7, -1)), alpha);

            __m128i rgba3 = _mm_or_si128(_mm_shuffle_epi8(grayscale, _mm_setr_epi8(
                8, 8, 8, -1, 9, 9, 9, -1, 10, 10, 10, -1, 11, 11, 11, -1)), alpha);

            __m128i rgba4 = _mm_or_si128(_mm_shuffle_epi8(grayscale, _mm_setr_epi8(
                12, 12, 12, -1, 13, 13, 13, -1, 14, 14, 14, -1, 15, 15, 15, -1)), alpha);

            _mm_storeu_si128((__m128i*)(dst + i * 4), rgba1);  // Store 16 bytes (RGBA)
            _mm_storeu_si128((__m128i*)(dst + i * 4 + 16), rgba2); // Next
            _mm_storeu_si128((__m128i*)(dst + i * 4 + 32), rgba3); // Next
            _mm_storeu_si128((__m128i*)(dst + i * 4 + 48), rgba4); // Next
        }

        // Tail processing for remaining pixels
        for (; i < count; ++i) {
            dst[i * 4 + 0] = src[i];
            dst[i * 4 + 1] = src[i];
            dst[i * 4 + 2] = src[i];
            dst[i * 4 + 3] = 255;  // Set alpha to fully opaque
        }
    }

    void PixelConverter::Grayscale8ToRGB24(const uint8_t* src, uint8_t* dst, size_t count) {
        size_t i = 0;

        // Process in batches of 16 (16 pixels)
        for (; i + 15 < count; i += 16) {
            // Load 16 grayscale values
            __m128i gray = _mm_loadu_si128((__m128i*)(src + i));

            // Expand each grayscale value to RGB (R=G=B=gray)
            __m128i rgb1 = _mm_unpacklo_epi8(gray, gray);  // Lower 8 pixels
            __m128i rgb2 = _mm_unpackhi_epi8(gray, gray);  // Upper 8 pixels

            // Pack into RGB format (3 channels)
            __m128i rgb1_rgb = _mm_unpacklo_epi16(rgb1, _mm_setzero_si128()); // For first 8 pixels
            __m128i rgb1_final = _mm_packus_epi16(rgb1_rgb, rgb1_rgb);  // Pack to 24 bits

            __m128i rgb2_rgb = _mm_unpacklo_epi16(rgb2, _mm_setzero_si128()); // For second 8 pixels
            __m128i rgb2_final = _mm_packus_epi16(rgb2_rgb, rgb2_rgb);  // Pack to 24 bits

            // Store RGB values
            _mm_storeu_si128((__m128i*)(dst + i * 3), rgb1_final);
            _mm_storeu_si128((__m128i*)(dst + i * 3 + 16), rgb2_final);
        }

        // Tail processing for remaining pixels
        for (; i < count; ++i) {
            dst[i * 3 + 0] = src[i];  // Same value for R, G, B
            dst[i * 3 + 1] = src[i];
            dst[i * 3 + 2] = src[i];
        }
    }

    void PixelConverter::RGBA32ToGrayscale8(const uint8_t* src, uint8_t* dst, size_t count) {
        size_t i = 0;

        // Process in batches of 16 pixels
        for (; i + 3 < count; i += 4) {
            // Load 16 bytes (4 pixels)
            __m128i rgba = _mm_loadu_si128((__m128i*)(src + i * 4));

            // Extract the R, G, B channels and calculate grayscale
            __m128i r = _mm_shuffle_epi8(rgba, _mm_setr_epi8(0, 0, 0, 0, 4, 4, 4, 4, 8, 8, 8, 8, -1, -1, -1, -1));
            __m128i g = _mm_shuffle_epi8(rgba, _mm_setr_epi8(1, 1, 1, 1, 5, 5, 5, 5, 9, 9, 9, 9, -1, -1, -1, -1));
            __m128i b = _mm_shuffle_epi8(rgba, _mm_setr_epi8(2, 2, 2, 2, 6, 6, 6, 6, 10, 10, 10, 10, -1, -1, -1, -1));

            // Convert to grayscale using the coefficients
            __m128i gray = _mm_add_epi32(_mm_add_epi32(_mm_mullo_epi32(r, _mm_set1_epi32(299)),
                _mm_mullo_epi32(g, _mm_set1_epi32(587))),
                _mm_mullo_epi32(b, _mm_set1_epi32(114)));

            gray = _mm_srli_epi32(gray, 10);  // Right shift to fit in 8 bits
            gray = _mm_packus_epi32(gray, gray); // Pack to fit in 8 bits
            gray = _mm_packus_epi16(gray, gray); // Final packing

            // Store the result
            _mm_storeu_si128((__m128i*)(dst + i), gray);
        }

        // Tail processing for remaining pixels
        for (; i < count; ++i) {
            dst[i] = static_cast<uint8_t>(0.299f * src[i * 4 + 0] +
                0.587f * src[i * 4 + 1] +
                0.114f * src[i * 4 + 2]);
        }
    }

    void PixelConverter::RGB24ToGrayscale8(const uint8_t* src, uint8_t* dst, size_t count) {
        size_t i = 0;

        // Process in batches of 16 pixels
        for (; i + 3 < count; i += 4) {
            // Load 12 bytes (4 pixels)
            __m128i rgb = _mm_loadu_si128((__m128i*)(src + i * 3));

            // Extract R, G, B channels
            __m128i r = _mm_shuffle_epi8(rgb, _mm_setr_epi8(0, 0, 0, 0, 3, 3, 3, 3, 6, 6, 6, 6, -1, -1, -1, -1));
            __m128i g = _mm_shuffle_epi8(rgb, _mm_setr_epi8(1, 1, 1, 1, 4, 4, 4, 4, 7, 7, 7, 7, -1, -1, -1, -1));
            __m128i b = _mm_shuffle_epi8(rgb, _mm_setr_epi8(2, 2, 2, 2, 5, 5, 5, 5, 8, 8, 8, 8, -1, -1, -1, -1));

            // Convert to grayscale using the coefficients
            __m128i gray = _mm_add_epi32(_mm_add_epi32(_mm_mullo_epi32(r, _mm_set1_epi32(299)),
                _mm_mullo_epi32(g, _mm_set1_epi32(587))),
                _mm_mullo_epi32(b, _mm_set1_epi32(114)));

            gray = _mm_srli_epi32(gray, 10);  // Right shift to fit in 8 bits
            gray = _mm_packus_epi32(gray, gray); // Pack to fit in 8 bits
            gray = _mm_packus_epi16(gray, gray); // Final packing

            // Store the result
            _mm_storeu_si128((__m128i*)(dst + i), gray);
        }

        // Tail processing for remaining pixels
        for (; i < count; ++i) {
            dst[i] = static_cast<uint8_t>(0.299f * src[i * 3 + 0] +
                0.587f * src[i * 3 + 1] +
                0.114f * src[i * 3 + 2]);
        }
    }

} // namespace Renderer2D
