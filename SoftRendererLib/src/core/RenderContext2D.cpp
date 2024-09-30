#include "RenderContext2D.h"
#include "../data/PixelFormat/PixelConverter.h"
#include "../data/PixelFormat/PixelFormatInfo.h"

void RenderContext2D::SetTargetTexture(Texture *targettexture)
{
    this->targetTexture = targettexture;
}

void RenderContext2D::ClearTarget(Color color)
{
    if(targetTexture != nullptr)
    {
        PixelFormat format = targetTexture->GetFormat();
        PixelFormatInfo info = PixelFormatRegistry::GetInfo(format);

        // Get pointer to the texture data
        uint8_t* textureData = targetTexture->GetData();
        uint16_t width = targetTexture->GetWidth();
        uint16_t height = targetTexture->GetHeight();

        // Allocate memory to store the converted color data
        uint8_t* pixelData = new uint8_t[info.bytesPerPixel];

        // Convert the input color to the target pixel format
        color.ConvertTo(format, pixelData);

        // Iterate through all pixels and set them to the converted color
        for (uint16_t y = 0; y < height; ++y)
        {
            for (uint16_t x = 0; x < width; ++x)
            {
                // Calculate the position in the texture data array
                uint32_t pixelIndex = (y * width + x) * info.bytesPerPixel;

                // Copy the pixel data to the texture data
                for (uint32_t byte = 0; byte < info.bytesPerPixel; ++byte)
                {
                    textureData[pixelIndex + byte] = pixelData[byte];
                }
            }
        }

        // Free allocated memory for pixel data
        delete[] pixelData;
    }
}



void RenderContext2D::DrawRect(Color color, uint16_t x, uint16_t y, uint16_t length, uint16_t height)
{
    if (targetTexture == nullptr)
        return;

    PixelFormat format = targetTexture->GetFormat();
    PixelFormatInfo info = PixelFormatRegistry::GetInfo(format);
    
    // Get pointer to the texture data
    uint8_t* textureData = targetTexture->GetData();
    uint16_t textureWidth = targetTexture->GetWidth();
    uint16_t textureHeight = targetTexture->GetHeight();
    
    // Allocate memory for storing the converted pixel color
    uint8_t* pixelData = new uint8_t[info.bytesPerPixel];

    // Convert the input color to the format of the texture
    color.ConvertTo(format, pixelData);

    // Clip the rectangle to the texture boundaries
    uint16_t rectXStart = std::max(0, (int)x);
    uint16_t rectYStart = std::max(0, (int)y);
    uint16_t rectXEnd = std::min((int)(x + length), (int)textureWidth);
    uint16_t rectYEnd = std::min((int)(y + height), (int)textureHeight);

    // Iterate over the rectangle area and set each pixel
    for (uint16_t posY = rectYStart; posY < rectYEnd; ++posY)
    {
        for (uint16_t posX = rectXStart; posX < rectXEnd; ++posX)
        {
            // Calculate the position in the texture data array
            uint32_t pixelIndex = (posY * textureWidth + posX) * info.bytesPerPixel;

            // Copy the pixel data (color) to the texture data
            for (uint32_t byte = 0; byte < info.bytesPerPixel; ++byte)
            {
                textureData[pixelIndex + byte] = pixelData[byte];
            }
        }
    }

    // Free allocated memory for pixel data
    delete[] pixelData;
}
