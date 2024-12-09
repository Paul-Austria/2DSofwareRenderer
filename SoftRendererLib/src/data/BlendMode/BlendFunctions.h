#ifndef BLENDFUNCTIONS_H
#define BLENDFUNCTIONS_H

#include "../Color.h"
#include "../PixelFormat/PixelFormat.h"
#include <algorithm>
#include "../PixelFormat/PixelFormatInfo.h"
#include <map>
#include <iostream>

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
                                   SelectedBlendMode selectedBlendMode);

        static void BlendRGB24(uint8_t *dstRow,
                               const uint8_t *srcRow,
                               size_t rowLength,
                               const PixelFormatInfo &targetInfo,
                               const PixelFormatInfo &sourceInfo,
                               SelectedBlendMode selectedBlendMode);

        static BlendFunc GetBlendFunc(PixelFormat format)
        {
            switch (format)
            {
            case PixelFormat::RGB24:
            case PixelFormat::BGR24:
                return BlendRGB24;
            default:
                return nullptr;
            }
        }

    public:
        static void BlendRow(uint8_t *dstRow, const uint8_t *srcRow, size_t rowLength, const PixelFormatInfo &targetInfo, const PixelFormatInfo &sourceInfo, SelectedBlendMode selectedBlendMode = SelectedBlendMode::SIMPLE);
    };

} // namespace Renderer2D

#endif // !BLENDFUNCTIONS_H