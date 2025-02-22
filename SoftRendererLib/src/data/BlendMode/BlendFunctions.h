#ifndef BLENDFUNCTIONS_H
#define BLENDFUNCTIONS_H

#include "../Color.h"
#include "../PixelFormat/PixelFormat.h"
#include <algorithm>
#include "../PixelFormat/PixelFormatInfo.h"
#include <map>
#include <iostream>
#include "BlendMode.h"

namespace Renderer2D
{
    class BlendFunctions
    {
    private:
        using BlendFunc = void (*)(uint8_t *dstRow,
                                   const uint8_t *srcRow,
                                   size_t rowLength,
                                   const PixelFormatInfo &targetInfo,
                                   const PixelFormatInfo &sourceInfo,
                                   Coloring coloring,
                                   BlendMode selectedBlendMode);

        static void BlendRGB24(uint8_t *dstRow,
                               const uint8_t *srcRow,
                               size_t rowLength,
                               const PixelFormatInfo &targetInfo,
                               const PixelFormatInfo &sourceInfo,
                               Coloring coloring,
                               BlendMode selectedBlendMode);

        // BLend pixel for clean byte sperated colors
        static inline void BlendPixel(uint8_t *dstPixel, const uint8_t *srcPixel, uint8_t alpha, uint8_t invAlpha, BlendMode blendMode);

        static BlendFunc GetBlendFunc(PixelFormat format, bool useSolidColor)
        {
            switch (format)
            {
            case PixelFormat::RGB24:
                if (useSolidColor)
                    return BlendSolidRowRGB24;
                return BlendRGB24;
            case PixelFormat::BGR24:
                if (useSolidColor)
                    return BlendSolidRowRGB24;
                return BlendRGB24;
            default:
                return nullptr;
            }
        }

        static void BlendSolidRowRGB24(uint8_t *dstRow,
                                       const uint8_t *srcRow,
                                       size_t rowLength,
                                       const PixelFormatInfo &targetInfo,
                                       const PixelFormatInfo &sourceInfo,
                                       Coloring coloring,
                                       BlendMode selectedBlendMode);

    public:
        static void BlendRow(uint8_t *dstRow,
                             const uint8_t *srcRow,
                             size_t rowLength,
                             const PixelFormatInfo &targetInfo,
                             const PixelFormatInfo &sourceInfo,
                             Coloring coloring,
                             bool useSolidColor = false,
                             BlendMode selectedBlendMode = BlendMode::SIMPLE

        );
    };

} // namespace Renderer2D

#endif // !BLENDFUNCTIONS_H