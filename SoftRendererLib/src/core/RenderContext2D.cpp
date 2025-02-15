#include "RenderContext2D.h"
#include "../data/PixelFormat/PixelConverter.h"
#include "../data/PixelFormat/PixelFormatInfo.h"
#include <cmath>
#include <algorithm>
#include <stdio.h>
#include "../util/MemHandler.h"
#include "../data/BlendMode/BlendFunctions.h"

using namespace Renderer2D;



RenderContext2D::RenderContext2D() : primitivesRenderer(*this), basicTextureRenderer(*this), transformedTextureRenderer(*this)
{
}
void RenderContext2D::SetTargetTexture(Texture *targettexture)
{
    this->targetTexture = targettexture;
}

Texture * RenderContext2D::GetTargetTexture()
{
    return targetTexture;
}

void RenderContext2D::SetBlendMode(BlendMode mode)
{
    this->mode = mode;
}

BlendMode Renderer2D::RenderContext2D::GetBlendMode()
{
    return this->mode;
}

void Renderer2D::RenderContext2D::SetSamplingMethod(SamplingMethod method)
{
    this->samplingMethod = method;
}

SamplingMethod Renderer2D::RenderContext2D::GetSamplingMethod()
{
    return samplingMethod;
}

void RenderContext2D::ClearTarget(Color color)
{
    if (targetTexture == nullptr)
    {
        return;
    }

    PixelFormat format = targetTexture->GetFormat();
    PixelFormatInfo info = PixelFormatRegistry::GetInfo(format);

    // Get pointer to the texture data
    uint8_t *textureData = targetTexture->GetData();
    uint16_t width = targetTexture->GetWidth();
    uint16_t height = targetTexture->GetHeight();

    // Use a stack-allocated array for pixel data
    uint8_t pixelData[4];
    color.ConvertTo(format, pixelData);

    // Calculate the total number of pixels
    size_t totalPixels = width * height;

    // Clear the texture by filling it with the converted color
    for (size_t i = 0; i < totalPixels; ++i)
    {
        // Calculate the position in the texture data array
        size_t pixelIndex = i * info.bytesPerPixel;
        MemHandler::MemCopy(&textureData[pixelIndex], pixelData, info.bytesPerPixel);
    }
}

void RenderContext2D::EnableClipping(bool clipping)
{
    this->enableClipping = clipping;
}
bool Renderer2D::RenderContext2D::IsClippingEnabled()
{
    return enableClipping;
}
void RenderContext2D::SetClipping(uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY)
{
    this->clippingArea.startX = startX;
    this->clippingArea.startY = startY;
    this->clippingArea.endX = endX;
    this->clippingArea.endY = endY;
}

ClippingArea Renderer2D::RenderContext2D::GetClippingArea()
{
    return clippingArea;
}

void Renderer2D::RenderContext2D::SetColoringSettings(Coloring coloring)
{
    this->colorOverlay = coloring;
}

Coloring &Renderer2D::RenderContext2D::GetColoring()
{
    return colorOverlay;
}
