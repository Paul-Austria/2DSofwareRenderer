
#include "TransformedTextureRenderer.h"
#include <algorithm>
#include "../../util/MemHandler.h"
#include "../../data/BlendMode/BlendFunctions.h"
#include "../../data/PixelFormat/PixelConverter.h"
#include "../RenderContext2D.h"
#include <float.h>
#include <math.h>

using namespace Tergos2D;

TransformedTextureRenderer::TransformedTextureRenderer(RenderContext2D &context) : RendererBase(context)
{
}

void TransformedTextureRenderer::DrawTexture(Texture &texture, int16_t x, int16_t y, float angle, int16_t offsetX, int16_t offsetY)
{
    auto targetTexture = context.GetTargetTexture();
    if (!targetTexture)
        return;

    // Validate angle
    int normalizedAngle = static_cast<int>(angle) % 360;
    if (normalizedAngle < 0)
        normalizedAngle += 360;

    if (normalizedAngle == 0)
    {

        context.basicTextureRenderer.DrawTexture(texture, x, y);
        return;
    }

    // Get source texture information
    PixelFormat sourceFormat = texture.GetFormat();
    PixelFormatInfo sourceInfo = PixelFormatRegistry::GetInfo(sourceFormat);
    uint8_t *sourceData = texture.GetData();
    uint16_t sourceWidth = texture.GetWidth();
    uint16_t sourceHeight = texture.GetHeight();
    size_t sourcePitch = texture.GetPitch();

    // Determine blending mode
    BlendContext bc = context.GetBlendContext();
    bc.mode = context.BlendModeToUse(sourceInfo);
    PixelFormat targetFormat = targetTexture->GetFormat();
    PixelFormatInfo targetInfo = PixelFormatRegistry::GetInfo(targetFormat);
    uint8_t *targetData = targetTexture->GetData();
    uint16_t targetWidth = targetTexture->GetWidth();
    uint16_t targetHeight = targetTexture->GetHeight();
    size_t targetPitch = targetTexture->GetPitch();

    // Allocate a buffer for one row of converted pixels
    size_t rowLength = sourceWidth * targetInfo.bytesPerPixel;
    alignas(16) uint8_t buffer[MAXROWLENGTH * MAXBYTESPERPIXEL];
    PixelConverter::ConvertFunc convertFunc = PixelConverter::GetConversionFunction(sourceFormat, targetFormat);

    // Calculate the center of the source image
    int centerX = sourceWidth / 2;
    int centerY = sourceHeight / 2;

    // Calculate the offset to maintain the center point
    int offX = (x + centerX - sourceHeight / 2);
    int offY = (y + centerY - sourceWidth / 2);

    ClippingArea clippingArea = context.GetClippingArea();
    if (false)
    {
    }
    else
    {
        float radians = normalizedAngle * 3.14159265358979f / 180.0f;
        float cosAngle = cos(radians);
        float sinAngle = sin(radians);

        offsetX =  offsetX;
        offsetY =  offsetY;

        float pivotX = x + offsetX;
        float pivotY = y + offsetY;

        // Calculate the bounds of the rotated image by transforming all corners
        float corners[4][2] = {
            {-offsetX, -offsetY},                           // Top-left
            {sourceWidth - offsetX, -offsetY},              // Top-right
            {-offsetX, sourceHeight - offsetY},             // Bottom-left
            {sourceWidth - offsetX, sourceHeight - offsetY} // Bottom-right
        };

        // Find the min/max coordinates after rotation
        float minX = FLT_MAX, minY = FLT_MAX;
        float maxX = -FLT_MAX, maxY = -FLT_MAX;

        for (int i = 0; i < 4; i++)
        {
            float rotX = corners[i][0] * cosAngle - corners[i][1] * sinAngle + pivotX;
            float rotY = corners[i][0] * sinAngle + corners[i][1] * cosAngle + pivotY;

            minX = std::min(minX, rotX);
            minY = std::min(minY, rotY);
            maxX = std::max(maxX, rotX);
            maxY = std::max(maxY, rotY);
        }

        int boundMinX = static_cast<int>(floor(minX)) - 1;
        int boundMinY = static_cast<int>(floor(minY)) - 1;
        int boundMaxX = static_cast<int>(ceil(maxX)) + 1;
        int boundMaxY = static_cast<int>(ceil(maxY)) + 1;

        int lastConvertedRow = -1;

        for (int destY = boundMinY; destY <= boundMaxY; destY++)
        {
            for (int destX = boundMinX; destX <= boundMaxX; destX++)
            {
                if (destX < 0 || destX >= targetWidth || destY < 0 || destY >= targetHeight)
                {
                    continue;
                }

                if (context.IsClippingEnabled())
                {
                    if (destX < clippingArea.startX || destX >= clippingArea.endX || destY < clippingArea.startY || destY >= clippingArea.endY)
                    {
                        continue;
                    }
                }

                float dx = destX - pivotX;
                float dy = destY - pivotY;

                float srcXf = dx * cosAngle + dy * sinAngle + offsetX;
                float srcYf = -dx * sinAngle + dy * cosAngle + offsetY;

                int srcX = static_cast<int>(srcXf);
                int srcY = static_cast<int>(srcYf);

                if (srcX >= 0 && srcX < sourceWidth && srcY >= 0 && srcY < sourceHeight)
                {
                    uint8_t *sourcePixel = nullptr;
                    uint8_t *targetPixel = nullptr;
                    uint8_t *sourceRow = sourceData + (srcY * sourcePitch);
                    switch (bc.mode)
                    {
                    case BlendMode::NOBLEND:
                        if (srcY != lastConvertedRow)
                        {
                            convertFunc(sourceRow, buffer, sourceWidth);
                            lastConvertedRow = srcY;
                        }
                        sourcePixel = buffer + (srcX * targetInfo.bytesPerPixel);
                        targetPixel = targetData + (destY * targetPitch) + (destX * targetInfo.bytesPerPixel);
                        memcpy(targetPixel, sourcePixel, targetInfo.bytesPerPixel);
                        break;

                    default:
                    {
                        sourcePixel = sourceRow + (srcX * sourceInfo.bytesPerPixel);
                        targetPixel = targetData + (destY * targetPitch) + (destX * targetInfo.bytesPerPixel);
                        context.GetBlendFunc()(targetPixel, sourcePixel, 1, targetInfo, sourceInfo, context.GetColoring(),false,bc);
                    }
                    break;
                    }
                }
            }
        }
    }
    // Rotation-specific loops
}

void TransformedTextureRenderer::DrawTextureAndScale(Texture &texture, int16_t x, int16_t y,
                                             float scaleX, float scaleY, float angle,
                                             int16_t offsetX, int16_t offsetY)
{
    auto targetTexture = context.GetTargetTexture();
    if (!targetTexture || scaleX <= 0 || scaleY <= 0)
        return;

    // Get source texture information
    PixelFormat sourceFormat = texture.GetFormat();
    PixelFormatInfo sourceInfo = PixelFormatRegistry::GetInfo(sourceFormat);
    uint8_t *sourceData = texture.GetData();
    uint16_t sourceWidth = texture.GetWidth();
    uint16_t sourceHeight = texture.GetHeight();
    size_t sourcePitch = texture.GetPitch();

    // Calculate scaled dimensions
    uint16_t scaledWidth = static_cast<uint16_t>(sourceWidth * scaleX);
    uint16_t scaledHeight = static_cast<uint16_t>(sourceHeight * scaleY);

    // Validate angle
    int normalizedAngle = static_cast<int>(angle) % 360;
    if (normalizedAngle < 0)
        normalizedAngle += 360;

    // Get target texture information
    PixelFormat targetFormat = targetTexture->GetFormat();
    PixelFormatInfo targetInfo = PixelFormatRegistry::GetInfo(targetFormat);
    uint8_t *targetData = targetTexture->GetData();
    uint16_t targetWidth = targetTexture->GetWidth();
    uint16_t targetHeight = targetTexture->GetHeight();
    size_t targetPitch = targetTexture->GetPitch();

    // Determine blending mode
    BlendContext bc = context.GetBlendContext();
    bc.mode = context.BlendModeToUse(sourceInfo);

    // Calculate rotation center with scaling
    float centerX = scaledWidth / 2.0f;
    float centerY = scaledHeight / 2.0f;

    // Calculate the pivot point including offset
    float pivotX = centerX + offsetX;
    float pivotY = centerY + offsetY;

    float radians = normalizedAngle * 3.14159265358979f / 180.0f;
    float cosAngle = cos(radians);
    float sinAngle = sin(radians);

    // Calculate the bounds of the rotated and scaled image
    float corners[4][2] = {
        {-centerX, -centerY},                           // Top-left
        {scaledWidth - centerX, -centerY},              // Top-right
        {-centerX, scaledHeight - centerY},             // Bottom-left
        {scaledWidth - centerX, scaledHeight - centerY} // Bottom-right
    };

    // Find the min/max coordinates after rotation
    float minX = FLT_MAX, minY = FLT_MAX;
    float maxX = -FLT_MAX, maxY = -FLT_MAX;

    for (int i = 0; i < 4; i++)
    {
        float rotX = corners[i][0] * cosAngle - corners[i][1] * sinAngle + pivotX;
        float rotY = corners[i][0] * sinAngle + corners[i][1] * cosAngle + pivotY;

        minX = std::min(minX, rotX);
        minY = std::min(minY, rotY);
        maxX = std::max(maxX, rotX);
        maxY = std::max(maxY, rotY);
    }

    // Calculate bounds with padding
    int boundMinX = static_cast<int>(floor(minX)) - 1;
    int boundMinY = static_cast<int>(floor(minY)) - 1;
    int boundMaxX = static_cast<int>(ceil(maxX)) + 1;
    int boundMaxY = static_cast<int>(ceil(maxY)) + 1;

    // Adjust pivot to the new bounds min point
    float newPivotX = x - boundMinX;
    float newPivotY = y - boundMinY;

    // Clamp bounds to target texture and clipping region
    if (context.IsClippingEnabled())
    {
        auto clippingArea = context.GetClippingArea();
        boundMinX = std::max(boundMinX, static_cast<int>(clippingArea.startX));
        boundMinY = std::max(boundMinY, static_cast<int>(clippingArea.startY));
        boundMaxX = std::min(boundMaxX, static_cast<int>(clippingArea.endX));
        boundMaxY = std::min(boundMaxY, static_cast<int>(clippingArea.endY));
    }

    boundMinX = std::max(boundMinX, 0);
    boundMinY = std::max(boundMinY, 0);
    boundMaxX = std::min(boundMaxX, static_cast<int>(targetWidth));
    boundMaxY = std::min(boundMaxY, static_cast<int>(targetHeight));

    uint8_t dstBuffer[MAXBYTESPERPIXEL];

    // Inverse transformation matrix
    float invCosAngle = cosAngle;
    float invSinAngle = -sinAngle;

    for (int destY = boundMinY; destY < boundMaxY; destY++)
    {
        for (int destX = boundMinX; destX < boundMaxX; destX++)
        {
            // Calculate source position with rotation and scaling
            float dx = destX - newPivotX;
            float dy = destY - newPivotY;

            // Apply inverse rotation
            float rotX = dx * invCosAngle - dy * invSinAngle;
            float rotY = dx * invSinAngle + dy * invCosAngle;

            // Transform to source space (accounting for scale)
            float srcXf = (rotX + centerX) / scaleX;
            float srcYf = (rotY + centerY) / scaleY;

            // Check if the source pixel is within bounds
            if (srcXf >= 0 && srcXf < sourceWidth && srcYf >= 0 && srcYf < sourceHeight)
            {
                switch (context.GetSamplingMethod())
                {
                case SamplingMethod::NEAREST:
                {
                    uint16_t sx = static_cast<uint16_t>(srcXf + 0.5f);
                    uint16_t sy = static_cast<uint16_t>(srcYf + 0.5f);
                    const uint8_t *srcPixel = sourceData + sy * sourcePitch + sx * sourceInfo.bytesPerPixel;
                    PixelConverter::Convert(sourceFormat, targetFormat, srcPixel, dstBuffer, 1);
                    break;
                }

                case SamplingMethod::LINEAR:
                {
                    int x0 = static_cast<int>(srcXf);
                    int y0 = static_cast<int>(srcYf);
                    int x1 = std::min(x0 + 1, static_cast<int>(sourceWidth - 1));
                    int y1 = std::min(y0 + 1, static_cast<int>(sourceHeight - 1));

                    float fx = srcXf - x0;
                    float fy = srcYf - y0;

                    const uint8_t *pixels[4] = {
                        sourceData + y0 * sourcePitch + x0 * sourceInfo.bytesPerPixel,
                        sourceData + y0 * sourcePitch + x1 * sourceInfo.bytesPerPixel,
                        sourceData + y1 * sourcePitch + x0 * sourceInfo.bytesPerPixel,
                        sourceData + y1 * sourcePitch + x1 * sourceInfo.bytesPerPixel};

                    Color colors[4];
                    for (int i = 0; i < 4; i++)
                    {
                        colors[i] = Color(pixels[i], sourceFormat);
                    }

                    Color top = Color::Lerp(colors[0], colors[1], fx);
                    Color bottom = Color::Lerp(colors[2], colors[3], fx);
                    Color finalColor = Color::Lerp(top, bottom, fy);
                    finalColor.ConvertTo(targetFormat, dstBuffer);
                    break;
                }
                }

                uint8_t *targetPixel = targetData + destY * targetPitch + destX * targetInfo.bytesPerPixel;

                if (bc.mode != BlendMode::NOBLEND)
                {
                    context.GetBlendFunc()(targetPixel, dstBuffer, 1, targetInfo, sourceInfo, context.GetColoring(), false, context.GetBlendContext());
                }
                else
                {
                    MemHandler::MemCopy(targetPixel, dstBuffer, targetInfo.bytesPerPixel);
                }
            }
        }
    }
}



void TransformedTextureRenderer::DrawTexture(Texture &texture, const float transformationMatrix[3][3])
{
    auto targetTexture = context.GetTargetTexture();
    if (!targetTexture || !texture.GetData())
    {
        return;
    }

    // Get source texture information
    PixelFormat sourceFormat = texture.GetFormat();
    PixelFormatInfo sourceInfo = PixelFormatRegistry::GetInfo(sourceFormat);
    uint8_t *sourceData = texture.GetData();
    uint16_t sourceWidth = texture.GetWidth();
    uint16_t sourceHeight = texture.GetHeight();
    size_t sourcePitch = texture.GetPitch();

    // Get target texture information
    PixelFormat targetFormat = targetTexture->GetFormat();
    PixelFormatInfo targetInfo = PixelFormatRegistry::GetInfo(targetFormat);
    uint8_t *targetData = targetTexture->GetData();
    uint16_t targetWidth = targetTexture->GetWidth();
    uint16_t targetHeight = targetTexture->GetHeight();
    size_t targetPitch = targetTexture->GetPitch();

    // Calculate the bounding box of the transformed source texture
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::min();
    float maxY = std::numeric_limits<float>::min();

    std::vector<std::pair<float, float>> corners = {
        {0, 0},
        {static_cast<float>(sourceWidth), 0},
        {0, static_cast<float>(sourceHeight)},
        {static_cast<float>(sourceWidth), static_cast<float>(sourceHeight)}
    };

    for (const auto& corner : corners)
    {
        float x = transformationMatrix[0][0] * corner.first + transformationMatrix[0][1] * corner.second + transformationMatrix[0][2];
        float y = transformationMatrix[1][0] * corner.first + transformationMatrix[1][1] * corner.second + transformationMatrix[1][2];

        if (x < minX) minX = x;
        if (y < minY) minY = y;
        if (x > maxX) maxX = x;
        if (y > maxY) maxY = y;
    }

    // Clamp the bounding box to the target texture's dimensions
    int16_t startX = std::max(static_cast<int16_t>(std::floor(minX)), static_cast<int16_t>(0));
    int16_t startY = std::max(static_cast<int16_t>(std::floor(minY)), static_cast<int16_t>(0));
    int16_t endX = std::min(static_cast<int16_t>(std::ceil(maxX)), static_cast<int16_t>(targetWidth));
    int16_t endY = std::min(static_cast<int16_t>(std::ceil(maxY)), static_cast<int16_t>(targetHeight));


    if (context.IsClippingEnabled())
    {
        auto clippingArea = context.GetClippingArea();
        startX = std::max(startX, static_cast<int16_t>(clippingArea.startX));
        startY = std::max(startY, static_cast<int16_t>(clippingArea.startY));
        endX = std::min(endX, static_cast<int16_t>(clippingArea.endX));
        endY = std::min(endY, static_cast<int16_t>(clippingArea.endY));
    }

    // Define the inverse transformation matrix
    float invMatrix[3][3];
    float det = transformationMatrix[0][0] * (transformationMatrix[1][1] * transformationMatrix[2][2] - transformationMatrix[1][2] * transformationMatrix[2][1]) -
                transformationMatrix[0][1] * (transformationMatrix[1][0] * transformationMatrix[2][2] - transformationMatrix[1][2] * transformationMatrix[2][0]) +
                transformationMatrix[0][2] * (transformationMatrix[1][0] * transformationMatrix[2][1] - transformationMatrix[1][1] * transformationMatrix[2][0]);

    if (det == 0.0f)
        return; // Transformation matrix is not invertible

    float invDet = 1.0f / det;

    // Calculate the inverse matrix
    invMatrix[0][0] = (transformationMatrix[1][1] * transformationMatrix[2][2] - transformationMatrix[1][2] * transformationMatrix[2][1]) * invDet;
    invMatrix[0][1] = (transformationMatrix[0][2] * transformationMatrix[2][1] - transformationMatrix[0][1] * transformationMatrix[2][2]) * invDet;
    invMatrix[0][2] = (transformationMatrix[0][1] * transformationMatrix[1][2] - transformationMatrix[0][2] * transformationMatrix[1][1]) * invDet;
    invMatrix[1][0] = (transformationMatrix[1][2] * transformationMatrix[2][0] - transformationMatrix[1][0] * transformationMatrix[2][2]) * invDet;
    invMatrix[1][1] = (transformationMatrix[0][0] * transformationMatrix[2][2] - transformationMatrix[0][2] * transformationMatrix[2][0]) * invDet;
    invMatrix[1][2] = (transformationMatrix[0][2] * transformationMatrix[1][0] - transformationMatrix[0][0] * transformationMatrix[1][2]) * invDet;
    invMatrix[2][0] = (transformationMatrix[1][0] * transformationMatrix[2][1] - transformationMatrix[1][1] * transformationMatrix[2][0]) * invDet;
    invMatrix[2][1] = (transformationMatrix[0][1] * transformationMatrix[2][0] - transformationMatrix[0][0] * transformationMatrix[2][1]) * invDet;
    invMatrix[2][2] = (transformationMatrix[0][0] * transformationMatrix[1][1] - transformationMatrix[0][1] * transformationMatrix[1][0]) * invDet;

    BlendContext bc = context.GetBlendContext();
    bc.mode = context.BlendModeToUse(sourceInfo);

    // Iterate over the bounding box in the target texture
    uint8_t buffer[100*4];
    int pos = 0;

    const int maxPos = 10;
    uint8_t *targetPixel = nullptr;
    PixelConverter::ConvertFunc convertFunc = PixelConverter::GetConversionFunction(sourceFormat, targetFormat);
    if(!convertFunc) return;
    for (int16_t y = startY; y < endY; ++y)
    {
        for (int16_t x = startX; x < endX; ++x)
        {
            // Apply the inverse transformation to find the corresponding source pixel
            float srcX = invMatrix[0][0] * x + invMatrix[0][1] * y + invMatrix[0][2];
            float srcY = invMatrix[1][0] * x + invMatrix[1][1] * y + invMatrix[1][2];

            // Check if the source pixel is within bounds
            if (srcX >= 0 && srcX < sourceWidth && srcY >= 0 && srcY < sourceHeight)
            {
                uint16_t intSrcX = static_cast<uint16_t>(srcX);
                uint16_t intSrcY = static_cast<uint16_t>(srcY);
                if(pos == 0)
                {
                    targetPixel = targetData + y * targetPitch + x * targetInfo.bytesPerPixel;
                }
                const uint8_t *sourcePixel = sourceData + intSrcY * sourcePitch + intSrcX * sourceInfo.bytesPerPixel;
                std::memcpy(buffer + sourceInfo.bytesPerPixel*pos, sourcePixel,sourceInfo.bytesPerPixel);
                pos++;

                if (pos == maxPos)
                {

                    if(bc.mode == BlendMode::NOBLEND){
                        convertFunc(buffer, targetPixel, pos);
                    }
                    else{
                        context.GetBlendFunc()(targetPixel, buffer, pos, targetInfo, sourceInfo, context.GetColoring(),false,bc);
                    }
                    pos = 0;
                }

            }
        }
        if(pos != 0)
        {
            if(bc.mode == BlendMode::NOBLEND){
                convertFunc(buffer, targetPixel, pos);
            }
            else{
                context.GetBlendFunc()(targetPixel, buffer, pos, targetInfo, sourceInfo, context.GetColoring(),false,bc);
            }
            pos = 0;
        }
    }
}