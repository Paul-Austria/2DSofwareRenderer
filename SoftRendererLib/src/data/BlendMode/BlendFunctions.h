#ifndef BLENDFUNCTIONS_H
#define BLENDFUNCTIONS_H

#include "../Color.h"
#include "../PixelFormat/PixelFormat.h"
#include <algorithm>
#include "../PixelFormat/PixelFormatInfo.h"
#include <map>

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

        static const BlendFunc &GetBlendFunc(PixelFormat format, bool &success)
        {

            static const std::map<PixelFormat, BlendFunc> blendRegistry = {
                {PixelFormat::RGB24, BlendRGB24},
                {PixelFormat::BGR24, BlendRGB24}};

            auto t = blendRegistry.find(format);
            if (t == blendRegistry.end())
            {
                success = false;
                static const BlendFunc nullFunc = nullptr;
                return nullFunc;
            }
            success = true;
            return blendRegistry.at(format);
        }

    public:
        static void BlendRow(uint8_t *dstRow, const uint8_t *srcRow, size_t rowLength, const PixelFormatInfo &targetInfo, const PixelFormatInfo &sourceInfo, SelectedBlendMode selectedBlendMode = SelectedBlendMode::SIMPLE);
    };

} // namespace Renderer2D

#endif // !BLENDFUNCTIONS_H