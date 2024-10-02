#include "RenderContext2D.h"
#include "../data/PixelFormat/PixelConverter.h"
#include "../data/PixelFormat/PixelFormatInfo.h"

void RenderContext2D::SetTargetTexture(Texture *targettexture)
{
    this->targetTexture = targettexture;
}

void RenderContext2D::SetBlendMode(BlendMode mode){
    this->mode = mode;
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

void RenderContext2D::DrawRect(Color color, uint16_t x, uint16_t y, uint16_t length, uint16_t height, float angle)
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

    // Precompute sine and cosine of the rotation angle
    float cosAngle = cos(angle);
    float sinAngle = sin(angle);

    // Center of the rectangle
    float centerX = x + length / 2.0f;
    float centerY = y + height / 2.0f;

    // Loop over every pixel in the texture space
    for (uint16_t i = 0; i < textureWidth; i++)
    {
        for (uint16_t j = 0; j < textureHeight; j++)
        {
            // Calculate the coordinates relative to the rectangle center
            float localX = i - centerX;
            float localY = j - centerY;

            // Apply reverse rotation (unrotate the pixel)
            float originalX = localX * cosAngle + localY * sinAngle;
            float originalY = -localX * sinAngle + localY * cosAngle;

            // Shift the coordinates back to the original rectangle's space
            float rectX = originalX + length / 2.0f;
            float rectY = originalY + height / 2.0f;

            // Check if the pixel falls within the original rectangle bounds
            if (rectX >= 0 && rectX < length && rectY >= 0 && rectY < height)
            {
                // Calculate the offset in the texture data based on the pixel position
                size_t offset = (j * textureWidth + i) * info.bytesPerPixel;

                // Copy the converted color data into the texture at the correct position
                memcpy(&textureData[offset], pixelData, info.bytesPerPixel);
            }
        }
    }

    // Free allocated memory for pixel data
    delete[] pixelData;
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

    // Loop over the rectangle area
    for (size_t i = x; i < x + length; i++)
    {
        for (size_t j = y; j < y + height; j++)
        {
            // Ensure the pixel position is within the texture bounds
            if (i < textureWidth && j < textureHeight)
            {
                // Calculate the offset in the texture data based on the pixel position
                size_t offset = (j * textureWidth + i) * info.bytesPerPixel;

                // Copy the converted color data into the texture at the correct position
                memcpy(&textureData[offset], pixelData, info.bytesPerPixel);
            }
        }
    }
    

    // Free allocated memory for pixel data
    delete[] pixelData;
}


void RenderContext2D::DrawArray(uint8_t* data, uint16_t x, uint16_t y, uint16_t width, uint16_t height, PixelFormat sourceFormat)
{
    if (targetTexture == nullptr)
        return;

    // Get target texture properties
    uint8_t* textureData = targetTexture->GetData();
    uint16_t textureWidth = targetTexture->GetWidth();
    uint16_t textureHeight = targetTexture->GetHeight();

    // Clip the drawing area to avoid going out of bounds
    uint16_t clippedWidth = std::min(width, (uint16_t)(textureWidth - x));
    uint16_t clippedHeight = std::min(height, (uint16_t)(textureHeight - y));

    // Ensure the width and height are not negative
    if (clippedWidth <= 0 || clippedHeight <= 0)
        return;

    // Assuming source and target formats are the same
    for (uint16_t posY = 0; posY < clippedHeight; ++posY)
    {
        for (uint16_t posX = 0; posX < clippedWidth; ++posX)
        {
            // Source pixel index from `data` (assuming 3 bytes per pixel for RGB)
            uint32_t sourcePixelIndex = (posY * width + posX) * 3; // Assuming RGB24 (3 bytes per pixel)
            // Target pixel index in the texture data
            uint32_t targetPixelIndex = ((y + posY) * textureWidth + (x + posX)) * 3;

            // Copy RGB data from source to target
            textureData[targetPixelIndex] = data[sourcePixelIndex];     // Red
            textureData[targetPixelIndex + 1] = data[sourcePixelIndex + 1]; // Green
            textureData[targetPixelIndex + 2] = data[sourcePixelIndex + 2]; // Blue

            uint8_t d1 =  data[sourcePixelIndex];  
            uint8_t d2 =  data[sourcePixelIndex + 1];  
            uint8_t d3 =  data[sourcePixelIndex + 2];  
            uint8_t d4 = 44;
        }
    }
}

