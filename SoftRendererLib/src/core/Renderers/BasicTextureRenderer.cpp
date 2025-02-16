
#include "BasicTextureRenderer.h"
#include <algorithm>
#include "../../util/MemHandler.h"
#include "../../data/BlendMode/BlendFunctions.h"
#include "../../data/PixelFormat/PixelConverter.h"
#include "../RenderContext2D.h"

using namespace Renderer2D;

BasicTextureRenderer::BasicTextureRenderer(RenderContext2D &context) : RendererBase(context)
{
}

void BasicTextureRenderer::DrawTexture(Texture &texture, int16_t x, int16_t y)
{
    auto targetTexture = context.GetTargetTexture();
    if (!targetTexture)
        return;

    // Get target texture information
    PixelFormat targetFormat = targetTexture->GetFormat();
    PixelFormatInfo targetInfo = PixelFormatRegistry::GetInfo(targetFormat);
    uint8_t *targetData = targetTexture->GetData();
    uint16_t targetWidth = targetTexture->GetWidth();
    uint16_t targetHeight = targetTexture->GetHeight();
    size_t targetPitch = targetTexture->GetPitch(); // Row stride for target texture

    // Get source texture information
    PixelFormat sourceFormat = texture.GetFormat();
    PixelFormatInfo sourceInfo = PixelFormatRegistry::GetInfo(sourceFormat);
    uint8_t *sourceData = texture.GetData();
    uint16_t sourceWidth = texture.GetWidth();
    uint16_t sourceHeight = texture.GetHeight();
    size_t sourcePitch = texture.GetPitch(); // Row stride for source texture

    // Set clipping boundaries within the source and target textures
    auto clippingArea = context.GetClippingArea();
    int16_t clipStartX = context.IsClippingEnabled() ? std::max(x, clippingArea.startX) : x;
    int16_t clipStartY = context.IsClippingEnabled() ? std::max(y, clippingArea.startY) : y;
    int16_t clipEndX = context.IsClippingEnabled() ? std::min(static_cast<int>(x + sourceWidth), static_cast<int>(clippingArea.endX)) : x + sourceWidth;
    int16_t clipEndY = context.IsClippingEnabled() ? std::min(static_cast<int>(y + sourceHeight), static_cast<int>(clippingArea.endY)) : y + sourceHeight;

    // Restrict drawing to the target texture’s bounds
    clipEndX = std::min(clipEndX, (int16_t)targetWidth);
    clipEndY = std::min(clipEndY, (int16_t)targetHeight);

    // Adjust clipping start positions for negative coordinates
    if (x < 0)
    {
        clipStartX = std::max(clipStartX, static_cast<int16_t>(0));
    }
    if (y < 0)
    {
        clipStartY = std::max(clipStartY, static_cast<int16_t>(0));
    }

    // Check if there is anything to draw
    if (clipStartX >= clipEndX || clipStartY >= clipEndY)
        return;

    // Determine blending mode
    BlendMode subBlend = (context.GetBlendMode() == BlendMode::NOBLEND || sourceInfo.hasAlpha) ? context.GetBlendMode() : BlendMode::NOBLEND;

    switch (subBlend)
    {
    case BlendMode::NOBLEND:
    {
        // Conversion function for source to target format if necessary
        PixelConverter::ConvertFunc convertFunc = nullptr;

        convertFunc = PixelConverter::GetConversionFunction(sourceFormat, targetFormat);
        if (!convertFunc)
            return;

        for (uint16_t j = clipStartY; j < clipEndY; ++j)
        {
            uint8_t *targetRow = targetData + j * targetPitch + clipStartX * targetInfo.bytesPerPixel;
            const uint8_t *sourceRow = sourceData + (j - y) * sourcePitch + (clipStartX - x) * sourceInfo.bytesPerPixel;

            convertFunc(sourceRow, targetRow, clipEndX - clipStartX);
        }
        break;
    }
    default:
    {
        for (uint16_t j = clipStartY; j < clipEndY; ++j)
        {
            uint8_t *targetRow = targetData + j * targetPitch + clipStartX * targetInfo.bytesPerPixel;
            const uint8_t *sourceRow = sourceData + (j - y) * sourcePitch + (clipStartX - x) * sourceInfo.bytesPerPixel;

            // Blend the entire row
            BlendFunctions::BlendRow(targetRow, sourceRow, clipEndX - clipStartX, targetInfo, sourceInfo, context.GetColoring(), context.GetBlendMode());
        }
        break;
    }
    break;
    }
}