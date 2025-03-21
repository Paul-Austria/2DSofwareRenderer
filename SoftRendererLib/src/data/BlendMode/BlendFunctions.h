#ifndef BLENDFUNCTIONS_H
#define BLENDFUNCTIONS_H

#include "../Color.h"
#include "../PixelFormat/PixelFormat.h"
#include <algorithm>
#include "../PixelFormat/PixelFormatInfo.h"
#include <map>
#include <iostream>
#include "BlendMode.h"

namespace Tergos2D
{
    using BlendFunc = void (*)(uint8_t *dstRow,
                               const uint8_t *srcRow,
                               size_t rowLength,
                               const PixelFormatInfo &targetInfo,
                               const PixelFormatInfo &sourceInfo,
                               Coloring coloring,
                               bool useSolidColor,
                               BlendMode selectedBlendMode);

    class BlendFunctions
    {
    private:
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

    public:
        static void BlendRow(uint8_t *dstRow,
                             const uint8_t *srcRow,
                             size_t rowLength,
                             const PixelFormatInfo &targetInfo,
                             const PixelFormatInfo &sourceInfo,
                             Coloring coloring,
                             bool useSolidColor = false,
                             BlendMode selectedBlendMode = BlendMode::SIMPLE);

        static void BlendRGB24(uint8_t *dstRow,
                               const uint8_t *srcRow,
                               size_t rowLength,
                               const PixelFormatInfo &targetInfo,
                               const PixelFormatInfo &sourceInfo,
                               Coloring coloring,
                               bool useSolidColor,
                               BlendMode selectedBlendMode);

        // NoBranching
        static void BlendRGBA32ToRGB24(uint8_t *dstRow,
                                       const uint8_t *srcRow,
                                       size_t rowLength,
                                       const PixelFormatInfo &targetInfo,
                                       const PixelFormatInfo &sourceInfo,
                                       Coloring coloring,
                                       bool useSolidColor,
                                       BlendMode selectedBlendMode);

        static void BlendSolidRowRGB24(uint8_t *dstRow,
                                       const uint8_t *srcRow,
                                       size_t rowLength,
                                       const PixelFormatInfo &targetInfo,
                                       const PixelFormatInfo &sourceInfo,
                                       Coloring coloring,
                                       bool useSolidColor,
                                       BlendMode selectedBlendMode);
    };
} // namespace Tergos2D

#endif // !BLENDFUNCTIONS_H