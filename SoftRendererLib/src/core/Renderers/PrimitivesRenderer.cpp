#include "PrimitivesRenderer.h"
#include <algorithm>
#include "../util/MemHandler.h"
#include "../data/BlendMode/BlendFunctions.h"

#include "../RenderContext2D.h"

using namespace Renderer2D;


PrimitivesRenderer::PrimitivesRenderer(RenderContext2D& context) : RendererBase(context){
}



void PrimitivesRenderer::DrawRect(Color color, int16_t x, int16_t y, uint16_t length, uint16_t height)
{
    auto targetTexture = context.GetTargetTexture();
    if (!targetTexture)
        return;

    PixelFormat format = targetTexture->GetFormat();
    PixelFormatInfo info = PixelFormatRegistry::GetInfo(format);

    uint8_t *textureData = targetTexture->GetData();
    uint16_t textureWidth = targetTexture->GetWidth();
    uint16_t textureHeight = targetTexture->GetHeight();
    uint32_t pitch = targetTexture->GetPitch(); // Get the pitch (bytes per row)

    
    if(x < 0)
    {
        length = length + x;
        x = 0;
    }
    if(y < 0)
    {
        height = height + y;
        y = 0;
    }

    auto clippingArea = context.GetClippingArea();

    uint16_t clipStartX = context.IsClippingEnabled() ? std::max(x, clippingArea.startX) : x;
    uint16_t clipStartY = context.IsClippingEnabled() ? std::max(y, clippingArea.startY) : y;
    uint16_t clipEndX = context.IsClippingEnabled() ? std::min(static_cast<int>(x + length), static_cast<int>(clippingArea.endX)) : x + length;
    uint16_t clipEndY = context.IsClippingEnabled() ? std::min(static_cast<int>(y + height), static_cast<int>(clippingArea.endY)) : y + height;

    // Restrict drawing within the texture bounds
    clipEndX = std::min(clipEndX, textureWidth);
    clipEndY = std::min(clipEndY, textureHeight);

    // If nothing to draw, return
    if (clipStartX >= clipEndX || clipStartY >= clipEndY)
        return;

    // Calculate the number of bytes in a row
    size_t bytesPerRow = (clipEndX - clipStartX) * info.bytesPerPixel;

    BlendMode subBlend = context.GetBlendMode();
    if (color.GetAlpha() == 255)
        subBlend = BlendMode::NOBLEND;
    uint8_t *dest = textureData + (clipStartY * pitch) + (clipStartX * info.bytesPerPixel);

    switch (subBlend)
    {
    case BlendMode::NOBLEND:
    {

        uint8_t pixelData[MAXBYTESPERPIXEL];
        color.ConvertTo(format, pixelData);

        uint8_t singlePixelData[MAXBYTESPERPIXEL];
        MemHandler::MemCopy(singlePixelData, pixelData, info.bytesPerPixel);

        uint8_t rowPixelData[MAXROWLENGTH * MAXBYTESPERPIXEL];

        for (size_t byteIndex = 0; byteIndex < bytesPerRow; byteIndex += info.bytesPerPixel)
        {
            MemHandler::MemCopy(rowPixelData + byteIndex, singlePixelData, info.bytesPerPixel);
        }

        for (uint16_t j = clipStartY; j < clipEndY; ++j)
        {
            // Calculate the destination for the current row using pitch
            uint8_t *rowDest = dest + (j - clipStartY) * pitch;

            // Use memcpy to copy the whole row of pixels at once
            MemHandler::MemCopy(rowDest, rowPixelData, bytesPerRow);
        }
        break;
    }
    case BlendMode::BLEND:
    {

        uint8_t singlePixelData[MAXBYTESPERPIXEL];

        uint8_t rowPixelData[MAXROWLENGTH * MAXBYTESPERPIXEL];

        for (size_t byteIndex = 0; byteIndex < (clipEndX - clipStartX) * 4; byteIndex += 4)
        {
            MemHandler::MemCopy(rowPixelData + byteIndex, color.data, 4);
        }

        for (uint16_t j = clipStartY; j < clipEndY; ++j)
        {
            uint8_t *rowDest = dest + (j - clipStartY) * pitch;
            size_t rowLength = (clipEndX - clipStartX); // Number of pixels per row
            PixelFormatInfo infosrcColor = PixelFormatRegistry::GetInfo(PixelFormat::ARGB8888);
            BlendFunctions::BlendRow(rowDest, rowPixelData, rowLength, info, infosrcColor,context.GetColoring());
        }
        break;
    }
    default:
        break;
    }
}