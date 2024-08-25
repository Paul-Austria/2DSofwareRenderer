#include "Texture.h"
#include "PixelFormat/PixelFormatInfo.h"
Texture::Texture(uint32_t width, uint32_t height, PixelFormat format) : width(width), height(height), format(format)
{
    storedLocally = true;
    uint8_t bytesPerPixel = PixelFormatRegistry::GetInfo(format).bytesPerPixel;
    data = new uint8_t[bytesPerPixel*width*height*bytesPerPixel];
}

Texture::Texture(uint32_t width, uint32_t height, uint8_t *data, PixelFormat format): width(width), height(height), format(format), data(data)
{
    storedLocally = true;
}

Texture::~Texture()
{
    if(storedLocally) delete[] data;
}

uint8_t *Texture::GetData()
{
    return data;
}

PixelFormat Texture::GetFormat()
{
    return format;
}

uint32_t Texture::GetWidth()
{
    return width;
}

uint32_t Texture::GetHeight()
{
    return height;
}
